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

#include "libjwt/JsonPrinter.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <utility>

#include "libjwt/config.h"
#include "libjwt/InputError.h"
#include "libjwt/JsonLexer.h"
#include "libjwt/termcolor.hpp"

namespace jwt {

namespace {

constexpr const char* indent_unit = "  ";

class IWriteContext
{
public:
  virtual ~IWriteContext() = default;

  virtual bool is_object() const = 0;
  virtual bool can_write(TokenType type) const = 0;
  virtual void on_written(TokenType type) = 0;

  virtual bool is_expecting_object_key() const
  {
    return false;
  }
};

class ObjectWriteContext : public IWriteContext
{
public:
  ObjectWriteContext() : ows(owsExpectKey)
  {}

  bool is_object() const override
  {
    return true;
  }

  bool is_expecting_object_key() const override
  {
    return ows == owsExpectKey;
  }

  bool can_write(TokenType type) const override
  {
    switch (ows)
    {
      case owsExpectKey:              return type == TokenType::String || type == TokenType::ObjectEnd;
      case owsExpectSeparator:        return type == TokenType::Colon;
      case owsExpectValue:            return is_value_type(type);
      case owsExpectElementSeparator: return type == TokenType::Comma || type == TokenType::ObjectEnd;
      default:
        assert(false);
        return false;
    }
  }

  void on_written(TokenType type) override
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

      default:
        assert(false);
        break;
    }
  }

private:
  enum ObjectWriteState
  {
    owsExpectKey,
    owsExpectSeparator,
    owsExpectValue,
    owsExpectElementSeparator
  } ows;
}; // class ObjectWriteContext


class ArrayWriteContext : public IWriteContext
{
public:
  ArrayWriteContext() : aws(awsExpectValue)
  {}

  bool is_object() const override
  {
    return false;
  }

  bool can_write(TokenType type) const override
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

  void on_written(TokenType type) override
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

private:
  enum ArrayWriteState
  {
    awsExpectValue,
    awsExpectSeparator
  } aws;
}; // class ArrayWriteContext


class PrintingTokenVisitor : public ITokenVisitor
{
public:
  PrintingTokenVisitor(std::ostream& os)
      : os_(os)
  {}

  virtual void on_object_start(const Token& token) override
  {
    assert_writeable_and_update(token);
    os() << "{";
    push_object();
    newline_and_indent();\
  }

  virtual void on_field_separator(const Token& token) override
  {
    assert_writeable_and_update(token);
    os() << ": ";
  }

  virtual void on_object_end(const Token& token) override
  {
    assert_writeable_and_update(token);
    pop_object();
    newline_and_indent();
    os() << "}";
  }

  virtual void on_array_start(const Token& token) override
  {
    assert_writeable_and_update(token);
    os() << "[";
    push_array();
    newline_and_indent();
  }

  virtual void on_array_end(const Token& token) override
  {
    assert_writeable_and_update(token);
    pop_array();
    newline_and_indent();
    os() << "]";
  }

  virtual void on_element_separator(const Token& token) override
  {
    assert_writeable_and_update(token);
    os() << ",";
    newline_and_indent();
  }

  virtual void on_string(const Token& token) override
  {
    assert_writeable_and_update(token);
    write_token(token);
  }

  virtual void on_number(const Token& token) override
  {
    assert_writeable_and_update(token);
    write_token(token);
  }

  virtual void on_literal(const Token& token) override
  {
    assert_writeable_and_update(token);
    write_token(token);
  }

  virtual void on_eof() override
  {
    if (contexts_.size() != 0)
    {
      // fail
      std::cerr << "Unterminated array or object!" << std::endl;
    }
  }

protected:
  std::ostream& os()
  {
    return os_;
  }

  bool is_expecting_object_key() const noexcept
  {
    return in_object() && contexts_.top()->is_expecting_object_key();
  }

private: // output functions
  void write_token(const Token& token)
  {
    os().write(token.text, token.end - token.begin);
  }

  void newline_and_indent()
  {
    os() << newline;
    indent();
  }

  void indent()
  {
    for (int i = 0; i < contexts_.size(); ++i)
    {
      os() << indent_unit;
    }
  }

private: // validation functions
  void assert_writeable_and_update(const Token& token)
  {
    if (contexts_.size() > 0)
    {
      std::shared_ptr<IWriteContext> wc = contexts_.top();
      if (!wc->can_write(token.type))
      {
        std::cerr << std::endl << "Token not writeable! tok=" << token << std::endl;
        assert(false);
      }
      wc->on_written(token.type);
    }
  }

  bool in_object() const noexcept
  {
    return contexts_.size() > 0 && contexts_.top()->is_object();
  }

  bool in_array() const noexcept
  {
    return contexts_.size() > 0 && !contexts_.top()->is_object();
  }

private:
  void push_object()
  {
    contexts_.push(std::make_shared<ObjectWriteContext>());
  }

  void push_array()
  {
    contexts_.push(std::make_shared<ArrayWriteContext>());
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
  std::ostream& os_;
  std::stack<std::shared_ptr<IWriteContext>> contexts_;
}; // class PrintingTokenVisitor


class AnsiPrintingTokenVisitor : public PrintingTokenVisitor
{
public:
  AnsiPrintingTokenVisitor(std::ostream& os) : PrintingTokenVisitor(os)
  {}

  virtual void on_string(const Token& token) override
  {
    AnsiColor::color_manip color;
    if (is_expecting_object_key())
    {
      color = termcolor::blue_light;
    }
    else
    {
      color = termcolor::cyan;
    }

    AnsiColor ac(os(), color);
    PrintingTokenVisitor::on_string(token);
  }

private:
  class AnsiColor
  {
  public:
    typedef std::ostream& (*color_manip)(std::ostream&);

    AnsiColor(std::ostream& os, color_manip color)
        : os_(os)
    {
      os_ << color;
    }

    ~AnsiColor()
    {
      os_ << termcolor::reset;
    }

  private:
    std::ostream& os_;
  };
}; // class AnsiPrintingTokenVisitor

} // anonymous namespace

std::ostream& pretty_print_json(std::ostream& os, const std::string& json, bool use_ansi_colors)
{
  std::unique_ptr<ITokenVisitor> visitor = use_ansi_colors
      ? std::make_unique<AnsiPrintingTokenVisitor>(os)
      : std::make_unique<PrintingTokenVisitor>(os);

  JsonLexer lexer(json);
  lexer.tokenize(*visitor);

  return os;
}

}