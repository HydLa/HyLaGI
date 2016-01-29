#include "SequentialSimulator.h"
#include "Timer.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "SignalHandler.h"
#include "Logger.h"
#include <mutex>
#include <thread>

namespace hydla {
namespace simulator {

using namespace std;

SequentialSimulator::SequentialSimulator(Opts &opts):Simulator(opts), printer(backend)
{
}

SequentialSimulator::~SequentialSimulator()
{
}

phase_result_sptr_t SequentialSimulator::simulate()
{
  std::string error_str = "";
  make_initial_todo();

  try
  {
    dfs(result_root_, 0, opts_->num_phasesimulators, 0);
  }
  catch(const std::runtime_error &se)
  {
    error_str += "error ";
    error_str += ": ";
    error_str += se.what();
    error_str += "\n";
    HYDLA_LOGGER_DEBUG_VAR(error_str);
    std::cout << error_str;
    exit_status = EXIT_FAILURE;
  }

  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}

void SequentialSimulator::dfs(phase_result_sptr_t current, int ps_begin, int ps_end, int level)
{
  HYDLA_LOGGER_DEBUG_VAR(*current);
  if(signal_handler::interrupted)
  {
    current->simulation_state = INTERRUPTED;
    return;
  }
  (*phase_simulators_)[ps_begin]->apply_diff(*current);
  while(!current->todo_list.empty())
  {
    const int num_todo = current->todo_list.size();
    const int num_ps = ps_end - ps_begin;
    const int num_threads = num_todo>num_ps ? num_ps : num_todo;
    const int ps_per_dfs = num_ps/num_threads;
    std::mutex current_mtx;
    bool break_flag = false;
    auto dfs_worker = [&current_mtx,&current,&break_flag,level,this](int ps_begin, int ps_end)
    {
      // ここでToDoを追加
      current_mtx.lock();
      phase_result_sptr_t todo = current->todo_list.front();
      current->todo_list.pop_front();
      current_mtx.unlock();
      profile_vector_->insert(todo);
      //
      if(todo->simulation_state == NOT_SIMULATED)
      {
        process_one_todo(todo, ps_begin);
        if(opts_->dump_in_progress){
          printer.output_one_phase(todo, "------ In Progress ------");
        }
      }

      dfs(todo, ps_begin, ps_end, level+1);
      if(!opts_->nd_mode || (opts_->stop_at_failure && assertion_failed) )
      {
        current_mtx.lock();
        omit_following_todos(current);
        current_mtx.unlock();
        break_flag = true;
      }
      //
    };
    std::vector<std::thread> threads(num_threads);
    for (int i = 0; i < num_threads; ++i) {
      if(i!=0) {
        (*phase_simulators_)[ps_begin+i*ps_per_dfs]->relation_graph_.reset(new RelationGraph(*((*phase_simulators_)[ps_begin]->relation_graph_)));
        (*phase_simulators_)[ps_begin+i*ps_per_dfs]->increment_phase_sum();
      }
      std::cerr << "level" << level << " thread" << i << " " << ps_begin+i*ps_per_dfs << "-" << ps_begin+(i+1)*ps_per_dfs << std::endl;
      threads[i] = std::thread(dfs_worker, ps_begin+i*ps_per_dfs, ps_begin+(i+1)*ps_per_dfs);
    }
    for (int i = 0; i < num_threads; ++i) {
      threads[i].join();
    }
    if(break_flag)
      break;
  }
  (*phase_simulators_)[ps_begin]->revert_diff(*current);
}

void SequentialSimulator::omit_following_todos(phase_result_sptr_t current)
{
  while(!current->todo_list.empty())
  {
    phase_result_sptr_t not_selected_children = current->todo_list.front();
    current->todo_list.pop_front();
    if(not_selected_children->simulation_state != SIMULATED)
    {
      current->children.push_back(not_selected_children);
    }
    not_selected_children->simulation_state = NOT_SIMULATED;
  }
}


} // simulator
} // hydla
