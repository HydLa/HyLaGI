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
#include "TreeInfixPrinter.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

class PhaseSimulator{

public:  


  typedef PhaseResult                                           phase_result_t; 
  typedef boost::shared_ptr<phase_result_t>                     phase_result_sptr_t;
  typedef boost::shared_ptr<const phase_result_t>               phase_result_const_sptr_t;
  typedef std::vector<phase_result_sptr_t>                      phase_result_sptrs_t;
  
  typedef SimulationTodo                                        simulation_phase_t;
  typedef boost::shared_ptr<simulation_phase_t>                 simulation_phase_sptr_t;
  
  
  struct TodoAndResult{
    simulation_phase_sptr_t todo;
    phase_result_sptr_t result;
    TodoAndResult(const simulation_phase_sptr_t& t, const phase_result_sptr_t &r):todo(t), result(r){
    }
    TodoAndResult(){}
  };
  
  typedef std::vector<TodoAndResult>                            todo_and_results_t;
  
  
  typedef PhaseSimulator                                    phase_simulator_t;

  typedef phase_result_t::variable_map_t      variable_map_t;
  typedef phase_result_t::variable_t          variable_t;
  typedef phase_result_t::parameter_t         parameter_t;
  typedef phase_result_t::value_t             value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  typedef value_t                             time_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;

  typedef std::map<module_set_sptr, node_sptr> false_map_t;

  typedef enum{
    FALSE_CONDITIONS_TRUE,
    FALSE_CONDITIONS_FALSE,
    FALSE_CONDITIONS_VARIABLE_CONDITIONS
  } FalseConditionsResult;
  
  PhaseSimulator(const Opts& opts);
  
  virtual ~PhaseSimulator();


  /**
   * ms����������悤�ȏ�����\�ߒ��ׂ�֐�
   * ����������ms��false_conditions_�ɒǉ������
   * @return
   * FALSE_CONDITIONS_TRUE                : �K������
   * FALSE_CONDITIONS_FALSE               : �������Ȃ�
   * FALSE_CONDITIONS_VARIABLE_CONDITIONS : �����ɂ���Ė�������
   */
  virtual FalseConditionsResult find_false_conditions(const module_set_sptr& ms) = 0;

  /**
   * Always��������������������W���[���W���̏W���̏����������疵����������𒲂ׂ�
   * ����������������������ꍇ�ɂ́A
   * ���݂̃��W���[���W�����܂��郂�W���[���W���ɓ���������ǉ�����
   * �K����������ꍇ�͌��݂̃��W���[���W������сA
   * ������܂��郂�W���[���W��������⃂�W���[���W���̏W�������菜��
   */
  virtual void check_all_module_set(module_set_container_sptr& msc_no_init);

  virtual void init_false_conditions(module_set_container_sptr& msc_no_init);

  virtual todo_and_results_t simulate_phase(simulation_phase_sptr_t& state, bool &consistent);
  
  virtual variable_map_t apply_time_to_vm(const variable_map_t &, const time_t &) = 0;
  
  
  /**
   * �V����simulation_phase_sptr_t�̍쐬
   */
  simulation_phase_sptr_t create_new_simulation_phase() const;

  /**
   * �^����ꂽPhaseResult�̏��������p�����C
   * �V����PhaseResult�̍쐬
   */
  simulation_phase_sptr_t create_new_simulation_phase(const simulation_phase_sptr_t& old) const;
  
  
  /**
   * ���W���[���W���̖����������m���߂�֐��DPoint Phase��
   * @return ���ɃV�~�����[�V��������ׂ��t�F�[�Y�̏W��
   */
  virtual todo_and_results_t simulate_ms_point(const module_set_sptr& ms,
                           simulation_phase_sptr_t& state, variable_map_t &, bool& consistent) = 0;

  /**
   * ���W���[���W���̖����������m���߂�֐��DInterval Phase��
   * @return ���ɃV�~�����[�V��������ׂ��t�F�[�Y�̏W��
   */
  virtual todo_and_results_t simulate_ms_interval(const module_set_sptr& ms,
                              simulation_phase_sptr_t& state, bool& consistent) = 0;

  virtual void initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c);

  virtual void set_parameter_set(parameter_t param)
  {
  }
  virtual parameter_set_t get_parameter_set()
  {
    // std::cout << "get param size" << parameter_set_->size() << std::endl;
    return *parameter_set_;
  }
  
  virtual bool is_safe() const{return is_safe_;}
  
protected:

  const Opts *opts_;

  bool is_safe_;
  
  variable_set_t *variable_set_;
  parameter_set_t *parameter_set_;
  variable_map_t *variable_map_;
  false_map_t false_conditions_;
};


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
