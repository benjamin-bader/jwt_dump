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

#include "libjwt/JsonPrinter.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <stack>
#include <utility>

#include "libjwt/config.h"
#include "libjwt/termcolor.hpp"

namespace jwt {

namespace {

constexpr const char* indent_unit = "  ";

class PrintingJsonVisitor : public IJsonVisitor
{
public:
  PrintingJsonVisitor(std::ostream& os)
    : os_(os)
  {}

  virtual void on_object_start() override
  {
    write_separator();
    os() << "{";
    push_object();
    newline_and_indent();
  }

  virtual void on_object_field_name(const std::string& name) override
  {
    ordered_json j;
    j = name;

    write_separator();
    os() << j.dump() << ": ";
    mark_field_value_expected();
  }

  virtual void on_object_end() override
  {
    pop_context();
    newline_and_indent();
    os() << "}";
    mark_value_written();
  }

  virtual void on_array_start() override
  {
    write_separator();
    os() << "[";
    push_array();
    newline_and_indent();
  }

  virtual void on_array_end() override
  {
    pop_context();
    newline_and_indent();
    os() << "]";
    mark_value_written();
  }

  virtual void on_null() override
  {
    write_separator();
    os() << "null";
    mark_value_written();
  }

  virtual void on_string(const std::string& value) override
  {
    ordered_json j;
    j = value; // json-escape the text

    write_separator();
    os() << j.dump();
    mark_value_written();
  }

  virtual void on_signed_number(std::int64_t value) override
  {
    write_separator();
    os() << value;
    mark_value_written();
  }

  virtual void on_unsigned_number(std::uint64_t value) override
  {
    write_separator();
    os() << value;
    mark_value_written();
  }

  virtual void on_floating_point_number(double value) override
  {
    write_separator();
    os() << value;
    mark_value_written();
  }

  virtual void on_boolean(bool value) override
  {
    write_separator();
    os() << value;
    mark_value_written();
  }

protected:
  std::ostream& os()
  {
    return os_;
  }

protected:
  void newline_and_indent()
  {
    os() << newline;
    indent();
  }

  void indent()
  {
    for (std::size_t i = 0; i < contexts_.size(); ++i)
    {
      os() << indent_unit;
    }
  }

  void push_object()
  {
    contexts_.push(Context{0, false});
  }

  void push_array()
  {
    contexts_.push(Context{0, false});
  }

  void pop_context()
  {
    contexts_.pop();
  }

  void write_separator()
  {
    if (value_needs_separator())
    {
      os() << ',';
      newline_and_indent();
    }
  }

  void mark_field_value_expected()
  {
    if (contexts_.size() > 0)
    {
      contexts_.top().expecting_field_value = true;
    }
  }

  void mark_value_written()
  {
    if (contexts_.size() > 0)
    {
      contexts_.top().num_written++;
      contexts_.top().expecting_field_value = false;
    }
  }

  bool value_needs_separator()
  {
    return contexts_.size() > 0
        && contexts_.top().num_written > 0
        && !contexts_.top().expecting_field_value;
  }

private:
  struct Context
  {
    Context(std::size_t num_written, bool expecting_field_value)
        : num_written(num_written), expecting_field_value(expecting_field_value)
    {}

    std::size_t num_written {0};
    bool expecting_field_value {false};
  };

  std::ostream& os_;
  std::stack<Context> contexts_;
};

class AnsiColor
{
public:
  typedef std::ostream& (*color_manip)(std::ostream&);

  AnsiColor(std::ostream& os, color_manip color)
      : os_(os)
  {
    os_ << color;
  }

  ~AnsiColor()
  {
    os_ << termcolor::reset;
  }

private:
  std::ostream& os_;
};

class AnsiPrintingJsonVisitor : public PrintingJsonVisitor
{
public:
  AnsiPrintingJsonVisitor(std::ostream& os)
    : PrintingJsonVisitor(os)
  {}

  virtual ~AnsiPrintingJsonVisitor() = default;

  virtual void on_string(const std::string& value) override
  {
    AnsiColor color{os(), termcolor::cyan};
    PrintingJsonVisitor::on_string(value);
  }

  virtual void on_object_field_name(const std::string& name) override
  {
    ordered_json j;
    j = name;

    write_separator();
    {
      AnsiColor color{os(), termcolor::blue_light};
      os() << j.dump();
    }
    os() << ": ";

    mark_field_value_expected();
  }
};

}

std::ostream& pretty_print_json(std::ostream& os, const ordered_json& json, bool use_ansi_colors)
{
  std::unique_ptr<IJsonVisitor> visitor = use_ansi_colors
      ? std::make_unique<AnsiPrintingJsonVisitor>(os)
      : std::make_unique<PrintingJsonVisitor>(os);

  visit(json, *visitor);

  return os;
}

}
