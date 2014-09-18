#pragma once

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"
#include "ConsistencyChecker.h"
#include "RelationGraph.h"
#include "AskRelationGraph.h"
#include "ConstraintDifferenceCalculator.h"

namespace hydla {
namespace simulator {

class AnalysisResultChecker;
class UnsatCoreFinder;
class ValueModifier;

struct FindMinTimeCandidate
{
  value_t time;
  bool on_time;
  parameter_map_t parameter_map;
};

struct DCCandidate 
{
  value_t time;         /// 離散変化時刻
  std::list<DiscreteAsk>        diff_positive_asks, /// newly entailed asks
                                diff_negative_asks; /// newly not entailed asks
  /// condition for parameter in this case
  parameter_map_t parameter_map;
  DCCandidate(const value_t &t, const std::list<DiscreteAsk> &dp, const std::list<DiscreteAsk> &dn, const parameter_map_t &p):time(t), diff_positive_asks(dp), diff_negative_asks(dn), parameter_map(p){}
  DCCandidate(){}
};

typedef std::list<DCCandidate> pp_time_result_t;


typedef std::vector<FindMinTimeCandidate> find_min_time_result_t;

struct CompareMinTimeResult
{
  std::vector<parameter_map_t> less_maps, greater_maps, equal_maps;
};

typedef CompareMinTimeResult compare_min_time_result_t;


typedef std::vector<parameter_map_t>                       parameter_maps_t;

class PhaseSimulator{

public:
  typedef std::vector<simulation_todo_sptr_t> todo_list_t;
  typedef std::vector<phase_result_sptr_t> result_list_t;

  typedef symbolic_expression::node_sptr node_sptr;

  PhaseSimulator(Simulator* simulator, const Opts& opts);
  PhaseSimulator(PhaseSimulator&);

  virtual ~PhaseSimulator();

  virtual void initialize(variable_set_t &v, parameter_map_t &p, variable_map_t &m,  module_set_container_sptr& msc, phase_result_sptr_t root);

  /**
   * calculate phase results from given todo
   */
  result_list_t calculate_phase_result(simulation_todo_sptr_t& todo);

  void process_todo(simulation_todo_sptr_t& todo);

  typedef std::set< std::string > change_variables_t;

 	/**
   * make todos from given phase_result
   */
  void make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo);

  void set_backend(backend_sptr_t);

  /// pointer to the backend to be used
  backend_sptr_t backend_;

protected:

  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;

  void simulate_ms(const module_set_t& unadopted_ms, simulation_todo_sptr_t& state, constraint_diff_t &local_diff);

  /**
   * 与えられたsimulation_todo_sptr_tの情報を引き継いだ，
   * 新たなsimulation_todo_sptr_tの作成
   */
  simulation_todo_sptr_t create_new_simulation_phase(const simulation_todo_sptr_t& old) const;
  /**
   * PPモードとIPモードを切り替える
   */
  void set_simulation_mode(const PhaseType& phase);
  Simulator* simulator_;

  void replace_prev2parameter(
                              PhaseResult &phase,
                              variable_map_t &vm,
                              parameter_map_t &parameter_map);


  const Opts *opts_;

  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  ask_set_t prev_asks_;
  std::set<std::string> variable_names;

  boost::shared_ptr<RelationGraph> relation_graph_;
  boost::shared_ptr<AskRelationGraph> guard_relation_graph_;

  /**
   * 解候補モジュール集合のコンテナ
   * （非always制約を除いたバージョン）
   */
  module_set_container_sptr msc_no_init_;

  phase_result_sptr_t result_root;

  phase_result_sptr_t make_new_phase(const phase_result_sptr_t& original);


  void push_branch_states(simulation_todo_sptr_t &original,
    CheckConsistencyResult &result);

  variable_map_t get_related_vm(const node_sptr &node, const variable_map_t &vm);

private:

  void make_results_from_todo(simulation_todo_sptr_t& todo);

  phase_result_sptr_t make_new_phase(simulation_todo_sptr_t& todo);

  void compare_min_time(pp_time_result_t &existing, const find_min_time_result_t &newcomer, const ask_t& ask, bool positive);

  bool relation_graph_is_taken_over;  /// indicates whether the state of relation_graph_ is taken over from parent phase

  bool calculate_closure(simulation_todo_sptr_t& state, constraint_diff_t &local_diff);

  CheckConsistencyResult check_consistency(const PhaseType &phase);

  PhaseType current_phase_;
  
  boost::shared_ptr<ConsistencyChecker> consistency_checker;

  int                                   phase_sum_;

  module_set_container_sptr             module_set_container;
  ask_set_t                             all_asks;
  std::map<int, ask_t >                 ask_map;
  ConstraintDifferenceCalculator        difference_calculator_;
  boost::shared_ptr<ValueModifier>      value_modifier;
  value_t                               max_time;
};


} //namespace simulator
} //namespace hydla
