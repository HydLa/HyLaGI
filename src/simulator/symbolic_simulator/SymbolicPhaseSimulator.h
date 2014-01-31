#ifndef _INCLUDED_SYMBOLIC_PHASE_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_PHASE_SIMULATOR_H_

#include <string>
#include <iostream>
#include <fstream>

#include <sstream>
#include <stack>

#include "ParseTree.h"

#include "Simulator.h"

#include "SymbolicTypes.h"
#include "PhaseSimulator.h"
#include "../output/TrajPrinter.h"
#include "UnsatCoreFinder.h"

namespace hydla {
namespace simulator {
namespace symbolic {

class AnalysisResultChecker;

class SymbolicPhaseSimulator : public simulator_t
{
public:
  typedef simulator::Opts Opts;
  typedef simulator::ConditionsResult ConditionsResult;
  typedef simulator::Phase                 Phase;
  typedef simulator::phase_result_sptr_t   phase_result_sptr_t;
  typedef ch::module_set_sptr              modulse_set_sptr;
  typedef std::set< std::string > change_variables_t;

  SymbolicPhaseSimulator(Simulator* simulator, const Opts& opts);
  virtual ~SymbolicPhaseSimulator();

  virtual void initialize(variable_set_t &v, parameter_map_t &p, variable_map_t &m, continuity_map_t& c, parse_tree_sptr pt, const module_set_container_sptr& msc);

  virtual void init_arc(const parse_tree_sptr& parse_tree);


  virtual void find_unsat_core(const module_set_sptr& ms,
      simulation_todo_sptr_t&,
      const variable_map_t& vm);


private:

  std::set<module_set_sptr> checkd_module_set_;
  
  /**
   * PPモードとIPモードを切り替える
   */

  virtual void set_simulation_mode(const Phase& phase);
  

  /**
   * 与えられた制約モジュール集合の閉包計算を行い，無矛盾性を判定するとともに対応する変数表を返す．
   */

  virtual simulator::CalculateVariableMapResult calculate_variable_map(const module_set_sptr& ms,
                           simulation_todo_sptr_t& state, const variable_map_t &, variable_maps_t& result_vm);

  void apply_discrete_causes_to_guard_judgement( ask_set_t& discrete_causes,
                                                 positive_asks_t& positive_asks,
                                                 negative_asks_t& negative_asks,
                                                 ask_set_t& unknown_asks );

void set_changing_variables( const phase_result_sptr_t& parent_phase,
                             const module_set_sptr& present_ms,
                             const positive_asks_t& positive_asks,
                             const negative_asks_t& negative_asks,
                             change_variables_t& changing_variables );

  void set_changed_variables(phase_result_sptr_t& phase);  

  change_variables_t get_difference_variables_from_2tells(const tells_t& larg, const tells_t& rarg);

  bool apply_entailment_change( const ask_set_t::iterator it,
                                const ask_set_t& previous_asks,
                                const bool in_IP,
                                change_variables_t& changing_variables,
                                ask_set_t& notcv_unknown_asks,
                                ask_set_t& unknown_asks );

void apply_previous_solution(const change_variables_t& changing_variables,
                             const ask_set_t::iterator it,
                             const bool in_IP,
                             const phase_result_sptr_t parent );

  /**
   * 与えられたフェーズの次のTodoを返す．
   */
  virtual todo_list_t make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo);

  bool calculate_closure(simulation_todo_sptr_t& state,
    const module_set_sptr& ms);

  /**
   * Check whether a guard is entailed or not.
   * If the entailment depends on the condition of variables or parameters, return BRANHC_VAR or BRANCH_PAR.
   * If the return value is BRANCH_PAR, the value of cc_result consists of cases the guard is entailed and cases the guard is not entailed.
   */
  CheckEntailmentResult check_entailment(
    CheckConsistencyResult &cc_result,
    const node_sptr& guard,
    const continuity_map_t& cont_map,
    const Phase& phase);
  
  CheckConsistencyResult check_consistency(const Phase &phase);
  
  void add_continuity(const continuity_map_t&, const Phase &phase);
  
  virtual module_set_list_t calculate_mms(
    simulation_todo_sptr_t& state,
    const variable_map_t& vm);

  virtual void mark_nodes_by_unsat_core(const modulse_set_sptr& ms,
      simulation_todo_sptr_t&,
    const variable_map_t& vm);

  virtual void set_backend(backend_sptr_t back);

  virtual simulator::CalculateVariableMapResult check_conditions(const module_set_sptr& ms, simulation_todo_sptr_t&, const variable_map_t &, bool b);
  
  virtual variable_map_t apply_time_to_vm(const variable_map_t &vm, const value_t &tm);

  void replace_prev2parameter(phase_result_sptr_t& state,
                              variable_map_t& vm,
                              parameter_map_t &parameter_map);
  
  continuity_map_t variable_derivative_map_;
  
  boost::shared_ptr<AnalysisResultChecker > analysis_result_checker_;
  boost::shared_ptr<UnsatCoreFinder > unsat_core_finder_;
  
  Phase current_phase_;
};

} // namespace symbolic
} // namespace simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_PHASE_SIMULATOR_H_
