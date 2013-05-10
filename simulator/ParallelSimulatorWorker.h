#ifndef _INCLUDED_HYDLA_PARALLEL_SIMULATOR_WORKER_H_
#define _INCLUDED_HYDLA_PARALLEL_SIMULATOR_WORKER_H_

#include "BatchSimulator.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>


namespace hydla {
namespace simulator {

class ParallelSimulator;

class ParallelSimulatorWorker: public BatchSimulator{

public:

  ParallelSimulatorWorker(Opts &opts, ParallelSimulator *master);
  
  virtual ~ParallelSimulatorWorker();

  virtual void initialize(const parse_tree_sptr& parse_tree,int id);
  /**
   * 各スレッドが与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate();
  /**
   * 待機スレッドにあるスレッドのシミュレーション終了を通知
   */
  void notify_simulation_end();
  
  virtual simulation_todo_sptr_t pop_todo();
  
  void set_result_root(phase_result_sptr_t r){result_root_ = r;}

  void set_thread_state(std::string str);

  void print_thread_state();
  
  private:

  ParallelSimulator* master_;

  static boost::condition condition_;

  static bool end_flag_;

  static int running_thread_count_;

  static std::vector<std::string> thread_state_;

  int thr_id_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_PARALLEL_SIMULATOR_WORKER_H_
