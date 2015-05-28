#pragma once

#include "Simulator.h"
#include "LTLNode.h"
#include "SymbolicTrajPrinter.h"
#include "ConsistencyChecker.h"

namespace hydla {
namespace simulator {

class LTLModelChecker: public Simulator{
public:
  LTLModelChecker(Opts &opts);
  virtual ~LTLModelChecker();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_sptr_t simulate();
private:
  void LTLsearch(phase_result_sptr_t current,ltl_node_list_t ltl_current,LTLNode* result_init,PropertyNode* property_init);
  ltl_node_list_t transition(ltl_node_list_t current,phase_result_sptr_t phase,consistency_checker_t consistency_checker);
  bool check_including(LTLNode* larger,LTLNode* smaller);
  bool check_edge_guard(phase_result_sptr_t phase,node_sptr guard,consistency_checker_t consistency_checker);

  io::SymbolicTrajPrinter printer;
  boost::shared_ptr<ConsistencyChecker> consistency_checker;
};

} // simulator
} // hydla
