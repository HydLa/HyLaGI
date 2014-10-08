#pragma once

#include <iostream>
#include <string>
#include <cassert>

#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"
#include "ConsistencyChecker.h"
#include "RelationGraph.h"
#include "AskRelationGraph.h"

namespace hydla {
namespace simulator {

class ValueModifier;

struct FindMinTimeCandidate
{
  value_t         time;
  bool            on_time;
  parameter_map_t parameter_map;
};

struct DCCandidate 
{
  value_t time;
  std::map<ask_t, bool>        diff_positive_asks, /// newly entailed asks
                                diff_negative_asks; /// newly not entailed asks
  /// condition for parameter in this case
  parameter_map_t parameter_map;
  DCCandidate(const value_t                &t,
              const std::map<ask_t, bool> &dp,
              const std::map<ask_t, bool> &dn,
              const parameter_map_t &p)
                :time(t),
                 diff_positive_asks(dp),
                 diff_negative_asks(dn),
                 parameter_map(p)
    {}
  DCCandidate(){}
};

typedef std::list<DCCandidate>            pp_time_result_t;
typedef std::vector<FindMinTimeCandidate> find_min_time_result_t;

struct CompareMinTimeResult
{
  std::vector<parameter_map_t> less_maps, greater_maps, equal_maps;
};


typedef std::vector<parameter_map_t>                       parameter_maps_t;
typedef symbolic_expression::node_sptr                     node_sptr;

class PhaseSimulator{
public:
  PhaseSimulator(Simulator* simulator, const Opts& opts);
  virtual ~PhaseSimulator();

  virtual void initialize(variable_set_t  &v,
                          parameter_map_t &p,
                          variable_map_t  &m,
                          module_set_container_sptr &msc,
                          phase_result_sptr_t root);

  void process_todo(simulation_job_sptr_t& todo);


  void set_backend(backend_sptr_t);

  /// apply diff of given PhaseResult for relation_graph
  void apply_diff(const phase_result_sptr_t &phase);
  /// revert diff
  void revert_diff(const phase_result_sptr_t &phase);

  boost::shared_ptr<RelationGraph> relation_graph_;

private:

  std::list<phase_result_sptr_t> simulate_ms(const module_set_t& unadopted_ms, simulation_job_sptr_t& state, constraint_diff_t &local_diff, std::map<ask_t, bool> &discrete_nonprev_positives, std::map<ask_t, bool> &discrete_nonprev_negatives);

  void replace_prev2parameter(
                              PhaseResult &phase,
                              variable_map_t &vm,
                              parameter_map_t &parameter_map);

  variable_map_t get_related_vm(const node_sptr &node, const variable_map_t &vm);

  phase_result_sptr_t make_new_phase(const phase_result_sptr_t& original);

  std::list<phase_result_sptr_t> make_results_from_todo(simulation_job_sptr_t& todo);

  void push_branch_states(simulation_job_sptr_t &original,
                          CheckConsistencyResult &result);

  phase_result_sptr_t make_new_phase(simulation_job_sptr_t& todo);

  pp_time_result_t compare_min_time(const pp_time_result_t &existing, const find_min_time_result_t &newcomer, const ask_t& ask, bool positive);

  bool calculate_closure(simulation_job_sptr_t& state, constraint_diff_t &local_diff, std::map<ask_t, bool> &discrete_nonprev_positives, std::map<ask_t, bool> &discrete_nonprev_negatives);

  bool judge_continuity(const simulation_job_sptr_t &job, const ask_t &ask);

 	/// make todos from given phase_result
  void make_next_todo(phase_result_sptr_t& phase, simulation_job_sptr_t& current_todo);

  Simulator* simulator_;

  const Opts *opts_;

  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  std::set<std::string> variable_names;

  module_set_container_sptr msc_no_init_;

  phase_result_sptr_t result_root;

  boost::shared_ptr<AskRelationGraph> guard_relation_graph_;

  bool relation_graph_is_taken_over;  /// indicates whether the state of relation_graph_ is taken over from parent phase

  boost::shared_ptr<ConsistencyChecker> consistency_checker;
  int                                   phase_sum_, todo_id;
  module_set_container_sptr             module_set_container;
  ask_set_t                             all_asks;
  boost::shared_ptr<ValueModifier>      value_modifier;
  value_t                               max_time;

  /// pointer to the backend to be used
  backend_sptr_t backend_;

};


} //namespace simulator
} //namespace hydla
