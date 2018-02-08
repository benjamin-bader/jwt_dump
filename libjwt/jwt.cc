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

#include "jwt.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <vector>

#include "base64.h"
#include "json.h"

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

constexpr const char* indent = "  ";

inline void do_indent(std::ostream& os, int level)
{
  for (int i = 0; i < level; ++i)
  {
    os << indent;
  }
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

Jwt::Jwt(const std::string& encoded)
{
  std::vector<std::string> parts = split(encoded, '.');
  if (parts.size() != 2 && parts.size() != 3)
  {
    throw std::runtime_error{"expected two or three segments delimited with a '.'"};
  }

  auto encoded_header = trim(parts[0]);
  auto encoded_payload = trim(parts[1]);

  header_ = encoded_header.size() != 0 ? base64_urlsafe_decode(encoded_header) : ""; 
  payload_ = encoded_payload.size() != 0 ? base64_urlsafe_decode(encoded_payload) : "";
  if (parts.size() == 3)
  {
    auto encoded_signature = trim(parts[2]);
    signature_ = encoded_signature; // no need to decode this, it's binary data
  }
}

void Jwt::dump(std::ostream& os)
{
  os << "Header: ";
  if (header_.size() > 0)
  {
    pretty_print_json(os, header_);
  }
  else
  {
    os << "{}";
  }
  os << newline;

  os << "Payload: ";
  if (payload_.size() > 0)
  {
    pretty_print_json(os, payload_);
  }
  else
  {
    os << "{}";
  }
  os << newline;

  os << "Signature: ";
  if (signature_.size() > 0)
  {
    os << signature_;
  }
  else
  {
    os << "<empty>";
  }
  os << newline;
}

}