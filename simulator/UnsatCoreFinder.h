#ifndef _INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_
#define _INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_

#include "SymbolicTypes.h"

#include "ParseTree.h"
#include "ModuleSet.h"
#include "Simulator.h"

namespace hydla{
namespace simulator{

class UnsatCoreFinder
{
public:
  UnsatCoreFinder();
  UnsatCoreFinder(backend_sptr_t back);
  virtual ~UnsatCoreFinder();

  typedef std::map<std::pair<hydla::parse_tree::node_sptr,std::string>,module_set_sptr > unsat_constraints_t;
  typedef std::map<std::pair<std::string,int>,module_set_sptr > unsat_continuities_t;

  typedef hydla::ch::ModuleSet::module_t module_t;
  typedef std::vector<module_t>                   module_list_t;

  virtual void print_unsat_cores(unsat_constraints_t S,unsat_continuities_t S4C);

  virtual void find_unsat_core(const module_set_sptr& ms,unsat_constraints_t& S,unsat_continuities_t& S4C,simulation_todo_sptr_t&,const variable_map_t&);

  virtual bool check_inconsistency();
  void set_backend(backend_sptr_t back);
  virtual bool check_unsat_core(unsat_constraints_t S,unsat_continuities_t S4C, const module_set_sptr& ms, simulation_todo_sptr_t&, const variable_map_t&);

  virtual void add_constraints(unsat_constraints_t S,unsat_continuities_t S4C, Phase phase);

protected:
  void reset(Phase phase, const variable_map_t &vm, const parameter_map_t &pm);
  backend_sptr_t backend_;
};

}
}
#endif //_INCLUDED_HYDLA_UNSAT_CORE_FINDER_H_

