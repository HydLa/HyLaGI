#ifndef _INCLUDED_HYDLA_ANALYSIS_RESULT_CHECKER_H_
#define _INCLUDED_HYDLA_ANALYSIS_RESULT_CHECKER_H_

#include "ParseTree.h"
#include "SymbolicSimulator.h"
#include "Simulator.h"
#include "ConstraintAnalyzer.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"

namespace hydla{
namespace symbolic_simulator{

class AnalysisResultChecker : public ConstraintAnalyzer
{
public:
  AnalysisResultChecker(const Opts& opts);
  virtual ~AnalysisResultChecker();

  virtual void set_solver(boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver);

  virtual SymbolicSimulator::CalculateVariableMapResult check_false_conditions(const module_set_sptr& ms, simulation_phase_sptr_t& state, const variable_map_t&, variable_map_t& result_vm, todo_and_results_t& result_todo);

  virtual void parse();

//  virtual void parse();

private:
  std::set<module_set_sptr> checkd_module_set_;

  virtual node_sptr string2node(std::string s);

};

}
}
#endif //_INCLUDED_CONSTRAINT_ANALYZER_H_

