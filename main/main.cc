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

#include "config.h"
#include "InputError.h"
#include "JsonPrinter.h"
#include "Jwt.h"

#if defined(JWT_OS_WIN)
#  include <io.h>
#  define isatty(x) _isatty(x)
#  define STDIN_FILENO 0
#  define STDOUT_FILENO 1
#else
#  include <unistd.h>
#endif

class UsageError : public std::runtime_error
{
public:
  UsageError(const std::string& what) : std::runtime_error(what)
  {}
};

class InvalidOptionError : public UsageError
{
public:
  InvalidOptionError(const char* opt) : UsageError(std::string{"Unrecognized option: "} + std::string{opt})
  {}
};

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

class Program
{
public:
  Program(int argc, char** argv);

  void run();

private:
  void print_header(const jwt::Jwt& token) const;
  void print_payload(const jwt::Jwt& token) const;
  void print_everything(const jwt::Jwt& token) const;
  void print_raw_json() const;

private:
  std::string input;
  bool use_ansi_colors;

  enum ProgramMode {
    modeDefault = 0,
    modeHeader = 1,
    modePayload = 2,
    modeRawJson = 4
  } mode;
};

Program::Program(int argc, char** argv)
{
  mode = modeDefault;

  for (int i = 1; i < argc; ++i)
  {
    char* opt = argv[i];
    if (strcmp("-h", opt) == 0 || strcmp("--help", opt) == 0)
    {
      usage();
      exit(0);
      return;
    }

    if (strcmp("-H", opt) == 0 || strcmp("--print-header", opt) == 0)
    {
      mode = static_cast<ProgramMode>(mode | modeHeader);
      continue;
    }

    if (strcmp("-p", opt) == 0 || strcmp("--print-payload", opt) == 0)
    {
      mode = static_cast<ProgramMode>(mode | modePayload);
      continue;
    }

    if (strcmp("-r", opt) == 0)
    {
      mode = static_cast<ProgramMode>(mode | modeRawJson);
      continue;
    }

    if (i == argc - 1)
    {
      input = std::string{opt};
    }
    else
    {
      throw InvalidOptionError(opt);
    }
  }

  if (input.empty() && !isatty(STDIN_FILENO))
  {
    std::stringstream ss;
    std::string line;
    while (std::getline(std::cin, line))
    {
      ss << line << std::endl;
    }

    input = ss.str();
  }

  if (input.empty())
  {
    throw UsageError("No token detected");
  }

  use_ansi_colors = isatty(STDOUT_FILENO);
}

void Program::print_header(const jwt::Jwt& token) const
{
  jwt::pretty_print_json(std::cout, token.header(), use_ansi_colors);
}

void Program::print_payload(const jwt::Jwt& token) const
{
  jwt::pretty_print_json(std::cout, token.header(), use_ansi_colors);
}

void Program::print_everything(const jwt::Jwt& token) const
{
  std::cout << "Header: " << std::endl;
  print_header(token);

  std::cout << "Payload: " << std::endl;
  print_payload(token);

  std::cout << "Signature: " << std::endl << token.signature() << std::endl;
}

void Program::print_raw_json() const
{
  jwt::pretty_print_json(std::cout, input, use_ansi_colors);
}

void Program::run()
{
  if (mode & modeRawJson)
  {
    print_raw_json();
    return;
  }

  jwt::Jwt token(input);

  if (mode == modeDefault)
  {
    print_everything(token);
    return;
  }

  if (mode & modeHeader)
  {
    print_header(token);
  }

  if (mode & modePayload)
  {
    if (mode & modeHeader)
    {
      std::cout << std::endl;
    }
    print_payload(token);
  }
}

int main(int argc, char** argv)
{
  try
  {
    Program program(argc, argv);
    program.run();
  }
  catch (const UsageError& ex)
  {
    std::cerr << ex.what() << std::endl;
    usage();
    return 1;
  }
  catch (const jwt::InputError& ex)
  {
    std::cerr << ex.what() << std::endl;
    return 1;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
    return 1;
  }
}
