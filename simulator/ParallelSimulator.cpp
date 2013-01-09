#include "ParallelSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

namespace hydla {
namespace simulator {


using namespace std;

ParallelSimulator::ParallelSimulator(Opts &opts):Simulator(opts){
}

ParallelSimulator::~ParallelSimulator(){}

/**
 * スレッドを立ち上げ、シミュレーションを開始する
 */
ParallelSimulator::phase_result_const_sptr_t ParallelSimulator::simulate()
{
  init_thread_state();

  for(int i=0; i<opts_->parallel_number; i++){
    thread_group_.create_thread(boost::bind(boost::bind(&ParallelSimulator::worker_simulate,this,_1),i));
  }
  thread_group_.join_all();
  return result_root_;
}

/**
 * 各スレッドが与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
 */
void ParallelSimulator::worker_simulate(int thr_id)
{
  std::string error_str;
  while(!end_flag_) {
    simulation_phase_sptr_t state(worker_pop_phase(thr_id));
    if(end_flag_) break;
    bool consistent;
    {
      phase_result_sptr_t& pr = state->phase_result;
      if( opts_->max_phase >= 0 && pr->step >= opts_->max_phase){
        pr->parent->cause_of_termination = simulator::STEP_LIMIT;
        continue;
      }
      
      HYDLA_LOGGER_PHASE("--- Next Phase---");
      HYDLA_LOGGER_PHASE("%% PhaseType: ", pr->phase);
      HYDLA_LOGGER_PHASE("%% id: ", pr->id);
      HYDLA_LOGGER_PHASE("%% step: ", pr->step);
      HYDLA_LOGGER_PHASE("%% time: ", *pr->current_time);
      HYDLA_LOGGER_PHASE("--- temporary_constraints ---\n", state->temporary_constraints);
      HYDLA_LOGGER_PHASE("--- parameter map ---\n", pr->parameter_map);
    }

    try{
      state->module_set_container->reset(state->visited_module_sets);
      timer::Timer phase_timer;
      PhaseSimulator::todo_and_results_t phases = phase_simulator_->simulate_phase(state, consistent);
      
      if(opts_->dump_in_progress){
        hydla::output::SymbolicTrajPrinter printer;
        for(unsigned int i=0;i<state->phase_result->parent->children.size();i++){
          printer.output_one_phase(state->phase_result->parent->children[i]);
        }
      }

      if((opts_->max_phase_expanded <= 0 || phase_id_ < opts_->max_phase_expanded) && !phases.empty()){
        for(unsigned int i = 0;i < phases.size();i++){
          PhaseSimulator::TodoAndResult& tr = phases[i];
          if(tr.todo.get() != NULL){
            if(tr.todo->phase_result->parent != result_root_){
              tr.todo->module_set_container = msc_no_init_;
            }
            else{
              // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
              tr.todo->module_set_container = msc_original_;
            }
            tr.todo->elapsed_time = phase_timer.get_elapsed_us() + state->elapsed_time;
            worker_push_phase(tr.todo,thr_id);
          }if(tr.result.get() != NULL){
            state->phase_result->parent->children.push_back(tr.result);
          }
          if(!opts_->nd_mode)break;
        }
      }
      
      HYDLA_LOGGER_PHASE("%% Result: ", phases.size(), "Phases\n");
      for(unsigned int i=0; i<phases.size();i++){
        if(phases[i].todo.get() != NULL){
          phase_result_sptr_t& pr = phases[i].todo->phase_result;
          HYDLA_LOGGER_PHASE("--- Phase", i ," ---");
          HYDLA_LOGGER_PHASE("%% PhaseType: ", pr->phase);
          HYDLA_LOGGER_PHASE("%% id: ", pr->id);
          HYDLA_LOGGER_PHASE("%% step: ", pr->step);
          HYDLA_LOGGER_PHASE("%% time: ", *pr->current_time);
          HYDLA_LOGGER_PHASE("--- parameter map ---\n", pr->parameter_map);
        }
      }
      
      if(!phase_simulator_->is_safe() && opts_->stop_at_failure){
        HYDLA_LOGGER_PHASE("%% Failure of assertion is detected");
        // assertion違反の場合が見つかったので，他のシミュレーションを中断して終了する
        while(!state_stack_.empty()) {
          simulation_phase_sptr_t tmp_state(pop_simulation_phase());
          tmp_state->phase_result->parent->cause_of_termination = OTHER_ASSERTION;
        }
      }
    }
    catch(const hydla::timeout::TimeOutError &te)
    {
      // タイムアウト発生
      phase_result_sptr_t& pr = state->phase_result;
      HYDLA_LOGGER_PHASE(te.what());
      pr->cause_of_termination = TIME_OUT_REACHED;
      pr->parent->children.push_back(pr);
    }
    catch(const std::runtime_error &se)
    {
      error_str = se.what();
      HYDLA_LOGGER_PHASE(se.what());
    }
    notify_simulation_end(thr_id);

  }
  if(!error_str.empty()){
    std::cout << error_str;
  }
}



void ParallelSimulator::notify_simulation_end(int thr_id){
  mutex::scoped_lock lk(mutex_);
  running_thread_count_--;
  set_thread_state(thr_id,"simulation finished");
  condition_.notify_all();
}

simulation_phase_sptr_t ParallelSimulator::worker_pop_phase(int thr_id){
  simulation_phase_sptr_t state = shared_ptr<ParallelSimulator::simulation_phase_t>(new ParallelSimulator::simulation_phase_t);
  mutex::scoped_lock lk(mutex_);
    cout<<"pop_phase at thread "<<thr_id<<endl;
  while(state_stack_.empty()){
      if(running_thread_count_>0){
        cout<<"thread "<<thr_id<<" waiting..."<<endl;
        set_thread_state(thr_id,"waiting");
        condition_.wait(lk);
        continue;
      }else{
        end_flag_=true;
        cout<<"  end_flag_ true"<<endl<<endl;
        return state;
      }
  }
  state = pop_simulation_phase();
  running_thread_count_++;
  set_thread_state(thr_id,"simulating phase " + boost::lexical_cast<string>(state->phase_result->id));
  return state;
}

void ParallelSimulator::worker_push_phase(const simulation_phase_sptr_t& state,int thr_id){
  mutex::scoped_lock lk(mutex_);
  push_simulation_phase(state);
  cout<<"push_phase at thread "<<thr_id<<" phase " <<boost::lexical_cast<string>(state->phase_result->id) <<endl<<endl;
}

void ParallelSimulator::init_thread_state(){
  for(int i=0;i<opts_->parallel_number; i++){
    thread_state_.push_back("not running");
  }
}

void ParallelSimulator::set_thread_state(int thr_id,string str){
  thread_state_[thr_id] = str;
  for(int i=0;i<opts_->parallel_number; i++){
    cout<<"thread "<<i<<": "<<thread_state_[i]<<endl;
  }
  cout<<endl;
}

ParallelSimulator::phase_result_const_sptr_t ParallelSimulator::get_result_root()
{
  return result_root_;
}

void ParallelSimulator::initialize(const parse_tree_sptr& parse_tree){
    running_thread_count_ = 0;
    end_flag_ = false;
    Simulator::initialize(parse_tree);
    //初期状態を作ってスタックに入れる
    simulation_phase_sptr_t state(new simulation_phase_t());
    state->elapsed_time = 0;
    phase_result_sptr_t &pr = state->phase_result;
    pr.reset(new phase_result_t());
    pr->cause_of_termination = NONE;
    
    pr->phase        = simulator::PointPhase;
    pr->step         = 0;
    pr->current_time = value_t(new hydla::symbolic_simulator::SymbolicValue("0"));
    state->module_set_container = msc_original_;
    pr->parent = result_root_;
    push_simulation_phase(state);
}



} // simulator
} // hydla