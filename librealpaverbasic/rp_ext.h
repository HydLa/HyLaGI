#ifndef _INCLUDED_RP_EXT_H_
#define _INCLUDED_RP_EXT_H_

#include <iostream>
#include "rp_interval.h"
#include "rp_constant.h"
#include "rp_variable.h"
#include "rp_union_interval_ext.h"

namespace rp {

template<typename T>
struct Dumper;
/*
std::ostream& dump(std::ostream& s, const T v,
                   const int digits, const int mode)
{
  return s;
}
*/

// constant
template<>
struct Dumper<rp_constant>
{
static std::ostream& dump(std::ostream& s, const rp_constant c,
                   const int digits, const int mode)
{
  s << rp_constant_name(c) << " := ";
  char tmp[255];
  rp_interval_print(tmp, rp_constant_val(c), digits, mode);
  s << tmp;
  return s;
}
};

// variable
template<>
struct Dumper<rp_variable>
{
static std::ostream& dump(std::ostream& s, const rp_variable v,
                   const int digits, const int mode)
{
  s << rp_variable_name(v);
  if(rp_variable_integer(v)) {
    s << ":int";
  } else {
    s << ":real/" << rp_variable_precision(v);
  }
  s << " ~ ";
  rp::dump_union_interval(s, rp_variable_domain(v), digits, mode);
  s << " occurs in " << rp_variable_constrained(v) << " constraints";
  return s;
}
};

}

#endif //_INCLUDED_RP_EXT_H_