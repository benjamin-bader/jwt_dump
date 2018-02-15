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
  JsonReader(JsonLexer&& lexer);

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