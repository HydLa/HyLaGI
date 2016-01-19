#pragma once

#include "Simulator.h"
#include "ConsistencyChecker.h"
#include "Automaton.h"

namespace hydla {
namespace simulator {

typedef std::list<AutomatonNode*>            HA_node_list_t;

class HybridAutomatonConverter: public Simulator{
public:
  HybridAutomatonConverter(Opts &opts);
  virtual ~HybridAutomatonConverter();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_sptr_t simulate();
private:
  bool check_including(AutomatonNode* larger,AutomatonNode* smaller);
  void HA_translate(phase_result_sptr_t current, AutomatonNode * current_automaton_node, HA_node_list_t created_nodes);
  AutomatonNode* create_phase_node(phase_result_sptr_t phase);
  AutomatonNode* transition(AutomatonNode *current, phase_result_sptr_t phase, HA_node_list_t &trace_path);
  AutomatonNode* detect_loop(AutomatonNode *new_node, HA_node_list_t trace_path);
  Automaton current_automaton;
  std::list<Automaton> result_automata;
};

} // simulator
} // hydla
