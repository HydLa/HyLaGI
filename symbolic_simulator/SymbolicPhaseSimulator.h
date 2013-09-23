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
#include "../solver/SymbolicSolver.h"
#include "PhaseSimulator.h"
#include "../output/TrajPrinter.h"

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

  SymbolicPhaseSimulator(Simulator* simulator, const Opts& opts);
  virtual ~SymbolicPhaseSimulator();

  virtual void initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c, const module_set_container_sptr& msc);
  virtual parameter_set_t get_parameter_set();

  virtual void init_arc(const parse_tree_sptr& parse_tree);

private:

  std::set<module_set_sptr> checkd_module_set_;

  variable_map_t shift_variable_map_time(const variable_map_t& vm,const time_t &time);
  
  /**
   * PPモードとIPモードを切り替える
   */

  virtual void set_simulation_mode(const Phase& phase);
  

  /**
   * 与えられた制約モジュール集合の閉包計算を行い，無矛盾性を判定するとともに対応する変数表を返す．
   */

  virtual simulator::CalculateVariableMapResult calculate_variable_map(const module_set_sptr& ms,
                           simulation_todo_sptr_t& state, const variable_map_t &, variable_maps_t& result_vm);

  void set_changed_variables(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo);  

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
    hydla::solver::CheckConsistencyResult &cc_result,
    const node_sptr& guard,
    const continuity_map_t& cont_map);

  void add_continuity(const continuity_map_t&);
  
  virtual module_set_list_t calculate_mms(
    simulation_todo_sptr_t& state,
    const variable_map_t& vm);

  virtual simulator::CalculateVariableMapResult check_conditions(const module_set_sptr& ms, simulation_todo_sptr_t&, const variable_map_t &, bool b);

  
  virtual variable_map_t apply_time_to_vm(const variable_map_t &vm, const time_t &tm)
  {
    variable_map_t ret;
    solver_->apply_time_to_vm(vm, ret, tm);
    return ret;
  }
  
  continuity_map_t variable_derivative_map_;
  
  boost::shared_ptr<AnalysisResultChecker > analysis_result_checker_;
  
  Phase current_phase_;
};

} // namespace symbolic
} // namespace simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_PHASE_SIMULATOR_H_
