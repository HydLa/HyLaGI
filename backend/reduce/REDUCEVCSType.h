#ifndef _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VCS_TYPE_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VCS_TYPE_H_

#include "../SymbolicVirtualConstraintSolver.h"

#include "REDUCELink.h"
#include "REDUCETime.h"
#include "REDUCEValue.h"
#include "REDUCEVariable.h"
#include <boost/shared_ptr.hpp>

namespace hydla {
namespace vcs {
namespace reduce {

typedef hydla::vcs::SymbolicVirtualConstraintSolver virtual_constraint_solver_t;
typedef boost::shared_ptr<REDUCELink> reduce_link_t;

} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VCS_TYPE_H_
