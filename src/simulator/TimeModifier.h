#pragma once

#include "Value.h"
#include "ValueRange.h"
#include "Backend.h"

namespace hydla {
namespace simulator {


class TimeModifier{
public:
  TimeModifier(backend::Backend &b);

  ValueRange substitute_time(const Value& time, const ValueRange& range);
  Value substitute_time(const Value& time, const Value& value);

  ValueRange shift_time(const Value& time, const ValueRange& range);
  Value shift_time(const Value& time, const Value& value);
  
  // apply arbitrary function with two arguments of Value for each element of ValueRange
  ValueRange apply_function(const std::string& function, const Value& time, const ValueRange& range);
  // apply arbitrary function with two arguments of Value
  Value apply_function(const std::string& function, const Value& time, const Value& value);
private:
  backend::Backend backend;
};

}
}


