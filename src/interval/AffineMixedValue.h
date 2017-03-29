#pragma once

#include "kv/affine.hpp"

namespace hydla {
namespace interval {

typedef kv::affine<double>                    affine_t;
typedef kv::interval<double>                  itvd;


typedef enum
{
  INTEGER,
  INTERVAL,
  AFFINE
}ValueType;

/**
 * A class to keep integer as long as possible
 */
class AffineMixedValue
{
  public:
  affine_t affine_value;
  int integer;
  itvd interval;
  ValueType type;

  AffineMixedValue();
  AffineMixedValue(int i);
  AffineMixedValue(itvd itv);
  AffineMixedValue(affine_t a);

  AffineMixedValue operator+(const AffineMixedValue &rhs) const;
  AffineMixedValue operator-(const AffineMixedValue &rhs) const;
  AffineMixedValue operator*(const AffineMixedValue &rhs) const;
  AffineMixedValue operator/(const AffineMixedValue &rhs) const;;
  AffineMixedValue operator^(const AffineMixedValue &rhs) const;;
  
  AffineMixedValue operator-() const;

  itvd to_interval() const;
  affine_t to_affine() const;
};

std::ostream& operator<<(std::ostream &ost, const AffineMixedValue &val);

} //namespace interval
} //namespace hydla
