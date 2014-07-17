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

typedef std::vector<parameter_map_t>                       parameter_maps_t;

typedef enum{
  CONDITIONS_TRUE,
  CONDITIONS_FALSE,
  CONDITIONS_VARIABLE_CONDITIONS
} ConditionsResult;

class PhaseSimulator{

public:
  typedef std::vector<simulation_todo_sptr_t> todo_list_t;
  typedef std::vector<phase_result_sptr_t> result_list_t;

  typedef symbolic_expression::node_sptr node_sptr;

  PhaseSimulator(Simulator* simulator, const Opts& opts, bool _validate = true);
  PhaseSimulator(PhaseSimulator&);

  virtual ~PhaseSimulator();

  void set_break_condition(symbolic_expression::node_sptr break_cond);
  symbolic_expression::node_sptr get_break_condition();

  virtual void initialize(variable_set_t &v, parameter_map_t &p, variable_map_t &m,  module_set_container_sptr& msc);

  /**
   * calculate phase results from given todo
   * @param todo_cont container of todo into which PhaseSimulator pushes todo if case analyses are needed
   *                  if it's null, PhaseSimulator uses internal container and handle all cases derived from given todo
   */
  result_list_t calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont = NULL);


  int get_phase_sum()const{return phase_sum_;}

  void set_select_function(int (*f)(result_list_t&)){select_phase_ = f;}


  typedef std::set< std::string > change_variables_t;

 	/**
   * make todos from given phase_result
   * this function doesn't change the 'phase' argument except the end time of phase
   */
  virtual todo_list_t make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo);

  void set_backend(backend_sptr_t);

  /// pointer to the backend to be used
  backend_sptr_t backend_;
  bool breaking;
  phase_result_sptr_t result_root;

protected:

  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;

  result_list_t simulate_ms(const module_set_t& unadopted_ms, simulation_todo_sptr_t& state);

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

  int phase_sum_;

  boost::shared_ptr<RelationGraph> relation_graph_;
  boost::shared_ptr<AskRelationGraph> guard_relation_graph_;

  /**
   * 解候補モジュール集合のコンテナ
   * （非always制約を除いたバージョン）
   */
  module_set_container_sptr msc_no_init_;

  todo_container_t* todo_container_;

  phase_result_sptr_t make_new_phase(const phase_result_sptr_t& original);


  void push_branch_states(simulation_todo_sptr_t &original,
    CheckConsistencyResult &result);


  /// ケースの選択時に使用する関数ポインタ
  int (*select_phase_)(result_list_t&);
  symbolic_expression::node_sptr break_condition_;

private:

  result_list_t make_results_from_todo(simulation_todo_sptr_t& todo);

  phase_result_sptr_t make_new_phase(simulation_todo_sptr_t& todo);

  void set_symmetric_difference(
    const ConstraintStore& parent_constraints,
    const ConstraintStore& current_constraints,
    ConstraintStore& result );

  bool calculate_closure(simulation_todo_sptr_t& state);


  CheckConsistencyResult check_consistency(const PhaseType &phase);

  PhaseType current_phase_;
  
  boost::shared_ptr<ConsistencyChecker> consistency_checker;

  module_set_container_sptr module_set_container;

  std::map<int, boost::shared_ptr<symbolic_expression::Ask> > ask_map;

  ConstraintDifferenceCalculator difference_calculator_;

  boost::shared_ptr<ValueModifier> value_modifier;

  bool validate;
};


} //namespace simulator
} //namespace hydla
