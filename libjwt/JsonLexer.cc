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

#include "libjwt/JsonLexer.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include "libjwt/InputError.h"

namespace jwt {

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

namespace {

inline bool is_blank(char c) noexcept
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

} // anonymous namespace

JsonLexer::JsonLexer(const std::string& text)
    : text_(text)
    , ix_(0)
    , end_(text.size())
{}

void JsonLexer::tokenize(ITokenVisitor& visitor)
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
    std::stringstream ss;
    ss << "Invalid input at offset " << ix_;
    throw InputError(ss.str());
  }
}

bool JsonLexer::next_token(Token& token) noexcept
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

bool JsonLexer::read_string_token(Token& token, size_t begin) noexcept
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

bool JsonLexer::read_number_token(Token& token, size_t tokenStart) noexcept
{
  // we _definitely_ don't care about validating correct floating-point here, just that
  // it _might_ be a number.
  size_t i = ix_;
  while (i < end_)
  {
    char c = char_at(i);
    if (isdigit(c) || c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+')
    {
      i++;
      continue;
    }

    break;
  }

  token.type = TokenType::Number;
  token.begin = tokenStart;
  token.end = i;
  token.text = text_.c_str() + tokenStart;
  ix_ = i;
  return true;
}

bool JsonLexer::read_literal_token(Token& token, size_t tokenStart, const char* expected) noexcept
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

void JsonLexer::consume_whitespace() noexcept
{
  while (ix_ < end_ && is_blank(text_[ix_]))
  {
    ++ix_;
  }
}

char JsonLexer::next_char() noexcept
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

char JsonLexer::char_at(size_t index) const noexcept
{
  assert(index < end_);
  return text_[index];
}

bool JsonLexer::is_eof() const noexcept
{
  return ix_ >= end_;
}

} // namespace jwt