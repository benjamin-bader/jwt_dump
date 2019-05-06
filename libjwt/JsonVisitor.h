/*
Copyright (C) 2019 Benjamin Bader

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

#include <cstdint>
#include <string>

#include "nlohmann/json.hpp"

namespace jwt {

class IJsonVisitor
{
public:
  virtual ~IJsonVisitor() = default;

  virtual void on_object_start() = 0;
  virtual void on_object_field_name(const std::string& name) = 0;
  virtual void on_object_end() = 0;

  virtual void on_array_start() = 0;
  virtual void on_array_end() = 0;

  virtual void on_null() = 0;
  virtual void on_string(const std::string& value) = 0;
  virtual void on_signed_number(std::int64_t value) = 0;
  virtual void on_unsigned_number(std::uint64_t value) = 0;
  virtual void on_floating_point_number(double value) = 0;
  virtual void on_boolean(bool value) = 0;
};

void visit(const nlohmann::json& json, IJsonVisitor& visitor);

}
