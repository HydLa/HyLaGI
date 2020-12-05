#pragma once

#include "kv/constants.hpp"
#include "kv/dd.hpp"
#include "kv/interval.hpp"
#include "kv/rdd.hpp"
#include "kv/rdouble.hpp"

namespace hydla {
namespace interval {
typedef kv::interval<double> itvd;

class IntervalOrInteger {
public:
  itvd interval_value;
  int integer;
  bool is_integer;

  IntervalOrInteger operator+(const IntervalOrInteger &rhs);
  IntervalOrInteger operator-(const IntervalOrInteger &rhs);
  IntervalOrInteger operator*(const IntervalOrInteger &rhs);
  IntervalOrInteger operator/(const IntervalOrInteger &rhs);
  IntervalOrInteger operator-();
};

std::ostream &operator<<(std::ostream &ost, const IntervalOrInteger &val);

} // namespace interval
} // namespace hydla
