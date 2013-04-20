#ifndef _INCLUDED_HYDLA_CONSTRAINT_ANALYZER_H_
#define _INCLUDED_HYDLA_CONSTRAINT_ANALYZER_H_

#include "SymbolicTypes.h"

#include "ParseTree.h"
#include "Simulator.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"

namespace hydla{
namespace symbolic_simulator{

  class ConstraintAnalyzer : public hydla::simulator::Simulator
{
public:
  typedef enum{
    FALSE_CONDITIONS_TRUE,
    FALSE_CONDITIONS_FALSE,
    FALSE_CONDITIONS_VARIABLE_CONDITIONS
  } FalseConditionsResult;

  typedef std::map<std::string, hydla::parse_tree::node_sptr> false_map_t;
  //  typedef std::map<module_set_sptr, node_sptr> false_map_t;

  ConstraintAnalyzer(simulator::Opts& opts);
  virtual ~ConstraintAnalyzer();

  virtual simulator::phase_result_const_sptr_t simulate();

  virtual void output_false_conditions();

  virtual FalseConditionsResult find_false_conditions(const module_set_sptr& ms);

  virtual void check_all_module_set();

  virtual void initialize(const parse_tree_sptr& parse_tree);  

protected:

  boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver_;

  false_map_t false_conditions_;

};

}
}
#endif //_INCLUDED_CONSTRAINT_ANALYZER_H_
