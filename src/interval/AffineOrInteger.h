#ifndef _INCLUDED_HYDLA_AFFINE_OR_INTEGER_H_
#define _INCLUDED_HYDLA_AFFINE_OR_INTEGER_H_

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
  AffineOrInteger operator+(const AffineOrInteger &rhs)
  {
    AffineOrInteger ret;
    if(is_integer && rhs.is_integer)
    {
      ret.is_integer = true;
      ret.integer = integer + rhs.integer;
    }
    else
    {
      ret.is_integer = false;
      if(is_integer)
      {
        ret.affine_value = integer + rhs.affine_value;
      }
      else
      {
        if(rhs.is_integer)
        {
          ret.affine_value = affine_value + rhs.integer;
        }
        else
        {
          ret.affine_value = affine_value + rhs.affine_value;
        }
      }
    }
    return ret;
  }
  AffineOrInteger operator-(const AffineOrInteger &rhs)
  {
    AffineOrInteger ret;
    if(is_integer && rhs.is_integer)
    {
      ret.is_integer = true;
      ret.integer = integer - rhs.integer;
    }
    else
    {
      ret.is_integer = false;
      if(is_integer)
      {
        ret.affine_value = integer - rhs.affine_value;
      }
      else
      {
        if(rhs.is_integer)
        {
          ret.affine_value = affine_value - rhs.integer;
        }
        else
        {
          ret.affine_value = affine_value - rhs.affine_value;
        }
      }
    }
    return ret;
  }
  AffineOrInteger operator*(const AffineOrInteger &rhs)
  {
    AffineOrInteger ret;
    if(is_integer && rhs.is_integer)
    {
      ret.is_integer = true;
      ret.integer = integer * rhs.integer;
    }
    else
    {
      ret.is_integer = false;
      if(is_integer)
      {
        ret.affine_value = integer * rhs.affine_value;
      }
      else
      {
        if(rhs.is_integer)
        {
          ret.affine_value = affine_value * rhs.integer;
        }
        else
        {
          ret.affine_value = affine_value * rhs.affine_value;
        }
      }
    }
    return ret;
  }
  AffineOrInteger operator/(const AffineOrInteger &rhs)
  {
    AffineOrInteger ret;
    if(is_integer && rhs.is_integer &&
       ((integer > rhs.integer && integer % rhs.integer == 0)
        || (integer < rhs.integer && rhs.integer % integer == 0)
       )
      )
    {
      ret.is_integer = true;
      ret.integer = integer / rhs.integer;
    }
    else
    {
      ret.is_integer = false;
      if(is_integer)
      {
        ret.affine_value = (double)integer * rhs.affine_value;
      }
      else
      {
        if(rhs.is_integer)
        {
          ret.affine_value = affine_value * (double)rhs.integer;
        }
        else
        {
          ret.affine_value = affine_value * rhs.affine_value;
        }
      }
    }
    return ret;
  }
  AffineOrInteger operator-()
  {
    AffineOrInteger ret;
    ret.is_integer = is_integer;
    if(is_integer)
    {
      ret.integer = -integer;
    }
    else
    {
      ret.affine_value = -affine_value;
    }
    return ret;
  }

  std::ostream& operator<<(std::ostream &ost)
  {
    if(is_integer)
    {
      ost << "integer: " << integer;
    }else
    {
      ost << "affine: " << affine_value;
    }
    return ost;
  }
};

} //namespace interval
} //namespace hydla

#endif // include guard
