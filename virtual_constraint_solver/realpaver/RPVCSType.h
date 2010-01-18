#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_VCS_TYPE_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_VCS_TYPE_H_

#include "../VirtualConstraintSolver.h"

#include "RPVariable.h"
#include "RPValue.h"
#include "RPTime.h"

namespace hydla {
namespace vcs {
namespace realpaver {

typedef hydla::vcs::VirtualConstraintSolver<
  RPVariable, RPValue, RPTime> virtual_constraint_solver_t;

} // namespace realpaver
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_VCS_TYPE_H_
