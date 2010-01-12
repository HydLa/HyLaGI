#ifndef _INCLUDED_RP_EXPRESSION_SYMBOL_EXT_H_
#define _INCLUDED_RP_EXPRESSION_SYMBOL_EXT_H_

#include <iostream>
#include "rp_expression_symbol.h"

namespace rp {

std::ostream& dump_erep(std::ostream& s, const rp_erep f,
                        const rp_vector_variable vars,
                        const int digits, const int mode);

} // namespace rp

#endif // _INCLUDED_RP_EXPRESSION_SYMBOL_EXT_H_