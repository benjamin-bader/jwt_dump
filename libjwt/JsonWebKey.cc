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

#include "JsonWebKey.h"

#include <cassert>
#include <functional>
#include <string>
#include <unordered_map>

#include "Base64.h"
#include "InputError.h"
#include "JsonLexer.h"

namespace jwt {

bool try_parse_algorithm(const std::string& algName, Algorithm* alg)
{
  static std::unordered_map<std::string, Algorithm> lookup {
    { "none",  Algorithm::none  },
    { "HS256", Algorithm::HS256 },
    { "HS384", Algorithm::HS384 },
    { "HS512", Algorithm::HS512 },
    { "RS256", Algorithm::RS256 },
    { "RS384", Algorithm::RS384 },
    { "RS512", Algorithm::RS512 },
    { "ES256", Algorithm::ES256 },
    { "ES384", Algorithm::ES384 },
    { "ES512", Algorithm::ES512 },
    { "PS256", Algorithm::PS256 },
    { "PS384", Algorithm::PS384 },
    { "PS512", Algorithm::PS512 },
  };

  assert(alg != nullptr);

  auto it = lookup.find(algName);
  if (it != lookup.end())
  {
    *alg = it->second;
    return true;
  }
  return false;
}

std::string algorithm_to_string(Algorithm alg)
{
  switch (alg)
  {
    case Algorithm::none:  return "none";
    case Algorithm::HS256: return "HS256";
    case Algorithm::HS384: return "HS384";
    case Algorithm::HS512: return "HS512";
    case Algorithm::RS256: return "RS256";
    case Algorithm::RS384: return "RS384";
    case Algorithm::RS512: return "RS512";
    case Algorithm::ES256: return "ES256";
    case Algorithm::ES384: return "ES384";
    case Algorithm::ES512: return "ES512";
    case Algorithm::PS256: return "PS256";
    case Algorithm::PS384: return "PS384";
    case Algorithm::PS512: return "PS512";
    default:
      assert(false);
      return "";
  }
}

class JsonWebKeyBase : public IJsonWebKey
{
public:
  JsonWebKeyBase(const std::string& key_id, Algorithm algo)
      : m_key_id(key_id)
      , m_algorithm(algo)
  {}

  const std::string& key_id() const override
  {
    return m_key_id;
  }

private:
  std::string m_key_id;
  Algorithm m_algorithm;
};

class KeyParsingVisitor : public ITokenVisitor
{
public:
  KeyParsingVisitor() = default;

  virtual void on_object_start(const Token& token) override
  {

  }

  virtual void on_field_separator(const Token& token) override
  {

  }

  virtual void on_object_end(const Token& token) override
  {

  }

  virtual void on_array_start(const Token& token) override
  {

  }

  virtual void on_array_end(const Token& token) override
  {

  }

  virtual void on_element_separator(const Token& token) override
  {

  }

  virtual void on_string(const Token& token) override
  {
    std::string unescaped;
    unescaped.resize(token.end - token.begin);
    for (size_t i = token.begin; i < token.end; ++i)
    {
      char c = token.text[i];
      if (c == '\\')
      {

      }
      else
      {

      }
    }
  }

  virtual void on_number(const Token& token) override
  {

  }

  virtual void on_literal(const Token& token) override
  {

  }

  virtual void on_eof() override
  {

  }
};

std::shared_ptr<IJsonWebKey> parse_json_web_key(const std::string& json)
{
  try
  {
    KeyParsingVisitor visitor;
    JsonLexer lexer(json);
    lexer.tokenize(visitor);
  }
  catch (const InputError& e)
  {
    return nullptr;
  }
}

} // namespace jwt