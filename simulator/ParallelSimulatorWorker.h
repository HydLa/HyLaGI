#ifndef _INCLUDED_HYDLA_PARALLEL_SIMULATOR_WORKER_H_
#define _INCLUDED_HYDLA_PARALLEL_SIMULATOR_WORKER_H_

#include "Simulator.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
using namespace boost;
namespace hydla {
namespace simulator {

class ParallelSimulator;

class ParallelSimulatorWorker: public Simulator{

public:

  ParallelSimulatorWorker(Opts &opts);
  
  virtual ~ParallelSimulatorWorker();

  virtual void initialize(const parse_tree_sptr& parse_tree,int id,ParallelSimulator *master);
  /**
   * �e�X���b�h���^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual phase_result_const_sptr_t simulate();
  /**
   * �ҋ@�X���b�h�ɂ���X���b�h�̃V�~�����[�V�����I����ʒm
   */
  void notify_simulation_end();
  /**
   * state��pop
   */  
  simulation_todo_sptr_t worker_pop_phase();
  /**
   * state��push
   */  
  void worker_push_phase(const simulation_todo_sptr_t& state);
  
  void set_result_root(phase_result_sptr_t r){result_root_ = r;}

  void set_thread_state(std::string str);
  
  private:
  
  /**
   * �V�~�����[�V�������Ŏg�p�����ϐ��\�̌��^
   */
  variable_map_t variable_map_;
  
  
  /**
   * �V�~�����[�V�����ΏۂƂȂ�p�[�X�c���[
   */
  parse_tree_sptr parse_tree_;

  boost::shared_ptr<ParallelSimulator> master_;

  static mutex mutex_;

  static condition condition_;

  static bool end_flag_;

  static int running_thread_count_;

  static std::vector<std::string> thread_state_;

  int thr_id_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_PARALLEL_SIMULATOR_WORKER_H_
