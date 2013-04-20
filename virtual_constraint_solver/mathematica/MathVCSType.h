#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_VCS_TYPE_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_VCS_TYPE_H_

#include "../SymbolicVirtualConstraintSolver.h"

#include "MathVariable.h"
#include "MathValue.h"
#include "MathTime.h"

namespace hydla {
namespace vcs {
namespace mathematica {

typedef hydla::vcs::SymbolicVirtualConstraintSolver virtual_constraint_solver_t;
typedef hydla::vcs::SymbolicVirtualConstraintSolver::value_t value_t;

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_VCS_TYPE_H_
