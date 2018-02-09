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

#ifndef JWT_JSON_H
#define JWT_JSON_H

#include <iosfwd>
#include <string>

namespace jwt {

#if defined(_WIN32) || defined(WIN32)
constexpr const char* newline = "\r\n";
#else
constexpr const char* newline = "\n";
#endif


std::ostream& pretty_print_json(std::ostream& os, const std::string& json, bool use_ansi_colors = false);

} // jwt

#endif // JWT_JSON_H