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

#include "JsonReaderTest.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "config.h"
#include "InputError.h"
#include "JsonLexer.h"
#include "JsonPrinter.h"
#include "JsonReader.h"

using namespace jwt;

TEST(JsonReaderTest, string_with_simple_unicode_escapes)
{
  std::string original = R"("15\u00b0C")";
  JsonReader reader(original);
  EXPECT_EQ("15°C", reader.next_string());
}

TEST(JsonReaderTest, string_with_surrogate_pair)
{
  std::string original = R"("here is a robot: \uD83E\uDD16")";
  JsonReader reader(original);
  EXPECT_EQ("here is a robot: \xF0\x9F\xA4\x96", reader.next_string());
}

TEST(JsonReaderTest, string_with_mismatched_surrogate_pair)
{
  std::string original = R"("here is a robot: \uDD16\uD83E")";
  JsonReader reader(original);
  EXPECT_THROW(reader.next_string(), InputError);
}

TEST(JsonReaderTest, string_with_unmatched_low_surrogate)
{
  std::string original = R"("here is a robot: \uDD16")";
  JsonReader reader(original);
  EXPECT_THROW(reader.next_string(), InputError);
}

TEST(JsonReaderTest, string_with_unmatched_high_surrogate)
{
  std::string original = R"("here is a robot: \uD83E")";
  JsonReader reader(original);
  EXPECT_THROW(reader.next_string(), InputError);
}

TEST(JsonReaderTest, string_with_three_byte_utf8_escape)
{
  std::string original = R"("\u20AC")"; // a "euro" symbol
  JsonReader reader(original);
  EXPECT_EQ("\xe2\x82\xac", reader.next_string());
}

TEST(JsonReaderTest, empty_string_literal)
{
  JsonReader reader("\"\"");
  EXPECT_EQ("", reader.next_string());
}

TEST(JsonReaderTest, string_with_all_the_escapes)
{
  std::string original = R"("\b\f\r\n\t\\\/")";
  JsonReader reader(original);
  EXPECT_EQ("\b\f\r\n\t\\/", reader.next_string());
}

TEST(JsonReaderTest, empty_object)
{
  std::string orig = " {   } ";
  JsonReader reader(orig);
  reader.begin_object();
  reader.end_object();
}

TEST(JsonReaderTest, object_with_string_value)
{
  std::string json = R"({ "key"   : "value" })";
  JsonReader reader(json);
  reader.begin_object();
  EXPECT_EQ("key", reader.next_name());
  EXPECT_EQ("value", reader.next_string());
  reader.end_object();
}

TEST(JsonReaderTest, object_with_dangling_name)
{
  JsonReader reader("{\"key\": }");
  reader.begin_object();
  EXPECT_EQ("key", reader.next_name());
  EXPECT_THROW(reader.end_object(), InputError);
}

TEST(JsonReaderTest, object_with_true_boolean)
{
  std::string json = R"({ "key": true })";
  JsonReader reader(json);
  reader.begin_object();
  EXPECT_EQ("key", reader.next_name());
  EXPECT_EQ(true, reader.next_bool());
  reader.end_object();
}

TEST(JsonReaderTest, object_with_false_boolean)
{
  std::string json = R"({ "key": false })";
  JsonReader reader(json);
  reader.begin_object();
  EXPECT_EQ("key", reader.next_name());
  EXPECT_EQ(false, reader.next_bool());
  reader.end_object();
}

TEST(JsonReaderTest, has_more_is_false_in_empty_object)
{
  JsonReader reader("{}");
  reader.begin_object();
  EXPECT_EQ(false, reader.has_more());
  reader.end_object();
}

TEST(JsonReaderTest, has_more_is_true_at_start_of_nonempty_object)
{
  JsonReader reader(R"({ "key": "value" })");
  reader.begin_object();
  EXPECT_EQ(true, reader.has_more());
  EXPECT_EQ("key", reader.next_name());
  EXPECT_EQ("value", reader.next_string());
  EXPECT_EQ(false, reader.has_more());
  reader.end_object();
}

TEST(JsonReaderTest, object_with_all_kinds_of_values)
{
  JsonReader reader(R"({
    "str": "a string",
    "num": 3.14159,
    "null": null,
    "arr": [ 1, 2, 3 ],
    "true": true,
    "false": false
  })");

  reader.begin_object();
  EXPECT_EQ("str", reader.next_name());
  EXPECT_EQ("a string", reader.next_string());

  EXPECT_EQ("num", reader.next_name());
  EXPECT_EQ(3.14159, reader.next_double());

  EXPECT_EQ("null", reader.next_name());
  reader.next_null();

  EXPECT_EQ("arr", reader.next_name());
  reader.begin_array();
  EXPECT_EQ(1, reader.next_i64());
  EXPECT_EQ(2, reader.next_i64());
  EXPECT_EQ(3, reader.next_i64());
  reader.end_array();

  EXPECT_EQ("true", reader.next_name());
  EXPECT_EQ(true, reader.next_bool());

  EXPECT_EQ("false", reader.next_name());
  EXPECT_EQ(false, reader.next_bool());
  reader.end_object();
}

TEST(JsonReaderTest, empty_array)
{
  std::string orig = "[]";
  JsonReader reader(orig);
  reader.begin_array();
  reader.end_array();
}

