#pragma once

#include "ConsistencyChecker.h"
#include "LTLNode.h"
#include "Simulator.h"
#include "SymbolicTrajPrinter.h"

namespace hydla {
namespace simulator {

typedef std::shared_ptr<hydla::simulator::ConsistencyChecker>
    consistency_checker_t;
typedef std::shared_ptr<hydla::symbolic_expression::Ask> ask_t;
typedef std::set<ask_t> asks_t;
typedef std::set<std::shared_ptr<hydla::symbolic_expression::Always>>
    always_set_t;
typedef hydla::hierarchy::ModuleSet module_set_t;
typedef std::shared_ptr<hydla::simulator::PhaseResult> phase_result_sptr_t;
typedef std::vector<phase_result_sptr_t> phase_result_sptrs_t;
typedef std::list<phase_result_sptr_t> phase_list_t;
typedef hydla::simulator::Value value_t;
typedef hydla::simulator::ValueRange range_t;
typedef hydla::simulator::Variable variable_t;
typedef hydla::simulator::Parameter parameter_t;
typedef std::map<variable_t, range_t, hydla::simulator::VariableComparator>
    variable_map_t;
typedef std::map<parameter_t, range_t, hydla::simulator::ParameterComparator>
    parameter_map_t;
typedef std::set<std::string> change_variables_t;
typedef std::map<hydla::simulator::constraint_t, bool> constraint_diff_t;
typedef std::map<module_set_t::module_t, bool> module_diff_t;
typedef hydla::hierarchy::ModuleSet module_set_t;
typedef std::set<module_set_t> module_set_set_t;

typedef std::vector<LTLNode *> ltl_node_list_t;
typedef std::vector<automaton_node_list_t> path_list_t;
typedef std::vector<ltl_node_list_t> ltl_path_list_t;
typedef std::pair<LTLNode *, node_sptr> ltl_edge_t;
typedef std::vector<ltl_edge_t> ltl_edge_list_t;

typedef struct current_checking_node {
  LTLNode *node;
  ltl_node_list_t created_nodes;
  ltl_path_list_t acceptance_path_list;
} current_checking_node_t;

typedef std::vector<current_checking_node_t> current_checking_node_list_t;

class LTLModelChecker : public Simulator {
public:
  LTLModelChecker(Opts &opts);
  virtual ~LTLModelChecker();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_sptr_t simulate();

private:
  void LTLsearch(phase_result_sptr_t current,
                 current_checking_node_list_t checking_list,
                 phase_list_t phase_list);
  bool check_including(LTLNode *larger, LTLNode *smaller);
  bool check_edge_guard(phase_result_sptr_t phase, node_sptr guard);
  bool check_including_wP(LTLNode *larger, LTLNode *smaller,
                          ConstraintStore par_cons);
  bool check_edge_guard_wP(phase_result_sptr_t phase, node_sptr guard,
                           ConstraintStore par_cons);

  current_checking_node_list_t transition(current_checking_node_list_t checking_list,
                                          phase_result_sptr_t phase);
  current_checking_node_list_t refresh(phase_list_t phase_list,
                                       ConstraintStore par_cons);
  LTLNode *detect_acceptance_cycle(LTLNode *new_node,
                                   ltl_path_list_t acceptance_path_list);
  LTLNode *detect_loop_in_path(LTLNode *new_node, ltl_node_list_t path);
  LTLNode *detect_acceptance_cycle_wP(LTLNode *new_node,
                                      ltl_path_list_t acceptance_path_list,
                                      ConstraintStore par_cons);
  LTLNode *detect_loop_in_path_wP(LTLNode *new_node, ltl_node_list_t path,
                                  ConstraintStore par_cons);

  /* phase_list_t get_path(phase_result_sptr_t phase); */

  Automaton current_automaton;
  std::list<Automaton> result_automata;
  int id_counter;
  std::shared_ptr<ConsistencyChecker> consistency_checker;
  PropertyNode *property_init;
};

} // namespace simulator
} // namespace hydla
