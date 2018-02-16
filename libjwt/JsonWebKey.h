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

#ifndef JWT_LIB_JSONWEBKEY_H
#define JWT_LIB_JSONWEBKEY_H

#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace jwt {

class Jwt;

enum class Algorithm : uint8_t
{
  none = 1,
  HS256,
  HS384,
  HS512,
  RS256,
  RS384,
  RS512,
  ES256,
  ES384,
  ES512,
  PS256,
  PS384,
  PS512,

  RSA1_5,
  RSA_OAEP,
  RSA_OAEP_256,
  A128KW,
  A192KW,
  A256KW,
  dir,
  ECDH_ES,
  ECDH_ES_A128KW,
  ECDH_ES_A192KW,
  ECDH_ES_A256KW,
  A128GCMKW,
  A192GCMKW,
  A256GCMKW,
  PBES2_HS256_A128KW,
  PBES2_HS384_A192KW,
  PBES2_HS512_A256KW,
};

class IJsonWebKey
{
public:
  virtual ~IJsonWebKey() = default;

  virtual const std::string& key_id() const = 0;

  virtual void set_algorithm(Algorithm alg);

public:
  virtual void sign() const = 0;
  virtual void verify() const = 0;
  virtual void encrypt() const = 0;
  virtual void decrypt() const = 0;
  virtual void wrapKey() const = 0;
  virtual void unwrapKey() const = 0;
  virtual void deriveKey() const = 0;
  virtual void deriveBits() const = 0;
};

std::shared_ptr<IJsonWebKey> parse_json_web_key(const std::string& json);

} // namespace jwt

#endif