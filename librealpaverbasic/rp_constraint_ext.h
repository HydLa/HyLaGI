#ifndef _INCLUDED_RP_CONSTRAINT_EXT_H_
#define _INCLUDED_RP_CONSTRAINT_EXT_H_

#include <iostream>
#include "rp_constraint.h"

namespace rp {

std::ostream& dump_ctr_num(std::ostream& s, const rp_ctr_num c,
                           const rp_vector_variable vars,
                           const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

std::ostream& dump_ctr_cond(std::ostream& s, const rp_ctr_cond c,
                            const rp_vector_variable vars,
                            const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

std::ostream& dump_ctr_piecewise(std::ostream& s, const rp_ctr_piecewise c,
                                 const rp_vector_variable vars,
                                 const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

std::ostream& dump_constraint(std::ostream& s, const rp_constraint c,
                              const rp_vector_variable vars,
                              const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

}

#endif //_INCLUDED_RP_CONSTRAINT_EXT_H_