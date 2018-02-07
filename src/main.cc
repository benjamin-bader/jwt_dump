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

#include <cstdio>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "jwt.h"

#if defined(_WIN32) || defined(WIN32)
#  include <io.h>
#  define isatty(x) _isatty(X)
#else
#  include <unistd.h>
#endif

int main(int argc, char** argv)
{
  std::string encoded_jwt;
  if (argc == 1)
  {
    if (isatty(STDIN_FILENO))
    {
      // we're in interactive mode - just take one line.
      std::getline(std::cin, encoded_jwt);
    }
    else
    {
      // stdin is redirected; buffer all of it.
      std::stringstream ss;

      for (std::string line; std::getline(std::cin, line); )
      {
        ss << line << "\n";
      }

      encoded_jwt = ss.str();
    }
  }
  else
  {
    encoded_jwt = std::string{argv[1]};
  }

  try
  {
    jwt::Jwt token(encoded_jwt);
    token.dump(std::cout);
    std::cout << std::endl; // flush
    return 0;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
    return 1;
  }
}
