#ifndef _INCLUDED_HYDLA_ANALYSIS_RESULT_CHECKER_H_
#define _INCLUDED_HYDLA_ANALYSIS_RESULT_CHECKER_H_

#include "ParseTree.h"
#include "SymbolicPhaseSimulator.h"
#include "Simulator.h"
#include "ConstraintAnalyzer.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"

namespace hydla{
namespace simulator{
namespace symbolic {

class AnalysisResultChecker : public ConstraintAnalyzer
{
public:
  AnalysisResultChecker(const simulator::Opts& opts);
  virtual ~AnalysisResultChecker();

  virtual void set_solver(boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver);

  virtual simulator::CalculateVariableMapResult 
    check_false_conditions(const hydla::ch::module_set_sptr& ms,
                           simulator::simulation_todo_sptr_t& state,
                           const variable_map_t&);

  virtual void parse();

private:
  std::set<module_set_sptr> checkd_module_set_;

  virtual parse_tree::node_sptr string2node(std::string s);

};

}
}
}
#endif //_INCLUDED_CONSTRAINT_ANALYZER_H_

