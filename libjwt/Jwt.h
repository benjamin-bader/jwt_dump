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

#include "nlohmann/json.hpp"

namespace jwt {

class Jwt
{
public:
  Jwt(const std::string& encoded);
  Jwt(const std::string& header, const std::string& payload, const std::string& signature);

  const std::string& header() const { return header_; }
  const std::string& payload() const { return payload_; }
  const std::string& signature() const { return signature_; }

  const nlohmann::json& header_obj() const { return header_obj_; }
  const nlohmann::json& payload_obj() const { return payload_obj_; }

  bool is_encrypted() const;
  bool is_signed() const;

private:
  std::string header_;
  std::string payload_;
  std::string signature_;

  nlohmann::json header_obj_;
  nlohmann::json payload_obj_;
};

}

#endif // JWT_LIB_JWT_H
