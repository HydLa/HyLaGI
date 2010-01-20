#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_BASE_VCS_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_BASE_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "RPVCSType.h"

namespace hydla {
namespace vcs {
namespace realpaver {

class RealPaverBaseVCS : 
  public virtual_constraint_solver_t
{
public:
  typedef boost::shared_ptr<hydla::parse_tree::Node> node_sptr;

  RealPaverBaseVCS(){};

  virtual ~RealPaverBaseVCS(){};

/******************** realpaver only ********************/

  virtual RealPaverBaseVCS* clone() = 0;

  virtual void add_single_constraint(const node_sptr& constraint_node,
    const bool neg_expression) = 0;

/******************** realpaver only ********************/

};

} // namespace realpaver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_BASE_VCS_H_
