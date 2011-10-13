#ifndef _INCLUDED_RP_PROBLEM_EXT_H_
#define _INCLUDED_RP_PROBLEM_EXT_H_

#include <iostream>
#include "rp_problem.h"
#include "rp_container_ext.h"

namespace rp {

std::ostream& dump_problem(std::ostream& s, const rp_problem p,
                           const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

}

std::ostream& operator<<(std::ostream& s, const rp_problem p);

#endif //_INCLUDED_RP_PROBLEM_EXT_H_