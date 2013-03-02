#include "ParallelSimulatorWorker.h"
#include "ParallelSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "SymbolicSimulator.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

namespace hydla {
namespace simulator {

using namespace std;

mutex ParallelSimulatorWorker::mutex_;
condition ParallelSimulatorWorker::condition_;
bool ParallelSimulatorWorker::end_flag_;
int ParallelSimulatorWorker::running_thread_count_;
std::vector<std::string> ParallelSimulatorWorker::thread_state_;

ParallelSimulatorWorker::ParallelSimulatorWorker(Opts &opts):Simulator(opts){
}

ParallelSimulatorWorker::~ParallelSimulatorWorker(){}

void ParallelSimulatorWorker::initialize(const parse_tree_sptr& parse_tree,int id,ParallelSimulator *master){
  set_phase_simulator(new hydla::symbolic_simulator::SymbolicSimulator(*opts_));
  master_.reset(master);
  thr_id_ = id;
  end_flag_ = false;
  running_thread_count_ = 0;
  thread_state_.push_back("not running");
  Simulator::initialize(parse_tree);
}

void ParallelSimulatorWorker::set_thread_state(string str){
  thread_state_[thr_id_] = str;
  for(int i=0;i<opts_->parallel_number; i++){
    cout<<"thread "<<i<<": "<<thread_state_[i]<<endl;
  }
  cout<<endl;
}

/**
 * 各スレッドが与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
 */
phase_result_const_sptr_t ParallelSimulatorWorker::simulate()
{
  std::string error_str;
  while(!end_flag_) {
    simulation_todo_sptr_t state(worker_pop_phase());
    if(end_flag_) break;
    if(state->parent.get() != result_root_.get()){
      state->module_set_container = msc_no_init_;
    }
    else{
      state->module_set_container = msc_original_;
    }
    bool consistent;
    {
      if( opts_->max_phase >= 0 && state->parent->step >= opts_->max_phase){
        state->parent->parent->cause_of_termination = simulator::STEP_LIMIT;
        continue;
      }
      HYDLA_LOGGER_PHASE("--- Next Phase---");
      log_simulation_phase(state);
    }

    try{
      state->module_set_container->reset(state->ms_to_visit);
      timer::Timer phase_timer;
      PhaseSimulator::todo_list_t phases = phase_simulator_->simulate_phase(state, consistent);

      if(opts_->dump_in_progress){
        hydla::output::SymbolicTrajPrinter printer;
        for(unsigned int i=0;i<state->parent->children.size();i++){
          printer.output_one_phase(state->parent->children[i]);
        }
      }

      if((opts_->max_phase_expanded <= 0 || phase_id_ < opts_->max_phase_expanded) && !phases.empty()){
        for(unsigned int i = 0;i < phases.size();i++){
          simulation_todo_sptr_t& todo = phases[i];
          if(todo->parent != result_root_)
          {
            todo->module_set_container = msc_no_init_;
          }
          else
          {
            todo->module_set_container = msc_original_;
          }
          todo->ms_to_visit = todo->module_set_container->get_full_ms_list();
          todo->elapsed_time = phase_timer.get_elapsed_us() + state->elapsed_time;
          push_simulation_phase(todo);
          if(todo->parent != state->parent){
            HYDLA_LOGGER_PHASE("%% push result");
            state->parent->children.push_back(todo->parent);
          }
          if(!opts_->nd_mode)break;
        }
      }

      HYDLA_LOGGER_PHASE("%% Result: ", phases.size(), "Phases\n");
      for(unsigned int i = 0; i < phases.size(); i++){
        HYDLA_LOGGER_PHASE("--- Phase", i ," ---");
        log_simulation_phase(phases[i]);
      }
      
      if(!phase_simulator_->is_safe() && opts_->stop_at_failure){
        HYDLA_LOGGER_PHASE("%% Failure of assertion is detected");
        // assertion違反の場合が見つかったので，他のシミュレーションを中断して終了する
        while(!state_stack_.empty()) {
          simulation_todo_sptr_t tmp_state(pop_simulation_phase());
          tmp_state->parent->cause_of_termination = OTHER_ASSERTION;
        }
      }
    }
    catch(const hydla::timeout::TimeOutError &te)
    {
      // タイムアウト発生
      HYDLA_LOGGER_PHASE(te.what());
      state->parent->cause_of_termination = TIME_OUT_REACHED;
      state->parent->parent->children.push_back(state->parent);
    }
    catch(const std::runtime_error &se)
    {
      error_str = se.what();
      HYDLA_LOGGER_PHASE(se.what());
    }
    notify_simulation_end();
  }
  if(!error_str.empty()){
    std::cout << error_str;
  }
 return result_root_;
}

simulation_todo_sptr_t ParallelSimulatorWorker::worker_pop_phase(){
  simulation_todo_sptr_t state;
  mutex::scoped_lock lk(mutex_);
    cout<<"pop_phase at thread "<<thr_id_<<endl;
  while(master_->state_stack_is_empty()){
    if(running_thread_count_>0){
        cout<<"stack is empty. thread "<<thr_id_<<" waiting..."<<endl;
        set_thread_state("waiting");
        condition_.wait(lk);
        continue;
      }else{
        end_flag_=true;
        cout<<"  end_flag_ true"<<endl<<endl;
        return state;
      }
  }
  state = master_->pop_phase();
  running_thread_count_++;
  set_thread_state("simulating phase " + boost::lexical_cast<string>(state->id));
  return state;
}

void ParallelSimulatorWorker::worker_push_phase(const simulation_todo_sptr_t& state){
  mutex::scoped_lock lk(mutex_);
  master_->push_phase(state);
  cout<<"push_phase at thread "<<thr_id_<<" phase " <<boost::lexical_cast<string>(state->id) <<endl<<endl;
}

void ParallelSimulatorWorker::notify_simulation_end(){
  mutex::scoped_lock lk(mutex_);
  running_thread_count_--;
  set_thread_state("simulation finished");
  condition_.notify_all();
}


} // simulator
} // hydla
