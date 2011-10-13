#include "rp_expression_ext.h"
#include "rp_expression_symbol_ext.h"

namespace rp {

std::ostream& dump_expression(std::ostream& s, const rp_expression e,
                   const rp_vector_variable vars,
                   const int digits, const int mode)
{
  rp::dump_erep(s, rp_expression_rep(e), vars, digits, mode);
  return s;
}

}
