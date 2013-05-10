#ifndef _INCLUDED_HYDLA_BATCH_SIMULATOR_H_
#define _INCLUDED_HYDLA_BATCH_SIMULATOR_H_

#include "Simulator.h"
#include "Logger.h"

namespace hydla {
namespace simulator {


struct BatchTodoContainer: public todo_container_t
{
  BatchTodoContainer(SearchMethod& method, boost::shared_ptr<entire_profile_t> vec)
    : search_method_(method), todo_id_(0), profile_vector_(vec){}

  virtual void push_todo(simulation_todo_sptr_t& todo)
  {
    todo->id = ++todo_id_;
    container_.push_front(todo);
  }
  
  virtual simulation_todo_sptr_t pop_todo()
  {
    simulation_todo_sptr_t todo;
    HYDLA_LOGGER_REST(__FUNCTION__, container_.size());
    if(search_method_ == DFS){
      todo = container_.front();
      container_.pop_front();
    }else{
      todo = container_.back();
      container_.pop_back();
    }
    HYDLA_LOGGER_REST(__FUNCTION__, container_.size());
    profile_vector_->push_back(todo);
    HYDLA_LOGGER_REST(__FUNCTION__, container_.size());
    return todo;
  }

  private:
  SearchMethod search_method_;
  int todo_id_;
  boost::shared_ptr<entire_profile_t> profile_vector_;
};

class BatchSimulator: public Simulator{
public:
  BatchSimulator(Opts &opts);
  
  virtual ~BatchSimulator();
  
  virtual void process_one_todo(simulation_todo_sptr_t& todo);
  
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  /**
   * @return the result of profiling
   */
  entire_profile_t get_profile(){return *profile_vector_;}

protected:

  /**
   * シミュレーション上のTodoを入れておくコンテナ
   */
  boost::shared_ptr<todo_container_t> todo_stack_;

  /**
   * 各Todoに対応するプロファイリングの結果
   */
  boost::shared_ptr<entire_profile_t> profile_vector_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_BATCH_SIMULATOR_H_