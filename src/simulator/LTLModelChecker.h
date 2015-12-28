#pragma once

#include "Simulator.h"
#include "LTLNode.h"
#include "SymbolicTrajPrinter.h"
#include "ConsistencyChecker.h"

namespace hydla {
namespace simulator {

typedef boost::shared_ptr<hydla::simulator::ConsistencyChecker>  consistency_checker_t;
typedef boost::shared_ptr<hydla::symbolic_expression::Ask>                       ask_t;
typedef std::set<ask_t >                                                        asks_t;
typedef std::set<boost::shared_ptr<hydla::symbolic_expression::Always> >  always_set_t;
typedef hydla::hierarchy::ModuleSet                                       module_set_t;
typedef boost::shared_ptr<hydla::simulator::PhaseResult>           phase_result_sptr_t;
typedef std::vector<phase_result_sptr_t >                         phase_result_sptrs_t;
typedef std::list<phase_result_sptr_t >                                    phase_list_t;
typedef hydla::simulator::Value                                                value_t;
typedef hydla::simulator::ValueRange                                           range_t;
typedef hydla::simulator::Variable                                          variable_t;
typedef hydla::simulator::Parameter                                        parameter_t;
typedef std::map<variable_t, range_t, hydla::simulator::VariableComparator>          variable_map_t;
typedef std::map<parameter_t, range_t, hydla::simulator::ParameterComparator>       parameter_map_t;
typedef std::set<std::string>                                                    change_variables_t;
typedef std::map<hydla::simulator::constraint_t, bool>                            constraint_diff_t;
typedef std::map<module_set_t::module_t, bool>                                        module_diff_t;
typedef hydla::hierarchy::ModuleSet                                                    module_set_t;
typedef std::set<module_set_t>                                                     module_set_set_t;

typedef std::vector<LTLNode*>            ltl_node_list_t;
typedef std::vector<automaton_node_list_t>   path_list_t;
typedef std::pair<LTLNode*,node_sptr>         ltl_edge_t;
typedef std::vector<ltl_edge_t>          ltl_edge_list_t;


class LTLModelChecker: public Simulator{
public:
  LTLModelChecker(Opts &opts);
  virtual ~LTLModelChecker();
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_sptr_t simulate();
private:
  void LTLsearch(phase_result_sptr_t current,ltl_node_list_t ltl_current,LTLNode* result_init);
  ltl_node_list_t transition(ltl_node_list_t current,phase_result_sptr_t phase,consistency_checker_t consistency_checker);
  bool check_including(LTLNode* larger,LTLNode* smaller);
  LTLNode* detect_acceptance_cycle(LTLNode* new_node,LTLNode* parent_node);
  LTLNode* detect_loop_in_path(LTLNode* new_node, automaton_node_list_t path);
  bool check_edge_guard(phase_result_sptr_t phase,node_sptr guard,consistency_checker_t consistency_checker);

  io::SymbolicTrajPrinter printer;
  boost::shared_ptr<ConsistencyChecker> consistency_checker;
};

} // simulator
} // hydla
