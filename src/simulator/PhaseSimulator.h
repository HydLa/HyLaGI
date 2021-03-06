#pragma once

#include <cassert>
#include <iostream>
#include <stack>
#include <string>

#include "ConsistencyChecker.h"
#include "ParseTreeSemanticAnalyzer.h"
#include "PhaseResult.h"
#include "RelationGraph.h"
#include "Simulator.h"
#include <boost/shared_ptr.hpp>
#include <memory>

namespace kv {
template <class T> class interval;
}

namespace hydla {
namespace simulator {

class ValueModifier;
class MinTimeCalculator;

struct CompareMinTimeResult {
  ConstraintStore less_cons, greater_cons, equal_cons;
};

typedef std::list<parameter_map_t> parameter_maps_t;
typedef symbolic_expression::node_sptr node_sptr;

struct BreakPoint {
  node_sptr condition; // Simulation will break if this condition is entailed
  bool (*call_back)(
      BreakPoint,
      phase_result_sptr_t); // return value of this function indicates whether
                            // the succeeding simulation is necessary or not
  void *tag;
};

struct TimeListElement {
  value_t time;
  ConstraintStore parameter_constraint;
  constraint_t guard;
  TimeListElement(value_t t, constraint_t g) : time(t), guard(g) {}
  TimeListElement() {}
};

struct IntervalNewtonResult {
  std::shared_ptr<kv::interval<double>> current_stack_top;
  std::shared_ptr<kv::interval<double>> min_interval;
  std::stack<kv::interval<double>, std::vector<kv::interval<double>>>
      next_stack;
  int time_id;
  value_t time_list_element_time;
  bool isAffine;
};

struct HistoryData {
  std::vector<IntervalNewtonResult> results;
};

class PhaseSimulator {
public:
  PhaseSimulator(Simulator *simulator, const Opts &opts);
  virtual ~PhaseSimulator();

  virtual void initialize(variable_set_t &v, parameter_map_t &p,
                          variable_map_t &m, module_set_container_sptr &msc,
                          phase_result_sptr_t root,
                          parser::ParseTreeSemanticAnalyzer *analyzer);

  phase_list_t process_todo(phase_result_sptr_t &);

  void set_backend(backend_sptr_t);

  void apply_diff(const PhaseResult &phase);

  void revert_diff(const PhaseResult &phase);
  void revert_diff(const asks_t &positive_asks, const asks_t &negative_asks,
                   const ConstraintStore &always_list,
                   const module_diff_t &module_diff);

  void add_break_point(BreakPoint b);

  std::shared_ptr<RelationGraph> relation_graph_;

  void print_completely_unconstrained_condition();

  std::pair<std::map<Variable, Variable>, constraint_t>
  clone_exists_constraint(ask_t positive, int phase_num);

private:
  struct StateOfIntervalNewton;

  std::list<phase_result_sptr_t> simulate_ms(const module_set_t &unadopted_ms,
                                             phase_result_sptr_t state,
                                             asks_t trigger_asks);

  ConstraintStore replace_prev_store(PhaseResult *parent, ConstraintStore orig);

  void replace_prev2parameter(PhaseResult &phase, variable_map_t &vm);

  void reset_parameter_constraint(ConstraintStore par_cons);

  ConstraintStore get_current_parameter_constraint();

  module_diff_t get_module_diff(module_set_t unadopted_ms,
                                module_set_t parent_unadopted);

  variable_map_t get_related_vm(const node_sptr &node,
                                const variable_map_t &vm);

  std::list<phase_result_sptr_t>
  make_results_from_todo(phase_result_sptr_t &todo);

  void push_branch_states(phase_result_sptr_t original,
                          CheckConsistencyResult &result);
  /**
   * 分岐用にフェーズリザルトをコピーする
   * @brief 必要なフィールドを新しいインスタンスに代入
   * @param (original) コピーしたいフェーズリザルト
   * @return コピーしたフェーズリザルト
   */
  phase_result_sptr_t clone_branch_state(phase_result_sptr_t original);
  find_min_time_result_t find_min_time_test(
      phase_result_sptr_t &phase, const constraint_t &guard,
      MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map,
      variable_map_t &original_vm, Value &time_limit, bool entailed);
  find_min_time_result_t calculate_tmp_min_time(
      phase_result_sptr_t &phase, const constraint_t &guard,
      MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map,
      variable_map_t &original_vm, Value &time_limit, bool entailed);

  find_min_time_result_t find_min_time(
      const constraint_t &guard, MinTimeCalculator &min_time_calculator,
      guard_time_map_t &guard_time_map, variable_map_t &original_vm,
      Value &time_limit, bool entailed, phase_result_sptr_t &phase,
      std::map<std::string, HistoryData> &atomic_guard_min_time_interval_map);

  find_min_time_result_t find_min_time_step_by_step(
      const constraint_t &guard, variable_map_t &original_vm, Value &time_limit,
      phase_result_sptr_t &phase, bool entailed,
      std::map<std::string, HistoryData> &atomic_guard_min_time_interval_map);

  bool checkAndUpdateGuards(std::map<constraint_t, bool> &guard_map,
                            constraint_t guard,
                            std::list<constraint_t> guard_list, bool &on_time,
                            bool entailed);

  pp_time_result_t compare_min_time(const pp_time_result_t &existing,
                                    const find_min_time_result_t &newcomer,
                                    const ask_t &ask);

  bool calculate_closure(phase_result_sptr_t &state, asks_t &trigger_asks,
                         ConstraintStore &diff_sum, asks_t &positive_asks,
                         asks_t &negative_asks, ConstraintStore &always);

  bool check_equality(const value_t &lhs, const value_t &rhs);

  kv::interval<double>
  calculate_zero_crossing_of_derivative(const constraint_t &guard,
                                        parameter_map_t &pm);

  std::list<kv::interval<double>>
  calculate_interval_newton_nd(const constraint_t &guard, parameter_map_t &pm);

  kv::interval<double> evaluate_interval(const phase_result_sptr_t phase,
                                         ValueRange range, bool use_affine);

  StateOfIntervalNewton initialize_newton_state(const constraint_t &time_guard,
                                                parameter_map_t &pm);

  ValueRange create_range_from_interval(kv::interval<double> itv);

  /// make todos from given phase_result
  void make_next_todo(phase_result_sptr_t &phase);

  void remove_redundant_parameters(phase_result_sptr_t phase);

  void approximate_phase(phase_result_sptr_t &phase,
                         variable_map_t &vm_to_approximate);

  void check_break_points(phase_result_sptr_t &phase, variable_map_t &vm);

  value_t calculate_middle_value(const phase_result_sptr_t &phase,
                                 ValueRange range);

  std::list<constraint_t> calculate_approximated_time_constraint(
      const constraint_t &guard, const variable_map_t &related_vm,
      parameter_map_t &pm, parameter_map_t &pm_for_newton,
      std::list<Parameter> &parameters);

  void add_parameter_constraint(const phase_result_sptr_t phase,
                                const Parameter &parameter,
                                ValueRange current_range);

  void print_possible_causes(const std::map<variable_set_t, module_set_t> &map);
  std::map<variable_set_t, module_set_t>
  filter_required(std::map<variable_set_t, module_set_t> causes);

  void update_condition(const variable_set_t &vs, const module_set_t &ms);

  Simulator *simulator_;

  const Opts *opts_;

  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  variable_set_t vars_to_approximate;
  std::set<std::string> variable_names;

  module_set_container_sptr msc_no_init_;

  phase_result_sptr_t result_root;

  std::shared_ptr<ConsistencyChecker> consistency_checker;
  int phase_sum_, time_id;
  module_set_container_sptr module_set_container;
  asks_t all_asks;
  std::shared_ptr<ValueModifier> value_modifier;
  value_t max_time;
  std::list<std::pair<BreakPoint, find_min_time_result_t>> break_point_list;
  bool aborting;
  int upper_bound_of_itv_newton = 100;

  /// pointer to the backend to be used
  backend_sptr_t backend_;

  // record minimum module_set_t which each variable is completely unconstrained
  std::map<variable_t, module_set_t> completely_unconstrained_condition;

  parser::ParseTreeSemanticAnalyzer *analyzer_;
};

} // namespace simulator
} // namespace hydla
