#ifndef _INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_
#define _INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_

#include "SymbolicTypes.h"

#include "ParseTree.h"
#include "ModuleSet.h"
#include "Simulator.h"
#include "../virtual_constraint_solver/SymbolicVirtualConstraintSolver.h"

namespace hydla{
namespace simulator{
namespace symbolic {

class UnsatCoreFinder : public hydla::simulator::Simulator
{
public:
  UnsatCoreFinder(const simulator::Opts& opts);
  virtual ~UnsatCoreFinder();

  typedef enum{
    FALSE_CONDITIONS_TRUE,
    FALSE_CONDITIONS_FALSE,
    FALSE_CONDITIONS_VARIABLE_CONDITIONS
  } FalseConditionsResult;

  typedef std::map<std::string, hydla::parse_tree::node_sptr> false_map_t;
  typedef std::map<std::pair<hydla::parse_tree::node_sptr,std::string>,module_set_sptr > unsat_constraints_t;
  typedef std::map<std::pair<std::string,int>,module_set_sptr > unsat_continuities_t;

  typedef hydla::ch::ModuleSet::module_t module_t;
  typedef std::vector<module_t>                   module_list_t;

  virtual simulator::phase_result_const_sptr_t simulate();

  virtual void print_unsat_cores(unsat_constraints_t S,unsat_continuities_t S4C);

  virtual void find_unsat_core(const module_set_sptr& ms,unsat_constraints_t S,unsat_continuities_t S4C,simulation_todo_sptr_t&,const variable_map_t&);

  virtual void check_all_module_set();

  virtual void initialize();

  virtual void init_variable_map();

  virtual bool check_inconsistency();

  virtual bool check_unsat_core(unsat_constraints_t S,unsat_continuities_t S4C, const module_set_sptr& ms, simulation_todo_sptr_t&, const variable_map_t&);

  virtual void add_constraints(unsat_constraints_t S,unsat_continuities_t S4C);

protected:

  boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver_;

  false_map_t false_conditions_;

};

}
}
}
#endif //_INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_

