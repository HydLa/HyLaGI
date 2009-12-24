#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_H_

#include "ConstraintBuilder.h"

//#define RP_RELATION_EQUAL     1
//#define RP_RELATION_SUPEQUAL  2
//#define RP_RELATION_INFEQUAL  3
//#define RP_RELATION_UNEQUAL 4
//#define RP_RELATION_SUP 5
//#define RP_RELATION_INF 6

namespace hydla {
namespace bp_simulator {

class ConsistencyChecker : public ConstraintBuilder {
public:
  ConsistencyChecker();
  ConsistencyChecker(bool debug_mode);

  virtual ~ConsistencyChecker();

  bool is_consistent(hydla::simulator::tells_t& collected_tells);

  // TellêßñÒ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

private:
  rp_vector_variable to_rp_vector();

  std::set<rp_constraint> constraints_;

  bool debug_mode_;

};

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_H__

