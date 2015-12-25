#pragma once

#include "Simulator.h"
#include "SymbolicTrajPrinter.h"
#include "ConsistencyChecker.h"
#include "HybridAutomaton.h"

namespace hydla {
namespace simulator {

class HybridAutomatonConverter: public Simulator{
public:
  HybridAutomatonConverter(Opts &opts);
  virtual ~HybridAutomatonConverter();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_sptr_t simulate();
private:
  bool check_including(HybridAutomaton* larger,HybridAutomaton* smaller);
  void HA_translate(phase_result_sptr_t current, HA_node_list_t current_automaton_node);
  HA_node_list_t transition(HA_node_list_t current,phase_result_sptr_t phase,consistency_checker_t consistency_checker);
  HybridAutomaton* detect_loop_in_path(HybridAutomaton* new_node, automaton_node_list_t path);
  io::SymbolicTrajPrinter printer;
  boost::shared_ptr<ConsistencyChecker> consistency_checker;
};

} // simulator
} // hydla
