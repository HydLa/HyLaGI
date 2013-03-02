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
   * 各スレッドが与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate();
  /**
   * 待機スレッドにあるスレッドのシミュレーション終了を通知
   */
  void notify_simulation_end();
  /**
   * stateのpop
   */  
  simulation_todo_sptr_t worker_pop_phase();
  /**
   * stateのpush
   */  
  void worker_push_phase(const simulation_todo_sptr_t& state);
  
  void set_result_root(phase_result_sptr_t r){result_root_ = r;}

  void set_thread_state(std::string str);
  
  private:
  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;
  
  
  /**
   * シミュレーション対象となるパースツリー
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
