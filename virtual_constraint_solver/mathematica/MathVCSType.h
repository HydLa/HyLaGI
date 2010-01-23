#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_VCS_TYPE_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_VCS_TYPE_H_

#include "../VirtualConstraintSolver.h"

#include "MathVariable.h"
#include "MathValue.h"
#include "MathTime.h"

namespace hydla {
namespace vcs {
namespace mathematica {

typedef hydla::vcs::VirtualConstraintSolver<
  MathVariable, MathValue, MathTime> virtual_constraint_solver_t;

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATH_VCS_TYPE_H_
