#ifndef _INCLUDED_SYMBOLIC_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_SIMULATOR_H_

#include <string>
#include <iostream>
#include <fstream>

#include <sstream>
#include <stack>

#include "ParseTree.h"

#include "Simulator.h"

#include "Types.h"
#include "SymbolicTypes.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"
#include "PhaseSimulator.h"
#include "../output/TrajPrinter.h"

namespace hydla {
namespace symbolic_simulator {


class SymbolicSimulator : public simulator_t
{
public:
  typedef hydla::vcs::SymbolicVirtualConstraintSolver solver_t;
  typedef simulator::Opts Opts;

  SymbolicSimulator(const Opts& opts);
  virtual ~SymbolicSimulator();

  virtual FalseConditionsResult find_false_conditions(const module_set_sptr& ms);

  virtual todo_and_results_t simulate_ms_point(const module_set_sptr& ms,
                           simulation_phase_sptr_t& state, variable_map_t &vm, bool& consistent);
  
  virtual todo_and_results_t simulate_ms_interval(const module_set_sptr& ms,
                              simulation_phase_sptr_t& state, bool& consistent);

  virtual void initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c);
  virtual void set_parameter_set(parameter_t param);
  virtual parameter_set_t get_parameter_set();

private:

  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;

  variable_map_t range_map_to_value_map(phase_result_sptr_t&,
    const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t &,
    parameter_map_t &);

  variable_t* get_variable(const std::string &name, const int &derivative_count){
    return &(*std::find(variable_set_->begin(), variable_set_->end(), (variable_t(name, derivative_count))));
  }
  
  variable_map_t shift_variable_map_time(const variable_map_t& vm,const time_t &time);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  CalculateClosureResult calculate_closure(simulation_phase_sptr_t& state,
    const module_set_sptr& ms);
  /**
   * Check whether a guard is entailed or not.
   * If the entailment depends on the condition of variables or parameters, return BRANHC_VAR or BRANCH_PAR.
   * If the return value is BRANCH_PAR, the value of cc_result consists of cases the guard is entailed and cases the guard is not entailed.
   */
  CheckEntailmentResult check_entailment(
    hydla::vcs::SymbolicVirtualConstraintSolver::check_consistency_result_t &cc_result,
    const node_sptr& guard,
    const continuity_map_t& cont_map);

  void push_branch_states(simulation_phase_sptr_t &original,
    hydla::vcs::SymbolicVirtualConstraintSolver::check_consistency_result_t &result,
    CalculateClosureResult &dst);

  void add_continuity(const continuity_map_t&);
  
  virtual variable_map_t apply_time_to_vm(const variable_map_t &vm, const time_t &tm)
  {
    variable_map_t ret;
    solver_->apply_time_to_vm(vm, ret, tm);
    return ret;
  }
  
  continuity_map_t variable_derivative_map_;

  /// 使用するソルバへのポインタ
  boost::shared_ptr<solver_t> solver_;
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_
