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

#ifndef JWT_LIB_JSONPRINTER_H
#define JWT_LIB_JSONPRINTER_H

#pragma once

#include <iosfwd>
#include <string>

#include "nlohmann/json.hpp"

namespace jwt {

std::ostream& pretty_print_json(std::ostream& os, const nlohmann::json& json, bool use_ansi_colors);

} // namespace jwt

#endif // JWT_LIB_JSONPRINTER_H
