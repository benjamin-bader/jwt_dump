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

#ifndef JWT_LIB_JWT_H
#define JWT_LIB_JWT_H

#include <string>

#include "libjwt/JsonVisitor.h"

namespace jwt {

class Jwt
{
public:
  Jwt(const std::string& original_header,
      const std::string& original_payload,
      const std::string& signature,
      const ordered_json& header,
      const ordered_json& payload);

  static Jwt parse(const std::string& encoded);

  const std::string& original_header() const { return original_header_; }
  const std::string& original_payload() const { return original_payload_; }
  const std::string& signature() const { return signature_; }

  const ordered_json& header() const { return header_; }
  const ordered_json& payload() const { return payload_; }

  bool is_encrypted() const;
  bool is_signed() const;

private:
  std::string original_header_;
  std::string original_payload_;
  std::string signature_;

  ordered_json header_;
  ordered_json payload_;
};

}

#endif // JWT_LIB_JWT_H
