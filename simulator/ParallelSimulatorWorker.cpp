#include "ParallelSimulatorWorker.h"
#include "ParallelSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "SymbolicPhaseSimulator.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

namespace hydla {
namespace simulator {

using namespace std;
using namespace boost;

condition ParallelSimulatorWorker::condition_;
bool ParallelSimulatorWorker::end_flag_;
int ParallelSimulatorWorker::running_thread_count_;
std::vector<std::string> ParallelSimulatorWorker::thread_state_;

ParallelSimulatorWorker::ParallelSimulatorWorker(Opts &opts, ParallelSimulator *master):NoninteractiveSimulator(opts), master_(master)
{
}

ParallelSimulatorWorker::~ParallelSimulatorWorker(){}

void ParallelSimulatorWorker::initialize(const parse_tree_sptr& parse_tree, int id)
{
  init_module_set_container(parse_tree);
  set_phase_simulator(new hydla::simulator::symbolic::SymbolicPhaseSimulator(*opts_));
  thr_id_ = id;
  end_flag_ = false;
  running_thread_count_ = 0;
  thread_state_.push_back("not running");
  todo_stack_ = master_->get_todo_stack();
  set_result_root(master_->get_result_root());
  profile_vector_ = master_->get_profile_vector();
  variable_set_ = master_->variable_set_;
  original_range_map_ = master_->original_range_map_;
  parameter_set_ = master_->parameter_set_;
  hydla::parse_tree::ParseTree::variable_map_t vm = parse_tree_->get_variable_map();
  phase_simulator_->initialize(*variable_set_, *parameter_set_,
   *original_range_map_, vm, msc_no_init_);
}

void ParallelSimulatorWorker::set_thread_state(string str)
{
  thread_state_[thr_id_] = str;
}


void ParallelSimulatorWorker::print_thread_state()
{
  for(int i=0;i<opts_->parallel_number; i++){
    cout << "thread " << i << ": " << thread_state_[i] << endl;
  }
  cout << endl;
}

/**
 * 各スレッドが与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
 */
phase_result_const_sptr_t ParallelSimulatorWorker::simulate()
{
  std::string error_str;
  while(!end_flag_) {
    simulation_todo_sptr_t todo(pop_todo());
    if(end_flag_) break;
    running_thread_count_++;
    set_thread_state("simulating phase " + boost::lexical_cast<string>(todo->id));
    if(todo->parent.get() != result_root_.get()){
      todo->module_set_container = msc_no_init_;
    }
    else{
      todo->module_set_container = msc_original_;
    }
      todo->ms_to_visit = todo->module_set_container->get_full_ms_list();
    process_one_todo(todo);
    notify_simulation_end();
  }
  if(!error_str.empty())
  {
    std::cout << error_str;
  }
  return result_root_;
}

simulation_todo_sptr_t ParallelSimulatorWorker::pop_todo()
{
  simulation_todo_sptr_t todo;
  recursive_mutex::scoped_lock lk(master_->get_mutex());
  cout << "pop_todo at thread " << thr_id_ << endl;
  
  while(todo_stack_->empty()){
    if(running_thread_count_>0)
    {
      cout << "stack is empty. thread " << thr_id_ << " waiting..." << endl;
      set_thread_state("waiting");
      print_thread_state();
      condition_.wait(lk);
      continue;
    }
    else
    {
      end_flag_=true;
      cout << "  end_flag_ true\n" << endl;
      return todo;
    }
  }
  todo = todo_stack_->pop_todo();
  cout << "thread " << thr_id_ << " got new todo" << endl;
  return todo;
}

void ParallelSimulatorWorker::notify_simulation_end()
{
  recursive_mutex::scoped_lock lk(master_->get_mutex());
  running_thread_count_--;
  set_thread_state("simulation finished");
  print_thread_state();
  condition_.notify_all();
}


} // simulator
} // hydla
