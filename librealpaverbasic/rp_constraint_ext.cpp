#include "rp_constraint_ext.h"
#include "rp_expression_ext.h"
#include "rp_union_interval_ext.h"

namespace rp {

std::ostream& dump_ctr_num(std::ostream& s, const rp_ctr_num c,
                           const rp_vector_variable vars,
                           const int digits, const int mode)
{
  rp::dump_expression(s, rp_ctr_num_left(c), vars, digits, mode);
  switch(rp_ctr_num_rel(c)) {
  case RP_RELATION_EQUAL:
    s << "=";
    break;
  case RP_RELATION_SUPEQUAL:
    s << ">=";
    break;
  case RP_RELATION_INFEQUAL:
    s << "<=";
    break;
  }
  rp::dump_expression(s, rp_ctr_num_right(c), vars, digits, mode);
  return s;
}

std::ostream& dump_ctr_cond(std::ostream& s, const rp_ctr_cond c,
                            const rp_vector_variable vars,
                            const int digits, const int mode)
{
  for(int i=0; i<rp_ctr_cond_guardsize(c); ++i) {
    if(i>0) s << " # ";
    rp::dump_ctr_num(s, rp_ctr_cond_guard_elem(c,i), vars, digits, mode);
  }
  s << "->";
  for(int i=0; i<rp_ctr_cond_concsize(c); ++i) {
    if(i>0) s << " # ";
    rp::dump_ctr_num(s, rp_ctr_cond_conc_elem(c,i), vars, digits, mode);
  }
  return s;
}

std::ostream& dump_ctr_piecewise(std::ostream& s, const rp_ctr_piecewise c,
                                 const rp_vector_variable vars,
                                 const int digits, const int mode)
{
  s << "piecewise(" << rp_vector_variable_elem(vars, rp_ctr_piecewise_var(c)) << ", ";
  for(int i=0; i<rp_ctr_piecewise_arity(c); ++i) {
    char tmp[255];
    rp_interval_print(tmp, rp_ctr_piecewise_elem_dom(c,i), digits, mode);
    s << tmp << ": ";
    for(int j=0; j<rp_ctr_piecewise_elem_size(c,i); ++j) {
      rp::dump_ctr_num(s, rp_ctr_piecewise_elem_ctrnum(c,i,j), vars, digits, mode);
      if(j<rp_ctr_piecewise_elem_size(c,i)-1) s << " # ";
    }
    if(i<rp_ctr_piecewise_arity(c)-1) s << " , ";
  }
  s << ")";
  s << " pieces: ";
  rp::dump_union_interval(s, rp_ctr_piecewise_guard(c), digits, mode);
  return s;
}

std::ostream& dump_constraint(std::ostream& s, const rp_constraint c,
                              const rp_vector_variable vars,
                              const int digits, const int mode)
{
  switch(rp_constraint_type(c)) {
  case RP_CONSTRAINT_NUMERICAL:
    rp::dump_ctr_num(s, rp_constraint_num(c), vars, digits, mode);
    break;
  case RP_CONSTRAINT_CONDITIONAL:
    rp::dump_ctr_cond(s, rp_constraint_cond(c), vars, digits, mode);
    break;
  case RP_CONSTRAINT_PIECEWISE:
    rp::dump_ctr_piecewise(s, rp_constraint_piece(c), vars, digits, mode);
    break;
  }
  return s;
}

}