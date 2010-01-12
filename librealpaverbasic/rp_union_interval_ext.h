#ifndef _INCLUDED_RP_UINION_INTERVAL_EXT_H_
#define _INCLUDED_RP_UINION_INTERVAL_EXT_H_

#include <iostream>
#include "rp_union_interval.h"

namespace rp {

std::ostream& dump_union_interval(std::ostream& s, const rp_union_interval u,
                   const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

}

#endif //_INCLUDED_RP_UINION_INTERVAL_EXT_H_