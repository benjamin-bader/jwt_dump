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

#ifndef JWT_LIB_JSONREADER_H
#define JWT_LIB_JSONREADER_H

#pragma once

#include <cstdint>
#include <string>
#include <stack>

#include "JsonLexer.h"

namespace jwt {

enum class ReadScope
{
  TopLevel,
  EmptyObject,
  NonemptyObject,
  EmptyArray,
  NonemptyArray,
  Name
};

class JsonReader
{
public:
  JsonReader(const std::string& json);
  JsonReader(const JsonReader& reader) noexcept;
  JsonReader(JsonReader&& reader) noexcept;

  void begin_object();
  void end_object();

  void begin_array();
  void end_array();

  bool has_more();

  std::string next_name();
  std::string next_string();
  int64_t next_i64();
  double next_double();
  bool next_bool();
  void next_null();

private:
  bool peek();
  void clear_peeked() noexcept;

  void expect_token(TokenType type);
  void consume() noexcept;

  void replace_top_scope(ReadScope scope);

private:
  JsonLexer lexer;
  Token peeked;

  std::stack<ReadScope> scopes;
};

} // namespace jwt

#endif // JWT_LIB_JSONREADER_H