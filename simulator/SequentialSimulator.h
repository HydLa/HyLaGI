#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace simulator {

class SequentialSimulator: public Simulator{
  public:
  typedef PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>           phase_result_const_sptr_t;
  typedef PhaseSimulator                                    phase_simulator_t;
  typedef phase_result_t::phase_result_sptr_t               phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;
  
  typedef SimulationTodo                                   simulation_phase_t;
  typedef boost::shared_ptr<SimulationTodo>                simulation_phase_sptr_t;
  typedef std::vector<simulation_phase_sptr_t>              simulation_phases_t;

  typedef phase_result_t::variable_map_t variable_map_t;
  typedef phase_result_t::variable_t     variable_t;
  typedef phase_result_t::parameter_t    parameter_t;
  typedef phase_result_t::value_t        value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;


  SequentialSimulator(Opts &opts);
  
  virtual ~SequentialSimulator();
  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual phase_result_const_sptr_t simulate();
  
  phase_result_const_sptr_t get_result_root();
  
  private:
  
  /**
   * �V�~�����[�V�������Ŏg�p�����ϐ��\�̌��^
   */
  variable_map_t variable_map_;

  
  /*
   * �V�~�����[�V�������Ɏg�p�����ϐ��ƋL���萔�̏W��
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  
  
  /**
   * �V�~�����[�V�����ΏۂƂȂ�p�[�X�c���[
   */
  parse_tree_sptr parse_tree_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_