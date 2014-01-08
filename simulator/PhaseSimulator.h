#ifndef _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
#define _INCLUDED_HYDLA_PHASE_SIMULATOR_H_

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#include "Timer.h"
#include "Logger.h"
#include "PhaseResult.h"
#include "RelationGraph.h"
#include "Simulator.h"

namespace hydla {

namespace simulator {


typedef std::vector<parameter_map_t>                       parameter_maps_t;

typedef std::vector<parameter_maps_t>    CheckConsistencyResult;


typedef enum{
  CONDITIONS_TRUE,
  CONDITIONS_FALSE,
  CONDITIONS_VARIABLE_CONDITIONS
} ConditionsResult;


typedef enum{
  CVM_INCONSISTENT,
  CVM_CONSISTENT,
  CVM_ERROR
} CalculateVariableMapResult;
 

class PhaseSimulator{

public:
  typedef std::vector<simulation_todo_sptr_t> todo_list_t;
  typedef std::vector<phase_result_sptr_t> result_list_t;

  typedef std::map<module_set_sptr, hydla::parse_tree::node_sptr> condition_map_t;
  typedef hydla::parse_tree::node_sptr node_sptr;

  PhaseSimulator(Simulator* simulator, const Opts& opts);
  PhaseSimulator(PhaseSimulator&);
  
  virtual ~PhaseSimulator();

  virtual void set_backend(backend_sptr_t back);

  void set_break_condition(node_sptr break_cond);
  node_sptr get_break_condition();


  /**
   * calculate phase results from given todo
   * @param todo_cont container of todo into which PhaseSimulator pushes todo if case analyses are needed
   *                  if it's null, PhaseSimulator uses internal container and handle all cases derived from given todo
   */
  result_list_t calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont = NULL);

  /**
   * HAConverter用：二つの変数の存在範囲の関係を求める
   * tmp_variable_phase in tmp_variable_past => true else false
   */ 
  bool check_include_bound(value_t tmp_variable_phase, value_t tmp_variable_past, parameter_map_t pm1, parameter_map_t pm2);

	/**
   * HASimulator用
   */ 
	void substitute_values_for_vm(phase_result_sptr_t pr, std::map<parameter_t, value_t> vm);
  void substitute_current_time_for_vm(phase_result_sptr_t pr, time_t current_time);
	
 	/**
   * make todos from given phase_result
   * this function doesn't change the 'phase' argument except the end time of phase
   */ 
  virtual todo_list_t make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo) = 0;

  virtual void initialize(variable_set_t &v,
    parameter_map_t &p,
    variable_map_t &m,
    continuity_map_t& c,
    parse_tree_sptr pt,
    const module_set_container_sptr &msc_no_init);
  
  int get_phase_sum()const{return phase_sum_;}
  
  void set_select_function(int (*f)(result_list_t&)){select_phase_ = f;}

  virtual void init_arc(const parse_tree_sptr& parse_tree) = 0;

  virtual void find_unsat_core(const module_set_sptr& ms,
      simulation_todo_sptr_t&,
    const variable_map_t& vm) = 0;


  /// pointer to the backend to be used
  backend_sptr_t backend_;
  bool breaking;
  
protected:
  
  
  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;
  
  /**
   * 与えられた制約モジュール集合の閉包計算を行い，無矛盾性を判定するとともに対応する変数表を返す．
   */

  virtual CalculateVariableMapResult calculate_variable_map(const module_set_sptr& ms,
                           simulation_todo_sptr_t& state, const variable_map_t &, variable_maps_t& result_vms) = 0;

  result_list_t simulate_ms(const module_set_sptr& ms, boost::shared_ptr<RelationGraph>& graph, 
                                  const variable_map_t& time_applied_map, simulation_todo_sptr_t& state);
                                  
                                  
  virtual CheckEntailmentResult check_entailment(
    CheckConsistencyResult &cc_result,
    const node_sptr& guard,
    const continuity_map_t& cont_map,
    const Phase& phase) = 0;
  
  virtual variable_map_t apply_time_to_vm(const variable_map_t &, const time_t &) = 0;
  
  /**
   * 与えられたsimulation_todo_sptr_tの情報を引き継いだ，
   * 新たなsimulation_todo_sptr_tの作成
   */
  simulation_todo_sptr_t create_new_simulation_phase(const simulation_todo_sptr_t& old) const;

  /**
   * PPモードとIPモードを切り替える
   */
  virtual void set_simulation_mode(const Phase& phase) = 0;


  Simulator* simulator_;

  void replace_prev2parameter(variable_map_t &vm, 
                              phase_result_sptr_t &phase);

  virtual CalculateVariableMapResult check_conditions(
    const module_set_sptr& ms,
    simulation_todo_sptr_t&,
    const variable_map_t &,
    bool b) = 0;

  virtual void mark_nodes_by_unsat_core(
    const module_set_sptr& ms,
    simulation_todo_sptr_t&,
    const variable_map_t&
      ) = 0;

  virtual module_set_list_t calculate_mms(
    simulation_todo_sptr_t& state,
    const variable_map_t& vm) = 0;

  const Opts *opts_;
  
  variable_set_t *variable_set_;
  parameter_map_t *parameter_map_;
  variable_map_t *variable_map_;
  negative_asks_t prev_guards_;

  int phase_sum_;

  /**
   * graph of relation between module_set for IP and PP
   */
  boost::shared_ptr<RelationGraph> pp_relation_graph_, ip_relation_graph_;
  
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
  node_sptr break_condition_;
  parse_tree_sptr parse_tree_;

  private:

  /**
   * merge rhs to lhs
   */
  void merge_variable_map(variable_map_t& lhs, const variable_map_t& rhs);

  void merge_variable_maps(variable_maps_t& lhs, const variable_maps_t& rhs);

  result_list_t make_results_from_todo(simulation_todo_sptr_t& todo);
  
  phase_result_sptr_t make_new_phase(simulation_todo_sptr_t& todo, const variable_map_t& vm);
};


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
