#include "rp_problem_ext.h"

namespace rp {

std::ostream& dump_problem(std::ostream& s, const rp_problem p,
                           const int digits, const int mode)
{
  s << "Problem " << rp_problem_name(p) << "\n";
  s << "--- nums:\n";
  rp::dump_vector<rp_constant>(s, rp_problem_nums(p), digits, mode);
  s << "--- vars:\n";
  rp::dump_vector<rp_variable>(s, rp_problem_vars(p), digits, mode);
  s << "--- ctrs:\n";
  rp::dump_vector_constraint(s, rp_problem_ctrs(p), rp_problem_vars(p), digits, mode);
  return s;
}

}

std::ostream& operator<<(std::ostream& s, const rp_problem p)
{
  return rp::dump_problem(s, p);
}
