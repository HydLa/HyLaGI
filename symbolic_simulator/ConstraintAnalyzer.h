#ifndef _INCLUDED_HYDLA_CONSTRAINT_ANALYZER_H_
#define _INCLUDED_HYDLA_CONSTRAINT_ANALYZER_H_

#include "SymbolicTypes.h"

#include "PhaseSimulator.h"
#include "ParseTree.h"
#include "SymbolicSimulator.h"
#include "Simulator.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"

namespace hydla{
namespace symbolic_simulator{

class ConstraintAnalyzer
{
public:
  typedef enum{
    FALSE_CONDITIONS_TRUE,
    FALSE_CONDITIONS_FALSE,
    FALSE_CONDITIONS_VARIABLE_CONDITIONS
  } FalseConditionsResult;
  
  typedef std::map<std::string, node_sptr> false_map_t;

  ConstraintAnalyzer();
  virtual ~ConstraintAnalyzer();

  virtual void output_false_conditions();

  virtual FalseConditionsResult find_false_conditions(const module_set_sptr& ms, const Opts& opts);

  virtual void check_all_module_set(module_set_container_sptr& msc_no_init, const Opts& opts);

  virtual void initialize(module_set_container_sptr msc_no_init, boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver);

  virtual SymbolicSimulator::CalculateVariableMapResult check_false_conditions(const Opts& opts, const module_set_sptr& ms, simulation_phase_sptr_t& state, const variable_map_t&, variable_map_t& result_vm, todo_and_results_t& result_todo);

private:
  std::set<module_set_sptr> checkd_module_set_;

  variable_set_t *variable_set_;
  parameter_set_t *parameter_set_;
  variable_map_t *variable_map_;

  continuity_map_t variable_derivative_map_;

  boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver_;

  false_map_t false_conditions_;

  simulation_phase_sptr_t create_new_simulation_phase(const simulation_phase_sptr_t& old) const;

  void push_branch_states(simulation_phase_sptr_t &original, hydla::vcs::SymbolicVirtualConstraintSolver::check_consistency_result_t &result, CalculateClosureResult &dst);

  void add_continuity(const continuity_map_t& continuity_map);

};

}
}
#endif //_INCLUDED_CONSTRAINT_ANALYZER_H_
