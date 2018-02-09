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
#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "json.h"
#include "jwt.h"

#if defined(_WIN32) || defined(WIN32)
#  include <io.h>
#  define isatty(x) _isatty(x)
#  define STDIN_FILENO 0
#  define STDOUT_FILENO 1
#else
#  include <unistd.h>
#endif

void usage()
{
  std::string lines[] = {
    "Parses and displays encoded JWT tokens.",
    "",
    "jwt_dump [-h|--help] [-H|--header] [-p|--payload]",
    "",
    "  -h OR --help              Displays this message.",
    "  -H OR --print-header      Displays the JWT header.",
    "  -p OR --print-payload     Displays the JWT payload.",
    "",
    "If no options are given, all parts of the token are displayed.",
    "Tokens may also be piped via stdin."
  };

  for (auto&& line : lines)
  {
    std::cerr << line << std::endl;
  }
}

int main(int argc, char** argv)
{
  std::string encoded_jwt;
  bool default_display_mode = true;
  bool show_header = false;
  bool show_payload = false;
  for (int i = 1; i < argc; ++i)
  {
    char* opt = argv[i];
    if (strcmp("-h", opt) == 0 || strcmp("--help", opt) == 0)
    {
      usage();
      return 0;
    }

    if (strcmp("-H", opt) == 0 || strcmp("--print-header", opt) == 0)
    {
      show_header = true;
      default_display_mode = false;
      continue;
    }

    if (strcmp("-p", opt) == 0 || strcmp("--print-payload", opt) == 0)
    {
      show_payload = true;
      default_display_mode = false;
      continue;
    }

    // unrecognized option, or token?
    if (i == argc - 1)
    {
      encoded_jwt = std::string{opt};
    }
    else
    {
      std::cerr << "Unrecognized option: " << opt << std::endl;
      usage();
      return -1;
    }
  }

  if (encoded_jwt.empty())
  {
    if (isatty(STDIN_FILENO))
    {
      // Interactive mode unsupported
      std::cerr << "No token detected" << std::endl;
      usage();
      return 1;
    }

    if (!std::getline(std::cin, encoded_jwt))
    {
      std::cerr << "wut" << std::endl;
      return 1;
    }
  }

  bool use_ansi_colors = isatty(STDOUT_FILENO);
  try
  {
    jwt::Jwt token(encoded_jwt);

    if (default_display_mode)
    {
      std::cout << "Header: " << std::endl;
      jwt::pretty_print_json(std::cout, token.header(), use_ansi_colors);
      std::cout << std::endl;

      std::cout << "Payload: " << std::endl;
      jwt::pretty_print_json(std::cout, token.payload(), use_ansi_colors);
      std::cout << std::endl;

      std::cout << "Signature: " << std::endl << token.signature();
    }
    else
    {
      if (show_header)
      {
        jwt::pretty_print_json(std::cout, token.header(), use_ansi_colors);
      }

      if (show_payload)
      {
        if (show_header)
        {
          std::cout << std::endl;
        }
        jwt::pretty_print_json(std::cout, token.payload(), use_ansi_colors);
      }
    }
    std::cout << std::endl; // flush
    return 0;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
    return 1;
  }
}
