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

#include "testjson.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "config.h"
#include "JsonLexer.h"
#include "JsonPrinter.h"

namespace {

void assert_format_equals(const std::string& input, const std::vector<std::string>& expected)
{
  std::stringstream expectedStream;
  bool appended = false;
  for (auto&& line : expected)
  {
    if (appended)
    {
      expectedStream << newline;
    }
    expectedStream << line;
    appended = true;
  }
  
  std::stringstream ss;
  jwt::pretty_print_json(ss, input);

  EXPECT_EQ(expectedStream.str(), ss.str());
}

} // anonymous namespace

TEST(JsonTest, FormatSimpleObject)
{
  std::string input = R"({"hello": "world"})";
  
  assert_format_equals(input, {
    "{",
    "  \"hello\": \"world\"",
    "}"
  });
}

TEST(JsonTest, FormatTopLevelString)
{
  std::string input = R"("this is some \"text\".")";
  std::string expected = input;
  assert_format_equals(input, { expected });
}

TEST(JsonTest, FormatTopLevelNumber)
{
  std::string input = "-3.14159e+15";
  std::string expected = input;
  assert_format_equals(input, { expected });
}

TEST(JsonTest, FormatShortTopLevelNumber)
{
  std::string input = "2";
  std::string expected = input;
  assert_format_equals(input, { expected });
}

TEST(JsonTest, FormatEmptyArray)
{
  std::string input = "[]";
  std::string expected = "[\n]";
  //assert_format_equals(input, expected);
}