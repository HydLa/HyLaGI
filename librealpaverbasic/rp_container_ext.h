#ifndef _INCLUDED_RP_CONTAINER_EXT_H_
#define _INCLUDED_RP_CONTAINER_EXT_H_

#include <iostream>
#include "rp_container.h"
#include "rp_constraint.h"
#include "rp_ext.h"

namespace rp {

template<typename T>
std::ostream& dump_vector(std::ostream& s, const rp_vector v,
                          const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND)
{
  if (rp_vector_size(v)==0) {
    s << "empty vector";
    return s;
  }
  for (int i=0; i<rp_vector_size(v); ++i) {
    rp::Dumper<T>::dump(s, reinterpret_cast<T>(rp_vector_elem(v,i)), digits, mode);
    s << "\n";
  }
  return s;
}

std::ostream& dump_vector_constraint(std::ostream& s, const rp_vector_constraint v,
                                     const rp_vector_variable vars,
                                     const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

}

#endif //_INCLUDED_RP_CONTAINER_EXT_H_