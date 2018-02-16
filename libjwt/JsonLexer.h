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

#ifndef JWT_LIB_JSONLEXER_H
#define JWT_LIB_JSONLEXER_H

#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

namespace jwt {

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

std::ostream& operator<<(std::ostream& os, TokenType type);
std::ostream& operator<<(std::ostream& os, const Token& token);

constexpr inline bool is_value_type(TokenType type)
{
  return type == TokenType::ObjectStart
      || type == TokenType::ArrayStart
      || type == TokenType::String
      || type == TokenType::Number
      || type == TokenType::Literal;
}

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

class JsonLexer
{
public:
  JsonLexer(const std::string& text);
  JsonLexer(const JsonLexer&) noexcept;
  JsonLexer(JsonLexer&&) noexcept;

  void tokenize(ITokenVisitor& visitor);

  bool next_token(Token& token) noexcept;

private:
  bool read_string_token(Token& token, size_t begin) noexcept;
  bool read_number_token(Token& token, size_t tokenStart) noexcept;
  bool read_literal_token(Token& token, size_t tokenStart, const char* expected) noexcept;

private:
  void consume_whitespace() noexcept;
  char next_char() noexcept;
  char char_at(size_t index) const noexcept;
  bool is_eof() const noexcept;

private:
  const std::string text_;
  size_t ix_;
  size_t end_;
};

} // namespace jwt

#endif // JWt_LIB_JSONLEXER_H