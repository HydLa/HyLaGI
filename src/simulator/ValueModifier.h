#pragma once

#include "Backend.h"
#include "PhaseResult.h"
#include "Value.h"
#include "ValueRange.h"

namespace hydla {
namespace simulator {

class ValueModifier {
public:
  ValueModifier(backend::Backend &b);

  variable_map_t substitute_time(const Value &time, const variable_map_t &map);
  ValueRange substitute_time(const Value &time, const ValueRange &range);
  Value substitute_time(const Value &time, const Value &value);

  variable_map_t shift_time(const Value &time, const variable_map_t &map);
  ValueRange shift_time(const Value &time, const ValueRange &range);
  Value shift_time(const Value &time, const Value &value);

  // apply function to variable_map_t
  variable_map_t apply_function(const std::string &function, const Value &time,
                                const variable_map_t &map);

  // apply function to ValueRange
  ValueRange apply_function(const std::string &function, const Value &time,
                            const ValueRange &range);

  // apply function with two arguments of Value
  Value apply_function(const std::string &function, const Value &time,
                       const Value &value);

  // apply function to variable_map_t
  variable_map_t apply_function(const std::string &function,
                                const variable_map_t &map,
                                const std::string &fmt = "");

  // apply function to ValueRange
  ValueRange apply_function(const std::string &function,
                            const ValueRange &range,
                            const std::string &fmt = "");

  // function with single argument of Value
  Value apply_function(const std::string &function, const Value &value,
                       const std::string &fmt = "");

private:
  backend::Backend backend;
};

} // namespace simulator
} // namespace hydla
