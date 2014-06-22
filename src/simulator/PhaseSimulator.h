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

namespace hydla {
namespace simulator {

class AnalysisResultChecker;
class UnsatCoreFinder;

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

  // typedef std::map<module_set_sptr, symbolic_expression::node_sptr> condition_map_t;
  typedef symbolic_expression::node_sptr node_sptr;

  PhaseSimulator(Simulator* simulator, const Opts& opts);
  PhaseSimulator(PhaseSimulator&);

  virtual ~PhaseSimulator();

  void set_break_condition(symbolic_expression::node_sptr break_cond);
  symbolic_expression::node_sptr get_break_condition();

  virtual void initialize(variable_set_t &v, parameter_map_t &p, variable_map_t &m,  module_set_container_sptr& msc);

  virtual void init_arc(const parse_tree_sptr& parse_tree);


  variable_map_t apply_time_to_vm(const variable_map_t &vm, const value_t &tm);
  variable_map_t shift_time_of_vm(const variable_map_t &vm, const value_t &tm);

  /**
   * calculate phase results from given todo
   * @param todo_cont container of todo into which PhaseSimulator pushes todo if case analyses are needed
   *                  if it's null, PhaseSimulator uses internal container and handle all cases derived from given todo
   */
  result_list_t calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont = NULL);


	/**
   * HASimulator用
   */
	void substitute_parameter_condition(phase_result_sptr_t pr, parameter_map_t pm);

  int get_phase_sum()const{return phase_sum_;}

  void set_select_function(int (*f)(result_list_t&)){select_phase_ = f;}


  typedef std::set< std::string > change_variables_t;

/*
  virtual void find_unsat_core(const module_set_t& ms,
      simulation_todo_sptr_t&,
      const variable_map_t& vm);
*/


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

  result_list_t simulate_ms(const module_set_t& ms, simulation_todo_sptr_t& state);

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
                              phase_result_sptr_t &phase,
                              variable_map_t &vm,
                              parameter_map_t &parameter_map);

  const Opts *opts_;

  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  negative_asks_t prev_asks_;

  int phase_sum_;

  boost::shared_ptr<RelationGraph> relation_graph_;

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

  /**
   * 与えられた制約モジュール集合の閉包計算を行い，無矛盾性を判定するとともに対応する変数表を返す．
   */
  virtual ConstraintStore calculate_constraint_store(const module_set_t& ms,
                           simulation_todo_sptr_t& state);

  void set_changing_variables( const phase_result_sptr_t& parent_phase,
                             const positive_asks_t& positive_asks,
                             const negative_asks_t& negative_asks,
                             change_variables_t& changing_variables );

  void set_changed_variables(phase_result_sptr_t& phase);

  change_variables_t get_difference_variables_from_2tells(const ConstraintStore& larg, const ConstraintStore& rarg);

  bool apply_entailment_change( const ask_set_t::iterator it,
                                const ask_set_t& previous_asks,
                                const bool in_IP,
                                change_variables_t& changing_variables,
                                ask_set_t& notcv_unknown_asks,
                                ask_set_t& unknown_asks );

  void apply_previous_solution(const change_variables_t& changing_variables,
                             const bool in_IP,
                             const phase_result_sptr_t parent,
                             const value_t& current_time );

  bool calculate_closure(simulation_todo_sptr_t& state,
    const module_set_t& ms);


  CheckConsistencyResult check_consistency(const PhaseType &phase);

  bool has_variables(symbolic_expression::node_sptr node, const change_variables_t &variables, bool include_prev);


/*
  virtual void mark_nodes_by_unsat_core(const modulse_set_sptr& ms,
      simulation_todo_sptr_t&,
    const variable_map_t& vm);
*/

  //virtual ConstraintStoreResult check_conditions(const module_set_t& ms, simulation_todo_sptr_t&, const variable_map_t &, bool b);
  void replace_prev2parameter(phase_result_sptr_t& state,
                              ConstraintStore& store,
                              parameter_map_t &parameter_map);

  boost::shared_ptr<AnalysisResultChecker > analysis_result_checker_;
  boost::shared_ptr<UnsatCoreFinder > unsat_core_finder_;

  PhaseType current_phase_;
  
  boost::shared_ptr<ConsistencyChecker> consistency_checker;

  module_set_container_sptr module_set_container;

  std::map<int, boost::shared_ptr<symbolic_expression::Ask> > ask_map;
};


} //namespace simulator
} //namespace hydla
