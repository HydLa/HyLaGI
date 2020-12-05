#include "IntervalOrInteger.h"

namespace hydla {
namespace interval {
std::ostream &operator<<(std::ostream &ost, const IntervalOrInteger &val) {
  if (val.is_integer) {
    ost << "integer: " << val.integer;
  } else {
    ost << "interval: " << val.interval_value;
  }
  return ost;
}

IntervalOrInteger IntervalOrInteger::operator+(const IntervalOrInteger &rhs) {
  IntervalOrInteger ret;
  if (is_integer && rhs.is_integer) {
    ret.is_integer = true;
    ret.integer = integer + rhs.integer;
  } else {
    ret.is_integer = false;
    if (is_integer) {
      ret.interval_value = integer + rhs.interval_value;
    } else {
      if (rhs.is_integer) {
        ret.interval_value = interval_value + rhs.integer;
      } else {
        ret.interval_value = interval_value + rhs.interval_value;
      }
    }
  }
  return ret;
}

IntervalOrInteger IntervalOrInteger::operator-(const IntervalOrInteger &rhs) {
  IntervalOrInteger ret;
  if (is_integer && rhs.is_integer) {
    ret.is_integer = true;
    ret.integer = integer - rhs.integer;
  } else {
    ret.is_integer = false;
    if (is_integer) {
      ret.interval_value = integer - rhs.interval_value;
    } else {
      if (rhs.is_integer) {
        ret.interval_value = interval_value - rhs.integer;
      } else {
        ret.interval_value = interval_value - rhs.interval_value;
      }
    }
  }
  return ret;
}

IntervalOrInteger IntervalOrInteger::operator*(const IntervalOrInteger &rhs) {
  IntervalOrInteger ret;
  if (is_integer && rhs.is_integer) {
    ret.is_integer = true;
    ret.integer = integer * rhs.integer;
  } else {
    ret.is_integer = false;
    if (is_integer) {
      ret.interval_value = integer * rhs.interval_value;
    } else {
      if (rhs.is_integer) {
        ret.interval_value = interval_value * rhs.integer;
      } else {
        ret.interval_value = interval_value * rhs.interval_value;
      }
    }
  }
  return ret;
}

IntervalOrInteger IntervalOrInteger::operator/(const IntervalOrInteger &rhs) {
  IntervalOrInteger ret;
  if (is_integer && rhs.is_integer &&
      (integer > rhs.integer && integer % rhs.integer == 0)) {
    ret.is_integer = true;
    ret.integer = integer / rhs.integer;
  } else {
    ret.is_integer = false;
    if (is_integer) {
      if (rhs.is_integer) {
        ret.interval_value = itvd(integer) / itvd(rhs.integer);
      } else {
        ret.interval_value = itvd(integer) / rhs.interval_value;
      }
    } else {
      if (rhs.is_integer) {
        ret.interval_value = interval_value / (itvd)rhs.integer;
      } else {
        ret.interval_value = interval_value / rhs.interval_value;
      }
    }
  }
  return ret;
}

IntervalOrInteger IntervalOrInteger::operator-() {
  IntervalOrInteger ret;
  ret.is_integer = is_integer;
  if (is_integer) {
    ret.integer = -integer;
  } else {
    ret.interval_value = -interval_value;
  }
  return ret;
}

} // namespace interval
} // namespace hydla
