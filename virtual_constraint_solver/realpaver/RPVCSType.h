#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_VCS_TYPE_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_VCS_TYPE_H_

#include "../VirtualConstraintSolver.h"

#include "RPVariable.h"
#include "RPValue.h"
#include "RPTime.h"

#include <boost/bimap/bimap.hpp>
#include <boost/bimap/set_of.hpp>

#define BP_PREV_STR "_p"
#define BP_DERIV_STR "_d"
#define BP_INITIAL_STR "_0"

namespace hydla {
namespace vcs {
namespace realpaver {

struct var_property {
public:
  var_property(int c, bool p) : derivative_count(c), prev_flag(p) {} 
  int derivative_count;
  bool prev_flag;
};

typedef boost::bimaps::bimap<
     boost::bimaps::set_of<std::string>,
     boost::bimaps::set_of<int>,
  boost::bimaps::with_info<var_property>
> var_name_map_t;


typedef hydla::vcs::VirtualConstraintSolver<
  RPVariable, RPValue, RPTime> virtual_constraint_solver_t;

} // namespace realpaver
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_VCS_TYPE_H_
