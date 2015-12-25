#pragma once

#include <iostream>
#include <string>
#include <cassert>

#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"
#include "ConsistencyChecker.h"
#include "RelationGraph.h"

namespace kv
{
template <class T> class interval;
}

namespace hydla {
namespace simulator {


class ValueModifier;
class MinTimeCalculator;

struct CompareMinTimeResult
{
  ConstraintStore less_cons, greater_cons, equal_cons;
};

typedef std::list<parameter_map_t>                       parameter_maps_t;
 typedef symbolic_expression::node_sptr                     node_sptr;


struct BreakPoint
{
  node_sptr condition;              // Simulation will break if this condition is entailed
  bool (*call_back)(BreakPoint, phase_result_sptr_t);    // return value of this function indicates whether the succeeding simulation is necessary or not
  void *tag;
};



class PhaseSimulator{
public:
  PhaseSimulator(Simulator* simulator, const Opts& opts);
  virtual ~PhaseSimulator();

  virtual void initialize(variable_set_t  &v,
                          parameter_map_t &p,
                          variable_map_t  &m,
                          module_set_container_sptr &msc,
                          phase_result_sptr_t root);


  void process_todo(phase_result_sptr_t&);


  void set_backend(backend_sptr_t);

  void apply_diff(const PhaseResult &phase);

  /// revert diff
  void revert_diff(const PhaseResult &phase);
  void revert_diff(const asks_t &positive_asks, const asks_t &negative_asks, const ConstraintStore &always_list, const module_diff_t &module_diff);

  void add_break_point(BreakPoint b);

  boost::shared_ptr<RelationGraph> relation_graph_;

private:

  std::list<phase_result_sptr_t> simulate_ms(const module_set_t& unadopted_ms, phase_result_sptr_t state, asks_t trigger_asks);

  ConstraintStore replace_prev_store(PhaseResult *parent, ConstraintStore orig);

  void replace_prev2parameter(
                              PhaseResult &phase,
                              variable_map_t &vm);

  void reset_parameter_constraint(ConstraintStore par_cons);

  ConstraintStore get_current_parameter_constraint();

  module_diff_t get_module_diff(module_set_t unadopted_ms, module_set_t parent_unadopted);

  variable_map_t get_related_vm(const node_sptr &node, const variable_map_t &vm);

  std::list<phase_result_sptr_t> make_results_from_todo(phase_result_sptr_t& todo);

  void push_branch_states(phase_result_sptr_t original,
                          CheckConsistencyResult &result);
  phase_result_sptr_t clone_branch_state(phase_result_sptr_t original);
  find_min_time_result_t find_min_time_test(phase_result_sptr_t &phase,const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed);
  find_min_time_result_t calculate_tmp_min_time(phase_result_sptr_t &phase,const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed);

  find_min_time_result_t find_min_time(const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed, phase_result_sptr_t &phase);

  pp_time_result_t compare_min_time(const pp_time_result_t &existing, const find_min_time_result_t &newcomer, const ask_t& ask);

  bool calculate_closure(phase_result_sptr_t& state, asks_t &trigger_asks,   ConstraintStore &diff_sum, asks_t &positive_asks, asks_t &negative_asks, ConstraintStore& always);

  bool check_equality(const value_t &lhs, const value_t &rhs);

  kv::interval<double> calculate_zero_crossing_of_derivative(const constraint_t& guard, const variable_map_t &related_vm, parameter_map_t &pm);
  
  std::list<kv::interval<double> > calculate_interval_newton_nd(const constraint_t& guard, const variable_map_t &related_vm, parameter_map_t &pm, bool additional_constraint);

  kv::interval<double> evaluate_interval(const phase_result_sptr_t phase, ValueRange range);

  ValueRange create_range_from_interval(kv::interval<double> itv);

 	/// make todos from given phase_result
  void make_next_todo(phase_result_sptr_t& phase);

  void approximate_phase(phase_result_sptr_t &phase, variable_map_t &vm_to_approximate);

  void check_break_points(phase_result_sptr_t &phase, variable_map_t &vm);

  value_t calculate_middle_value(const phase_result_sptr_t &phase, ValueRange range);

  std::list<constraint_t> calculate_approximated_time_constraint(const constraint_t& guard, const variable_map_t &related_vm, parameter_map_t &pm, parameter_map_t &pm_for_newton, std::list<Parameter> &parameters);

  void add_parameter_constraint(const phase_result_sptr_t phase, const Parameter &parameter, ValueRange current_range);

  Simulator* simulator_;

  const Opts *opts_;

  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  std::set<std::string> variable_names;

  module_set_container_sptr msc_no_init_;

  phase_result_sptr_t result_root;

  boost::shared_ptr<ConsistencyChecker> consistency_checker;
  int                                   phase_sum_, time_id;
  module_set_container_sptr             module_set_container;
  asks_t                                all_asks;
  boost::shared_ptr<ValueModifier>      value_modifier;
  value_t                               max_time;
  std::list<std::pair<BreakPoint, find_min_time_result_t> >                 break_point_list;
  bool                                  aborting;
  double                                upper_bound_of_itv_newton = 100;

  /// pointer to the backend to be used
  backend_sptr_t backend_;
};


} //namespace simulator
} //namespace hydla
