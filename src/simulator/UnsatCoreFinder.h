#pragma once

#include "ParseTree.h"
#include "ModuleSet.h"
#include "PhaseResult.h"
#include "Simulator.h"

namespace hydla{
namespace simulator{

class UnsatCoreFinder
{
public:
  UnsatCoreFinder();
  UnsatCoreFinder(backend_sptr_t back);
  ~UnsatCoreFinder();

  typedef std::map<std::pair<hydla::symbolic_expression::node_sptr,std::string>,module_set_t > unsat_constraints_t;
  typedef std::map<std::pair<std::string,int>,module_set_t > unsat_continuities_t;

  typedef hierarchy::ModuleSet::module_t module_t;
  typedef std::vector<module_t>                   module_list_t;

  void print_unsat_cores(unsat_constraints_t S,unsat_continuities_t S4C);

//  void find_unsat_core(const module_set_sptr& ms,unsat_constraints_t& S,unsat_continuities_t& S4C, simulation_job_sptr_t&,const variable_map_t&);

/*  void find_unsat_core(const module_set_sptr& ms,unsat_constraints_t& S,unsat_continuities_t& S4C,
  const asks_t &positive_asks,
  const asks_t &negative_asks,
  const variable_map_t& vm,
  const parameter_map_t &pm,
  PhaseType phase_type);
*/
  bool check_inconsistency(PhaseType phase_type);
  void set_backend(backend_sptr_t back);
//  bool check_unsat_core(unsat_constraints_t S,unsat_continuities_t S4C,const module_set_sptr& ms, PhaseType phase_type, const variable_map_t& vm, const parameter_map_t& pm);

  void add_constraints(unsat_constraints_t S,unsat_continuities_t S4C, PhaseType phase);

protected:
  void reset(PhaseType phase, const variable_map_t &vm, const parameter_map_t &pm);
  backend_sptr_t backend_;
};

}
}
