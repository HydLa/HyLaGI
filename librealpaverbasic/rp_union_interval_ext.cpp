#include "rp_union_interval_ext.h"

namespace rp {

std::ostream& dump_union_interval(std::ostream& s, const rp_union_interval u,
                                  const int digits, const int mode)
{
  if(rp_union_card(u)==0) {
    s << "empty";
    return s;
  }
  s << "{";
  char tmp[255];
  rp_interval_print(tmp, rp_union_elem(u,0), digits, mode);
  s << tmp;
  for(int i=1; i<rp_union_card(u); i++) {
    s << ",";
    char tmp[255];
    rp_interval_print(tmp, rp_union_elem(u,i), digits, mode);
  }
  s << "}";
  return s;
}

}
