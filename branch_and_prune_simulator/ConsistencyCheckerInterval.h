#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_

#include "BPTypes.h"
#include "ConstraintStoreInterval.h"

// parser
#include "Node.h"
#include "TreeVisitor.h"
#include "ParseTree.h"

// simulator
#include "TellCollector.h"

// librealpaverbasic
#include "realpaverbasic.h"

//#include "../PacketSender.h"

namespace hydla {
namespace bp_simulator {
/*
class ConsistencyCheckerInterval {
public:
  ConsistencyCheckerInterval(MathLink& ml);

  virtual ~ConsistencyCheckerInterval();

  bool is_consistent(simulator::tells_t& collected_tells,
    ConstraintStoreInterval& constraint_store);

private:
  //hydla::vcs::mathematica::PacketSender packet_sender_;
  MathLink& ml_;
  var_name_map_t vars_;
  rp_ctr_num ctr_;
  //bool in_prev_;
  //unsigned int derivative_count_;
  std::set<rp_constraint> constraints_;
  //MathLink ml;
  std::string send_expression_str;

};
*/

}
}

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_CONSISTENCY_CHECKER_INTERVAL_H_