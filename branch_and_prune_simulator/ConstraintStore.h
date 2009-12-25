#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_

#include <set>
#include <boost/bimap/bimap.hpp>
#include "realpaverbasic.h"

namespace hydla {
namespace bp_simulator {

/**
 * êßñÒÉXÉgÉA
 */
class ConstraintStore
{
public:
  ConstraintStore();
  ~ConstraintStore();

private:
  std::set<rp_constraint> exprs_;
  boost::bimaps::bimap<std::string, int> vars_;
};

} // namespace bp_simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_CONSTRAINT_STORE_H_