#include "JsonReader.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "InputError.h"

namespace jwt {

static inline constexpr int hex_value(char c) noexcept
{
  if (c >= '0' && c <= '9')
  {
    return c - '0';
  }
  else if (c >= 'a' && c <= 'f')
  {
    return c - 'a' + 10;
  }
  else if (c >= 'A' && c <= 'F')
  {
    return c - 'A' + 10;
  }
  else
  {
    return -1;
  }
}

static uint32_t read_codepoint(const char* text, size_t len, size_t pos)
{
  if (pos > len - 4)
  {
    throw InputError("Invalid JSON - unfinished unicode escape sequence");
  }

  uint32_t codepoint = 0;
  for (int i = 0; i < 4; ++i)
  {
    codepoint <<= 4;
    char c = text[pos + i];
    int value = hex_value(c);
    if (value == -1)
    {
      throw InputError("Invalid JSON - not a hex character");
    }
    codepoint |= static_cast<uint32_t>(value);
  }

  return codepoint;
}

static void unescape_unicode_sequence(const char* text, size_t text_len, size_t& i, std::string& unescaped)
{
  uint32_t codepoint = read_codepoint(text, text_len, i);
  i += 4;

  if (codepoint >= 0xD800 && codepoint <= 0xDBFF)
  {
    if (i < text_len - 2 && text[i] == '\\' && text[i + 1] == 'u')
    {
      uint32_t codepoint2 = read_codepoint(text, text_len, i + 2);
      i += 6;

      codepoint = (codepoint << 10) + codepoint2 - 0x035FDC00;
    }
    else
    {
      throw InputError("Invalid JSON - high surrogate must be followed by a low surrogate");
    }
  }
  else
  {
    if (codepoint >= 0xDC00 && codepoint <= 0xDFFF)
    {
      throw InputError("Invalid JSON - low surrogate must follow high surrogate");
    }
  }

  if (codepoint < 0x80)
  {
    unescaped += static_cast<char>(codepoint & 0xFF);
  }
  else if (codepoint < 0x800)
  {
    unescaped += static_cast<char>(0xC0 | (codepoint >> 6));
    unescaped += static_cast<char>(0x80 | (codepoint & 0x3F));
  }
  else if (codepoint < 0x10000)
  {
    unescaped += static_cast<char>(0xE0 | (codepoint >> 12));
    unescaped += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    unescaped += static_cast<char>(0x80 | (codepoint & 0x3F));
  }
  else
  {
    unescaped += static_cast<char>(0xF0 | (codepoint >> 18));
    unescaped += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
    unescaped += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    unescaped += static_cast<char>(0x80 | (codepoint & 0x3F));
  }
}

static std::string unescape_string_token(const char* text, size_t text_len)
{
  std::string unescaped;
  unescaped.reserve(text_len);

  // Tokenization should have already caught unquoted string literals
  assert(text[0] == '"' && text[text_len - 1] == '"');
  size_t unquoted_len = text_len - 1;

  if (unquoted_len == 0)
  {
    return "";
  }

  for (size_t i = 1; i < unquoted_len; ++i)
  {
    char c = text[i];
    if (c == '\\')
    {
      if (i == unquoted_len - 1)
      {
        throw InputError("Invalid JSON - unfinished escape sequence");
      }

      c = text[++i];
      switch (c)
      {
        case 'b':  unescaped += '\b'; break;
        case 'f':  unescaped += '\f'; break;
        case 'n':  unescaped += '\n'; break;
        case 'r':  unescaped += '\r'; break;
        case 't':  unescaped += '\t'; break;
        case '"':  unescaped += '"';  break;
        case '/':  unescaped += '/';  break;
        case '\\': unescaped += '\\'; break;
        case 'u':
          ++i;
          unescape_unicode_sequence(text, unquoted_len, i, unescaped);
          // we are positioned one ahead of the last character of the
          // sequence, and must decrement i to compensate for the
          // for-loop's increment step.
          --i;
          break;
      }
    }
    else
    {
      unescaped += c;
    }
  }

  unescaped.shrink_to_fit();
  return unescaped;
}

static inline std::string unescape_string_token(const Token& token)
{
  return unescape_string_token(token.text, token.end - token.begin);
}

JsonReader::JsonReader(const std::string& json)
    : JsonReader(std::move(JsonLexer{json}))
{
}

JsonReader::JsonReader(JsonLexer&& lexer)
    : lexer(std::move(lexer))
    , peeked()
    , scopes({ ReadScope::TopLevel })
{
  clear_peeked();
}

void JsonReader::begin_object()
{
  expect_token(TokenType::ObjectStart);
  consume();
  scopes.push(ReadScope::EmptyObject);
}

void JsonReader::end_object()
{
  expect_token(TokenType::ObjectEnd);
  consume();
  scopes.pop();
}

void JsonReader::begin_array()
{
  expect_token(TokenType::ArrayStart);
  consume();
  scopes.push(ReadScope::EmptyArray);
}

void JsonReader::end_array()
{
  expect_token(TokenType::ArrayEnd);
  consume();
  scopes.pop();
}

bool JsonReader::has_more()
{
  return peek() && peeked.type != TokenType::ObjectEnd && peeked.type != TokenType::ArrayEnd;
}

std::string JsonReader::next_name()
{
  if (! peek())
  {
    throw std::runtime_error("Expected a name, but there are no more tokens");
  }

  if (scopes.top() != ReadScope::Name)
  {
    throw std::runtime_error("No name is expected");
  }

  if (peeked.type != TokenType::String)
  {
    std::cerr << "token type: " << peeked.type << std::endl;
    throw std::runtime_error("Expected a string");
  }

  auto result = unescape_string_token(peeked);
  consume();

  return result;
}

std::string JsonReader::next_string()
{
  if (! peek())
  {
    throw InputError("No more input");
  }

  if (scopes.top() == ReadScope::Name)
  {
    throw InputError("Expected a string, but is positioned at a name");
  }

  if (peeked.type != TokenType::String)
  {
    std::cerr << "token type: " << peeked.type << std::endl;
    throw InputError("Expected a string");
  }

  auto result = unescape_string_token(peeked);
  consume();
  return result;
}

bool JsonReader::next_bool()
{
  expect_token(TokenType::Literal);
  bool result;
  if (std::strncmp(peeked.text, "true", 4) == 0)
  {
    result = true;
  }
  else if (std::strncmp(peeked.text, "false", 5) == 0)
  {
    result = false;
  }
  else
  {
    throw InputError(std::string{"Expected a boolean; got: "} + std::string{peeked.text, peeked.end - peeked.begin});
  }
  consume();
  return result;
}

int64_t JsonReader::next_i64()
{
  expect_token(TokenType::Number);

  return 0;
}

void JsonReader::expect_token(TokenType type)
{
  if (peeked.begin == -1)
  {
    if (!peek())
    {
      throw InputError("Unexpected EOF"); // not true, make a better error message
    }
  }

  if (peeked.type != type)
  {
    throw InputError("Unexpected token type");
  }
}

/**
 peek() will advance to the next interesting token, where "interesting" is either a
 value-token or a scope-changing token (array/object open/close).
 */
bool JsonReader::peek()
{
  if (peeked.begin != -1)
  {
    return true;
  }

  if (!lexer.next_token(peeked))
  {
    return false;
  }

  ReadScope scope = scopes.top();
  switch (scope)
  {
    case ReadScope::EmptyArray:
      if (peeked.type == TokenType::ArrayEnd)
      {
        return true;
      }
      else if (!is_value_type(peeked.type))
      {
        throw std::runtime_error("Invalid JSON");
      }
      replace_top_scope(ReadScope::NonemptyArray);
      return true;

    case ReadScope::NonemptyArray:
      if (peeked.type == TokenType::ArrayEnd)
      {
        return true;
      }
      else if (peeked.type != TokenType::Comma)
      {
        throw std::runtime_error("Invalid JSON - expected comma");
      }

      if (!lexer.next_token(peeked))
      {
        return false;
      }

      if (!is_value_type(peeked.type))
      {
        throw std::runtime_error("Invalid JSON - expected value type");
      }

      return true;

    case ReadScope::EmptyObject:
      if (peeked.type == TokenType::ObjectEnd)
      {
        return true;
      }
      else if (peeked.type == TokenType::String)
      {
        replace_top_scope(ReadScope::Name);
        return true;
      }
      else
      {
        throw std::runtime_error("Invalid JSON");
      }

    case ReadScope::NonemptyObject:
      if (peeked.type == TokenType::ObjectEnd)
      {
        return true;
      }
      else if (peeked.type != TokenType::Comma)
      {
        throw std::runtime_error("invalid json");
      }

      replace_top_scope(ReadScope::Name);

      if (!lexer.next_token(peeked))
      {
        return false;
      }

      if (peeked.type != TokenType::String)
      {
        throw std::runtime_error("Invalid JSON");
      }

      return true;

    case ReadScope::Name:
      if (peeked.type != TokenType::Colon)
      {
        throw std::runtime_error("Invalid JSON - expected ':'");
      }

      if (!lexer.next_token(peeked))
      {
        return false;
      }

      if (!is_value_type(peeked.type))
      {
        throw InputError("Invalid JSON (ReadScope::Name) - expected a value");
      }

      replace_top_scope(ReadScope::NonemptyObject);
      return true;

    case ReadScope::TopLevel:
      if (!is_value_type(peeked.type))
      {
        throw InputError("Invalid JSON - toplevel, expected a value");
      }
      return true;

    default:
      assert(false);
      return false;
  }
}

void JsonReader::clear_peeked() noexcept
{
  peeked.begin = -1;
  peeked.type = static_cast<TokenType>(-1);
}

void JsonReader::consume() noexcept
{
  peeked.begin = -1;
}

void JsonReader::replace_top_scope(ReadScope scope)
{
  if (scopes.top() == ReadScope::TopLevel)
  {
    throw std::runtime_error("No replaceable scope"); // This is obscure and unhelpful, find a better error message
  }

  scopes.pop();
  scopes.push(scope);
}

} // namespace jwt