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

#include "libjwt/Jwt.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

#include "libjwt/Base64.h"
#include "libjwt/InputError.h"

namespace jwt {

namespace {

template <typename Out>
void split(const std::string& str, char delimiter, Out&& result)
{
  std::stringstream ss(str);
  std::string item;
  while (std::getline(ss, item, delimiter))
  {
    *(result++) = item;
  }
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
  std::vector<std::string> result;
  split(str, delimiter, std::back_inserter(result));
  return result;
}

std::string trim(const std::string& text)
{
  auto begin = text.find_first_not_of(" \t");
  if (begin == std::string::npos)
  {
    return "";
  }

  auto end = text.find_last_not_of(" \t");
  auto range = end - begin + 1;
  return text.substr(begin, range);
}

} // anonymous namespace

Jwt Jwt::parse(const std::string& encoded)
{
  std::vector<std::string> parts = split(trim(encoded), '.');
  if (parts.size() != 2 && parts.size() != 3)
  {
    throw InputError{"expected two or three segments delimited with a '.'"};
  }

  auto encoded_header = parts[0];
  auto encoded_payload = parts[1];

  auto header = encoded_header.size() != 0 ? base64_urlsafe_decode(encoded_header) : "";
  auto payload = encoded_payload.size() != 0 ? base64_urlsafe_decode(encoded_payload) : "";
  std::string signature;
  if (parts.size() == 3)
  {
    auto encoded_signature = trim(parts[2]);
    signature = encoded_signature; // no need to decode this, it's binary data
  }

  return Jwt{header, payload, signature, ordered_json::parse(header), ordered_json::parse(payload)};
}

Jwt::Jwt(const std::string& original_header,
         const std::string& original_payload,
         const std::string& signature,
         const ordered_json& header,
         const ordered_json& payload)
    : original_header_(original_header)
    , original_payload_(original_payload)
    , signature_(signature)
    , header_(header)
    , payload_(payload)
{
}

bool Jwt::is_encrypted() const
{
  return header_["typ"] == "JWE";
}

bool Jwt::is_signed() const
{
  return signature_.size() > 0;
}

}
