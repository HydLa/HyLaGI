#ifndef _INCLUDED_HYDLA_ANALYSIS_RESULT_CHECKER_H_
#define _INCLUDED_HYDLA_ANALYSIS_RESULT_CHECKER_H_

#include "ParseTree.h"
#include "Simulator.h"
#include "ConstraintAnalyzer.h"

namespace hydla{
namespace simulator{

class AnalysisResultChecker : public ConstraintAnalyzer
{
public:
  AnalysisResultChecker(const simulator::Opts& opts);
  virtual ~AnalysisResultChecker();

  //virtual void set_solver();

  virtual module_set_list_t 
    calculate_mms(
      simulator::simulation_todo_sptr_t& state,
      const variable_map_t& vm,
      todo_container_t* todo_container);


  virtual bool 
    check_conditions(
      const hydla::symbolic_expression::node_sptr& cond,
      simulation_todo_sptr_t& state,
      const variable_map_t& vm,
      todo_container_t* todo_container);


  virtual bool 
    check_conditions(const hydla::hierarchy::module_set_sptr& ms,
                           simulator::simulation_todo_sptr_t& state,
		     const variable_map_t&,
		     bool b,
                     todo_container_t* todo_container);

  virtual void parse();

private:
  std::set<module_set_sptr> checkd_module_set_;

  virtual symbolic_expression::node_sptr string2node(std::string s);

};

}
}
#endif //_INCLUDED_CONSTRAINT_ANALYZER_H_
