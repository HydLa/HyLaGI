#ifndef _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_
#define _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_

#include "Simulator.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
using namespace boost;
namespace hydla {
namespace simulator {

class ParallelSimulator: public Simulator{
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


  ParallelSimulator(Opts &opts);
  
  virtual ~ParallelSimulator();
  /**
   * �X���b�h�𗧂��グ�A�V�~�����[�V�������J�n����
   */
  virtual phase_result_const_sptr_t simulate();
  /**
   * �e�X���b�h���^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual void worker_simulate(int thr_id);
  /**
   * �ҋ@�X���b�h�ɂ���X���b�h�̃V�~�����[�V�����I����ʒm
   */
  virtual void notify_simulation_end(int thr_id);
  /**
   * state��pop
   */  
  virtual simulation_phase_sptr_t worker_pop_phase(int thr_id);
  /**
   * state��push
   */  
  virtual void worker_push_phase(const simulation_phase_sptr_t& state,int thr_id);

  virtual void init_thread_state();
  virtual void set_thread_state(int thr_id,std::string str);

  /**
   * �V�~�����[�^�̏��������s��
   */
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
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

  std::vector<module_set_container_sptr> msc_no_inits_;

  thread_group thread_group_;

  mutex mutex_;

  condition condition_;

  volatile bool end_flag_;

  volatile int running_thread_count_;

  std::vector<std::string> thread_state_;

};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_