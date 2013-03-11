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

class PhaseSimulator{

public:  
  struct TodoAndResult{
    simulation_phase_sptr_t todo;
    phase_result_sptr_t result;
    TodoAndResult(const simulation_phase_sptr_t& t, const phase_result_sptr_t &r):todo(t), result(r){
    }
    TodoAndResult(){}
  };
  
  typedef std::vector<TodoAndResult> todo_and_results_t;

  typedef std::map<boost::shared_ptr<hydla::parse_tree::Ask>, bool> entailed_prev_map_t;

  typedef enum{
    CVM_INCONSISTENT,
    CVM_CONSISTENT,
    CVM_BRANCH,
    CVM_ERROR
  } CalculateVariableMapResult;

  PhaseSimulator(const Opts& opts);
  
  virtual ~PhaseSimulator();

  virtual variable_map_t apply_time_to_vm(const variable_map_t &, const time_t &) = 0;
  
  /**
   * merge rhs to lhs
   */
  virtual void merge_variable_map(variable_map_t& lhs, variable_map_t& rhs);

  /**
   * 新たなsimulation_phase_sptr_tの作成
   */
  simulation_phase_sptr_t create_new_simulation_phase() const;

  /**
   * 与えられたPhaseResultの情報を引き継いだ，
   * 新たなPhaseResultの作成
   */
  simulation_phase_sptr_t create_new_simulation_phase(const simulation_phase_sptr_t& old) const;

  /**
   * PPモードとIPモードを切り替える
   */
  virtual void set_simulation_mode(const Phase& phase) = 0;

  todo_and_results_t simulate_phase(simulation_phase_sptr_t& state, bool &consistent);
  
  todo_and_results_t simulate_ms(const module_set_sptr& ms, boost::shared_ptr<RelationGraph>& graph, 
                                  const variable_map_t& time_applied_map, simulation_phase_sptr_t& state, bool &consistent);

  /**
   * 与えられた制約モジュール集合の閉包計算を行い，無矛盾性を判定するとともに対応する変数表を返す．
   */

  virtual CalculateVariableMapResult calculate_variable_map(const module_set_sptr& ms,
                           simulation_phase_sptr_t& state, const variable_map_t &, variable_map_t& result_vm, todo_and_results_t& result_todo) = 0;

  /**
   * 与えられたフェーズの次のTODOを返す．
   */ 
  virtual todo_and_results_t make_next_todo(const module_set_sptr& ms, simulation_phase_sptr_t& state, variable_map_t &) = 0;

  virtual void initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, const module_set_sptr& ms, continuity_map_t& c);

  virtual parameter_set_t get_parameter_set()
  {
    return *parameter_set_;
  }
  
  virtual bool is_safe() const{return is_safe_;}

  virtual CalculateVariableMapResult check_false_conditions(const module_set_sptr& ms, simulation_phase_sptr_t& state, const variable_map_t &, variable_map_t& result_vm, todo_and_results_t& result_todo) = 0;
  
protected:

  variable_t* get_variable(const std::string &name, const int &derivative_count){
    return &(*std::find(variable_set_->begin(), variable_set_->end(), (variable_t(name, derivative_count))));
  }

  const Opts *opts_;
  bool is_safe_;
  
  variable_set_t *variable_set_;
  parameter_set_t *parameter_set_;
  variable_map_t *variable_map_;
  entailed_prev_map_t judged_prev_map_;
  negative_asks_t prev_guards_;
  
  /**
   * graph of relation between module_set for IP and PP
   */
  boost::shared_ptr<RelationGraph> pp_relation_graph_, ip_relation_graph_;
  
};


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
