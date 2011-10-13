#include "rp_container_ext.h"
#include "rp_constraint_ext.h"

namespace rp {

std::ostream& dump_vector_constraint(std::ostream& s, const rp_vector_constraint v,
                   const rp_vector_variable vars,
                   const int digits, const int mode)
{
  if(rp_vector_size(v)==0) {
    s << "empty vector";
    return s;
  }
  for(int i=0; i<rp_vector_size(v); ++i) {
    rp::dump_constraint(s, reinterpret_cast<rp_constraint>(rp_vector_elem(v,i)),
      vars, digits, mode);
    s << "\n";
  }
  return s;
}

}
