//

#include "libjwt/JsonVisitor.h"

#include <algorithm>
#include <functional>
#include <string>

namespace jwt {

void visit(const nlohmann::json& json, IJsonVisitor& visitor)
{
  if (json.is_object())
  {
    visitor.on_object_start();
    std::for_each(json.items().begin(), json.items().end(), [&](auto& el) {
      visitor.on_object_field_name(el.key());
      visit(el.value(), visitor);
    });
    visitor.on_object_end();
  }
  else if (json.is_array())
  {
    visitor.on_array_start();
    std::for_each(json.begin(), json.end(), [&](auto& el) { visit(el, visitor); });
    visitor.on_array_end();
  }
  else if (json.is_string())
  {
    visitor.on_string(json.get<std::string>());
  }
  else if (json.is_number_float())
  {
    visitor.on_floating_point_number(json.get<double>());
  }
  else if (json.is_number_unsigned())
  {
    visitor.on_unsigned_number(json.get<std::uint64_t>());
  }
  else if (json.is_number_integer())
  {
    visitor.on_signed_number(json.get<std::int64_t>());
  }
  else if (json.is_boolean())
  {
    visitor.on_boolean(json.get<bool>());
  }
  else if (json.is_null())
  {
    visitor.on_null();
  }
  else
  {
    abort();
  }
}

}
