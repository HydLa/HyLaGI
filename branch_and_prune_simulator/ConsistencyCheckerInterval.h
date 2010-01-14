#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_

#include "ConstraintBuilder.h"
#include "ConstraintStoreInterval.h"

namespace hydla {
namespace bp_simulator {

class ConsistencyCheckerInterval : public ConstraintBuilder {
public:
  ConsistencyCheckerInterval();

  virtual ~ConsistencyCheckerInterval();

  bool is_consistent(simulator::tells_t& collected_tells,
    ConstraintStoreInterval& constraint_store);

  // TellêßñÒ
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

private:
  std::set<rp_constraint> constraints_;

};

}
}

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_