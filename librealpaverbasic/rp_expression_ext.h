#ifndef _INCLUDED_RP_EXPRESSION_EXT_H_
#define _INCLUDED_RP_EXPRESSION_EXT_H_

#include <iostream>
#include "rp_expression.h"

namespace rp {

std::ostream& dump_expression(std::ostream& s, const rp_expression e,
                   const rp_vector_variable vars,
                   const int digits=10, const int mode=RP_INTERVAL_MODE_BOUND);

}

#endif //_INCLUDED_RP_EXPRESSION_EXT_H_