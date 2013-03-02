#ifndef _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_
#define _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_

#include "Simulator.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
using namespace boost;
namespace hydla {
namespace simulator {

class ParallelSimulatorWorker;

class ParallelSimulator: public Simulator{

public:
  ParallelSimulator(Opts &opts);
  
  virtual ~ParallelSimulator();

  void push_phase(const simulation_todo_sptr_t& state);
  simulation_todo_sptr_t pop_phase();
  /**
   * �X���b�h�𗧂��グ�A�V�~�����[�V�������J�n����
   */
  virtual phase_result_const_sptr_t simulate();

  /**
   * �V�~�����[�^�̏��������s��
   */
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  phase_result_const_sptr_t get_result_root();

  bool state_stack_is_empty();

  private:
  
  /**
   * �V�~�����[�V�������Ŏg�p�����ϐ��\�̌��^
   */
  variable_map_t variable_map_;  
  
  /**
   * parse tree to simulate
   */
  parse_tree_sptr parse_tree_;

  thread_group thread_group_;
  
  std::vector<boost::shared_ptr<ParallelSimulatorWorker> > workers_;

};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_
