#ifndef _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_
#define _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_

#include "BatchSimulator.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>


namespace hydla {
namespace simulator {

class ParallelSimulatorWorker;

struct ParallelTodoContainer: public BatchTodoContainer
{
  ParallelTodoContainer(SearchMethod& method, boost::shared_ptr<entire_profile_t> vec, boost::recursive_mutex* mut)
    : BatchTodoContainer(method, vec){mutex_ = mut;}

  virtual void push_todo(simulation_todo_sptr_t& todo)
  {
    boost::recursive_mutex::scoped_lock lk(*mutex_);
    BatchTodoContainer::push_todo(todo);
  }
  
  virtual simulation_todo_sptr_t pop_todo()
  {
    boost::recursive_mutex::scoped_lock lk(*mutex_);
    return BatchTodoContainer::pop_todo();
  }
  
  virtual bool empty()
  {
    boost::recursive_mutex::scoped_lock lk(*mutex_);
    return BatchTodoContainer::empty();
  }

  private:
  boost::recursive_mutex* mutex_;
};


class ParallelSimulator: public BatchSimulator{

public:
  ParallelSimulator(Opts &opts);
  
  virtual ~ParallelSimulator();

  /**
   * スレッドを立ち上げ、シミュレーションを開始する
   */
  virtual phase_result_const_sptr_t simulate();

  /**
   * シミュレータの初期化を行う
   */
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  boost::shared_ptr<todo_container_t> get_todo_stack(){return todo_stack_;}
  
  boost::shared_ptr<entire_profile_t> get_profile_vector(){return profile_vector_;}
  
  boost::recursive_mutex& get_mutex(){return mutex_;}

  private:
  
  boost::recursive_mutex mutex_;
  
  boost::thread_group thread_group_;
  
  std::vector<boost::shared_ptr<ParallelSimulatorWorker> > workers_;

};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_
