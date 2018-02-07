/*
Copyright (C) 2018 Benjamin Bader

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "json.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <vector>

namespace jwt {

namespace {

constexpr const char* indent_unit = "  ";

enum class TokenType
{
  ObjectStart,
  ObjectEnd,
  ArrayStart,
  ArrayEnd,
  Colon,
  Comma,
  String,
  Number,
  Literal
};

struct Token
{
  TokenType type;
  size_t begin;
  size_t end;
  const char* text;
};

std::ostream& operator<<(std::ostream& os, TokenType type)
{
  switch (type)
  {
    case TokenType::ObjectStart: return os << "TokenType::ObjectStart";
    case TokenType::ObjectEnd: return os << "TokenType::ObjectEnd";
    case TokenType::ArrayStart: return os << "TokenType::ArrayStart";
    case TokenType::ArrayEnd: return os << "TokenType::ArrayEnd";
    case TokenType::Colon: return os << "TokenType::Colon";
    case TokenType::Comma: return os << "TokenType::Comma";
    case TokenType::String: return os << "TokenType::String";
    case TokenType::Number: return os << "TokenType::Number";
    case TokenType::Literal: return os << "TokenType::Literal";
    default:
      assert(false);
      return os << "Unexpected token type (" << static_cast<int>(type) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const Token& token)
{
  os << "Token{type=" << token.type << ", begin=" << token.begin << ", end=" << token.end << "text=";
  os.write(token.text, token.end - token.begin);
  os << "}";
  return os;
}

constexpr inline bool is_value_type(TokenType type)
{
  return type == TokenType::ObjectStart
      || type == TokenType::ArrayStart
      || type == TokenType::String
      || type == TokenType::Number
      || type == TokenType::Literal;
}

enum ObjectWriteState
{
  owsExpectKey,
  owsExpectSeparator,
  owsExpectValue,
  owsExpectElementSeparator
};

enum ArrayWriteState
{
  awsExpectValue,
  awsExpectSeparator
};

struct WriteContext
{
  bool is_object;
  union {
    ObjectWriteState ows;
    ArrayWriteState aws;
  };

  bool can_write(TokenType type)
  {
    if (is_object)
    {
      switch (ows)
      {
        case owsExpectKey: return type == TokenType::String || type == TokenType::ObjectEnd;
        case owsExpectSeparator: return type == TokenType::Colon;
        case owsExpectValue: return is_value_type(type);
        case owsExpectElementSeparator: return type == TokenType::Comma || type == TokenType::ObjectEnd;
      }
    }
    else
    {
      if (aws == awsExpectValue)
      {
        return is_value_type(type) || type == TokenType::ArrayEnd;
      }
      else
      {
        return type == TokenType::Comma || type == TokenType::ArrayEnd;
      }
    }
  }

  void on_written(TokenType type)
  {
    if (is_object)
    {
      switch (ows)
      {
        case owsExpectKey:
          assert(type == TokenType::String || type == TokenType::ObjectEnd);
          if (type == TokenType::String)
          {
            ows = owsExpectSeparator;
          }
          // else the object is closed, and this context will go away presently.
          break;

        case owsExpectSeparator:
          assert(type == TokenType::Colon);
          ows = owsExpectValue;
          break;

        case owsExpectValue:
          assert(is_value_type(type));
          ows = owsExpectElementSeparator;
          break;

        case owsExpectElementSeparator:
          assert(type == TokenType::Comma || type == TokenType::ObjectEnd);
          ows = owsExpectKey;
          break;
      }
    }
    else
    {
      switch (aws)
      {
        case awsExpectValue:
          assert(is_value_type(type) || type == TokenType::ArrayEnd);
          if (type != TokenType::ArrayEnd)
          {
            aws = awsExpectSeparator;
          } // else the array is done
          break;
        case awsExpectSeparator:
          assert(type == TokenType::Comma || type == TokenType::ArrayEnd);
          aws = awsExpectValue;
          break;
      }
    }
  }

  static WriteContext object()
  {
    WriteContext wc;
    wc.is_object = true;
    wc.ows = owsExpectKey;
    return wc;
  }

  static WriteContext array()
  {
    WriteContext wc;
    wc.is_object = false;
    wc.aws = awsExpectValue;
    return wc;
  }
};

class ITokenVisitor
{
public:
  virtual ~ITokenVisitor() = default;

  virtual void on_object_start(const Token& token) = 0;
  virtual void on_field_separator(const Token& token) = 0;
  virtual void on_object_end(const Token& token) = 0;
  virtual void on_array_start(const Token& token) = 0;
  virtual void on_array_end(const Token& token) = 0;
  virtual void on_element_separator(const Token& token) = 0;
  virtual void on_string(const Token& token) = 0;
  virtual void on_number(const Token& token) = 0;
  virtual void on_literal(const Token& token) = 0;
  virtual void on_eof() = 0;
};

class PrintingTokenVisitor : public ITokenVisitor
{
public:
  PrintingTokenVisitor(std::ostream& os) : os(os)
  {}

  void on_object_start(const Token& token) override
  {
    assert_writeable_and_update(token);
    os << "{";
    push_object();
    newline_and_indent();
  }

  void on_field_separator(const Token& token) override
  {
    assert_writeable_and_update(token);
    os << ": ";
  }

  void on_object_end(const Token& token) override
  {
    assert_writeable_and_update(token);
    pop_object();
    newline_and_indent();
    os << "}";
  }

  void on_array_start(const Token& token) override
  {
    assert_writeable_and_update(token);
    os << "[";
    push_array();
    newline_and_indent();
  }

  void on_array_end(const Token& token) override
  {
    assert_writeable_and_update(token);
    pop_array();
    newline_and_indent();
    os << "]";
  }

  void on_element_separator(const Token& token) override
  {
    assert_writeable_and_update(token);
    os << ",";
    newline_and_indent();
  }

  void on_string(const Token& token) override
  {
    assert_writeable_and_update(token);
    write_token(token);
  }

  void on_number(const Token& token) override
  {
    assert_writeable_and_update(token);
    write_token(token);
  }

  void on_literal(const Token& token) override
  {
    assert_writeable_and_update(token);
    write_token(token);
  }

  void on_eof() override
  {
    if (contexts_.size() != 0)
    {
      // fail
      std::cerr << "Unterminated array or object!" << std::endl;
    }
  }

private: // output functions
  void write_token(const Token& token)
  {
    os.write(token.text, token.end - token.begin);
  }

  void newline_and_indent()
  {
    os << newline;
    indent();
  }

  void indent() const
  {
    for (int i = 0; i < contexts_.size(); ++i)
    {
      os << indent_unit;
    }
  }

private: // validation functions
  void assert_writeable_and_update(const Token& token)
  {
    if (contexts_.size() > 0)
    {
      WriteContext& wc = contexts_.top();
      if (!wc.can_write(token.type))
      {
        std::cerr << std::endl << "Token not writeable! tok=" << token << std::endl;
        std::cerr << "WriteContext{is_object=" << wc.is_object;
        if (wc.is_object)
        {
          std::cerr << ", ows=" << wc.ows;
        }
        else
        {
          std::cerr << ", aws=" << wc.aws;
        }
        std::cerr << "}" << std::endl;
        assert(false);
      }
      wc.on_written(token.type);
    }
  }

  bool in_object() const noexcept
  {
    return contexts_.size() > 0 && contexts_.top().is_object;
  }

  bool in_array() const noexcept
  {
    return contexts_.size() > 0 && !contexts_.top().is_object;
  }

private:
  void push_object()
  {
    contexts_.push(WriteContext::object());
  }

  void push_array()
  {
    contexts_.push(WriteContext::array());
  }

  void pop_object()
  {
    if (! in_object())
    {
      std::cerr << "Expected an object context" << std::endl;
      assert(false);
    }
    contexts_.pop();
  }

  void pop_array()
  {
    if (! in_array())
    {
      std::cerr << "Expected an array context" << std::endl;
      assert(false);
    }
    contexts_.pop();
  }

private:
  std::ostream& os;
  std::stack<WriteContext> contexts_;
};

class JsonLexer
{
public:
  JsonLexer(const std::string& text)
    : text_(text)
    , ix_(0)
    , end_(text.size())
  {}

  void tokenize(ITokenVisitor& visitor)
  {
    ix_ = 0;

    Token token;
    while (next_token(token))
    {
      switch (token.type)
      {
        case TokenType::ObjectStart:
          visitor.on_object_start(token);
          break;

        case TokenType::ObjectEnd:
          visitor.on_object_end(token);
          break;

        case TokenType::ArrayStart:
          visitor.on_array_start(token);
          break;

        case TokenType::ArrayEnd:
          visitor.on_array_end(token);
          break;

        case TokenType::Colon:
          visitor.on_field_separator(token);
          break;

        case TokenType::Comma:
          visitor.on_element_separator(token);
          break;

        case TokenType::String:
          visitor.on_string(token);
          break;

        case TokenType::Number:
          visitor.on_number(token);
          break;

        case TokenType::Literal:
          visitor.on_literal(token);
          break;

        default:
          std::cerr << "Unexpected token type: " << token.type << std::endl;
          assert(false);
          break;
      }
    }

    if (is_eof())
    {
      visitor.on_eof();
    }
    else
    {
      std::cerr << "Halted on invalid input at " << ix_ << std::endl;
    }
  }

private:
  bool next_token(Token& token) noexcept
  {
    token.begin = -1;
    token.end = -1;

    consume_whitespace();
    if (is_eof())
    {
      return false;
    }

    size_t tokenStart = ix_;
    char c = next_char();
    
    switch (c)
    {
      case '{':
        token.type = TokenType::ObjectStart;
        token.begin = tokenStart;
        token.end = ix_;
        token.text = text_.c_str() + tokenStart;
        return true;

      case '}':
        token.type = TokenType::ObjectEnd;
        token.begin = tokenStart;
        token.end = ix_;
        token.text = text_.c_str() + tokenStart;
        return true;

      case '[':
        token.type = TokenType::ArrayStart;
        token.begin = tokenStart;
        token.end = ix_;
        token.text = text_.c_str() + tokenStart;
        return true;

      case ']':
        token.type = TokenType::ArrayEnd;
        token.begin = tokenStart;
        token.end = ix_;
        token.text = text_.c_str() + tokenStart;
        return true;

      case ':':
        token.type = TokenType::Colon;
        token.begin = tokenStart;
        token.end = ix_;
        token.text = text_.c_str() + tokenStart;
        return true;

      case ',':
        token.type = TokenType::Comma;
        token.begin = tokenStart;
        token.end = ix_;
        token.text = text_.c_str() + tokenStart;
        return true;

      case '"':
        return read_string_token(token, tokenStart);

      case 'n':
        return read_literal_token(token, tokenStart, "null");

      case 't':
        return read_literal_token(token, tokenStart, "true");

      case 'f':
        return read_literal_token(token, tokenStart, "false");

      default:
        if (isdigit(c) || c == '+' || c == '-')
        {
          return read_number_token(token, tokenStart);
        }
        else
        {
          return false;
        }
    }
  }

  bool read_string_token(Token& token, size_t begin) noexcept
  {
    for (size_t i = ix_; i < end_; ++i)
    {
      char c = char_at(i);
      if (c == '"')
      {
        token.type = TokenType::String;
        token.begin = begin;
        token.end = i + 1;
        token.text = text_.c_str() + begin;
        ix_ = i + 1;
        return true;
      }

      if (c == '\\')
      {
        i++; // we don't care if the escape sequence is correct
        continue;
      }
    }

    // unterminated string literal, parse fails
    return false;
  }

  bool read_number_token(Token& token, size_t tokenStart) noexcept
  {
    for (size_t i = ix_; i < end_; ++i)
    {
      char c = char_at(i);
      if (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+')
      {
        // we _definitely_ don't care about validating correct floating-point here, just that
        // it _might_ be a number.
        continue;
      }

      token.type = TokenType::Number;
      token.begin = tokenStart;
      token.end = i;
      token.text = text_.c_str() + tokenStart;
      ix_ = i;
      return true;
    }

    // unterminated number literal
    return false;
  }

  bool read_literal_token(Token& token, size_t tokenStart, const char* expected) noexcept
  {
    const char* ptr = expected + 1; // first char is already matched
    size_t i = ix_;

    assert(*expected == text_[ix_ - 1]);

    while (*ptr && i < end_)
    {
      if (text_[i] != *ptr)
      {
        return false;
      }

      ++i;
      ++ptr;
    }

    if (*ptr)
    {
      // didn't match all of [expected] before EOF
      assert(i == end_);
      ix_ = end_;
      return false;
    }

    token.type = TokenType::Literal;
    token.begin = tokenStart;
    token.end = i;
    token.text = text_.c_str() + tokenStart;
    ix_ = i;
    return true;
  }

  void consume_whitespace() noexcept
  {
    while (ix_ < end_ && isblank(text_[ix_]))
    {
      ++ix_;
    }
  }

  char next_char() noexcept
  {
    if (ix_ < end_)
    {
      return text_[ix_++];
    }
    else
    {
      return 0;
    }
  }

  char char_at(size_t index) const noexcept
  {
    assert(index < end_);
    return text_[index];
  }

  bool is_eof() const noexcept
  {
    return ix_ >= end_;
  }

private:
  const std::string& text_;
  size_t ix_;
  size_t end_;
}; // class JsonPrinter

}

std::ostream& pretty_print_json(std::ostream& os, const std::string& json)
{
  PrintingTokenVisitor visitor(os);
  JsonLexer lexer(json);

  lexer.tokenize(visitor);

  return os;
}

}