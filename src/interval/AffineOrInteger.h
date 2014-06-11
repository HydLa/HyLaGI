#pragma once

#include "kv/affine.hpp"

namespace hydla {
namespace interval {

typedef kv::affine<double>                    affine_t;

/**
 * A class to keep integer as long as possible
 */
class AffineOrInteger
{
  public:
  affine_t affine_value;
  int integer;
  bool is_integer;

  AffineOrInteger operator+(const AffineOrInteger &rhs);
  AffineOrInteger operator-(const AffineOrInteger &rhs);
  AffineOrInteger operator*(const AffineOrInteger &rhs);
  AffineOrInteger operator/(const AffineOrInteger &rhs);
  AffineOrInteger operator-();
};

std::ostream& operator<<(std::ostream &ost, const AffineOrInteger &val);

} //namespace interval
} //namespace hydla
