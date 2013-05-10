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

namespace vcs{
  struct CheckConsistencyResult;
  class SymbolicVirtualConstraintSolver;
}

namespace simulator {


typedef enum{
  FALSE_CONDITIONS_TRUE,
  FALSE_CONDITIONS_FALSE,
  FALSE_CONDITIONS_VARIABLE_CONDITIONS
} FalseConditionsResult;


typedef enum{
  CVM_INCONSISTENT,
  CVM_CONSISTENT,
  CVM_ERROR
} CalculateVariableMapResult;
 

class PhaseSimulator{

public:
  typedef vcs::SymbolicVirtualConstraintSolver solver_t;
  typedef std::vector<simulation_todo_sptr_t> todo_list_t;
  typedef std::vector<phase_result_sptr_t> result_list_t;

  typedef std::map<module_set_sptr, hydla::parse_tree::node_sptr> false_map_t;
  typedef hydla::parse_tree::node_sptr node_sptr;

  PhaseSimulator(const Opts& opts);
  
  virtual ~PhaseSimulator();

  /**
   * calculate phase results from given todo
   * @param todo_cont container of todo into which PhaseSimulator pushes todo if case analyses are needed
   *                  if it's null, PhaseSimulator uses internal container and handle all cases derived from given todo
   */
  result_list_t calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont = NULL);


  /**
   * make todos from given phase_result
   * this function doesn't change the 'phase' argument except the end time of phase
   */ 
  virtual todo_list_t make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo) = 0;

  virtual void initialize(variable_set_t &v,
    parameter_set_t &p, 
    variable_range_map_t &m,
    continuity_map_t& c,
    const module_set_container_sptr &msc_no_init);

  virtual parameter_set_t get_parameter_set(){return *parameter_set_;}
  
  int get_phase_sum()const{return phase_sum_;}
  
  void set_select_function(int (*f)(result_list_t&)){select_phase_ = f;}
  
protected:
  
  
  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;
  
  /**
   * �^����ꂽ���񃂃W���[���W���̕�v�Z���s���C���������𔻒肷��ƂƂ��ɑΉ�����ϐ��\��Ԃ��D
   */

  virtual CalculateVariableMapResult calculate_variable_map(const module_set_sptr& ms,
                           simulation_todo_sptr_t& state, const variable_map_t &, variable_range_map_t& result_vm) = 0;

  result_list_t simulate_ms(const module_set_sptr& ms, boost::shared_ptr<RelationGraph>& graph, 
                                  const variable_map_t& time_applied_map, simulation_todo_sptr_t& state);
                                  
                                  
  virtual CheckEntailmentResult check_entailment(
    vcs::CheckConsistencyResult &cc_result,
    const node_sptr& guard,
    const continuity_map_t& cont_map) = 0;
  
  virtual variable_map_t apply_time_to_vm(const variable_map_t &, const time_t &) = 0;
  
  /**
   * �^����ꂽsimulation_todo_sptr_t�̏��������p�����C
   * �V����simulation_todo_sptr_t�̍쐬
   */
  simulation_todo_sptr_t create_new_simulation_phase(const simulation_todo_sptr_t& old) const;

  /**
   * PP���[�h��IP���[�h��؂�ւ���
   */
  virtual void set_simulation_mode(const Phase& phase) = 0;

protected:

  variable_t* get_variable(const std::string &name, const int &derivative_count){
    return &(*std::find(variable_set_->begin(), variable_set_->end(), (variable_t(name, derivative_count))));
  }

  virtual CalculateVariableMapResult check_false_conditions(const module_set_sptr& ms, simulation_todo_sptr_t&, const variable_map_t &, variable_range_map_t&) = 0;

  const Opts *opts_;
  
  variable_set_t *variable_set_;
  parameter_set_t *parameter_set_;
  variable_range_map_t *variable_map_;
  negative_asks_t prev_guards_;

  int phase_sum_;

  /**
   * graph of relation between module_set for IP and PP
   */
  boost::shared_ptr<RelationGraph> pp_relation_graph_, ip_relation_graph_;
  
  /**
   * ����⃂�W���[���W���̃R���e�i
   * �i��always������������o�[�W�����j
   */
  module_set_container_sptr msc_no_init_;

  todo_container_t* todo_container_;
  
  virtual variable_map_t range_map_to_value_map(phase_result_sptr_t&,
    const variable_range_map_t &,
    parameter_map_t &) = 0;

  
  phase_result_sptr_t make_new_phase(const phase_result_sptr_t& original);
  
  
  void push_branch_states(simulation_todo_sptr_t &original,
    hydla::vcs::CheckConsistencyResult &result);
    
  /// �g�p����\���o�ւ̃|�C���^
  boost::shared_ptr<solver_t> solver_;
  
  /// �P�[�X�̑I�����Ɏg�p����֐��|�C���^
  int (*select_phase_)(result_list_t&);

  private:

  /**
   * merge rhs to lhs
   */
  void merge_variable_map(variable_range_map_t& lhs, variable_range_map_t& rhs);

  result_list_t make_results_from_todo(simulation_todo_sptr_t& todo);
  
  phase_result_sptr_t make_new_phase(simulation_todo_sptr_t& todo, const variable_range_map_t& vm);
};


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
