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

namespace hydla {
namespace symbolic_simulator {


class SymbolicSimulator : public simulator_t
{
public:

  typedef hydla::vcs::SymbolicVirtualConstraintSolver solver_t;
  typedef simulator::Opts Opts;


  SymbolicSimulator(const Opts& opts);
  virtual ~SymbolicSimulator();

  virtual Phases simulate_ms_point(const module_set_sptr& ms, 
                           phase_result_sptr& state, variable_map_t &vm, bool& consistent);
  
  virtual Phases simulate_ms_interval(const module_set_sptr& ms, 
                              phase_result_sptr& state, bool& consistent);

  virtual void initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c);
  virtual void set_parameter_set(parameter_t param);
  virtual parameter_set_t get_parameter_set();

private:

  variable_map_t range_map_to_value_map(const phase_result_sptr&, const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t &, parameter_map_t &);

  variable_t* get_variable(const std::string &name, const int &derivative_count){
    return &(*std::find(variable_set_->begin(), variable_set_->end(), (variable_t(name, derivative_count))));
  }
  
  variable_map_t shift_variable_map_time(const variable_map_t& vm,const time_t &time);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  
  CalculateClosureResult calculate_closure(phase_result_sptr& state,
                        const module_set_sptr& ms, expanded_always_t &expanded_always,
                         positive_asks_t &positive_asks, negative_asks_t &negative_asks);

  void push_branch_states(phase_result_sptr &original, hydla::vcs::SymbolicVirtualConstraintSolver::check_consistency_result_t &result, CalculateClosureResult &dst);

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
