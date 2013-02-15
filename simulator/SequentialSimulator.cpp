#include "SequentialSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

namespace hydla {
namespace simulator {

using namespace std;

SequentialSimulator::SequentialSimulator(Opts &opts):Simulator(opts){
}

SequentialSimulator::~SequentialSimulator(){}
/**
 * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
 */
phase_result_const_sptr_t SequentialSimulator::simulate()
{
  std::string error_str;
  while(!state_stack_.empty()) {
    simulation_phase_sptr_t state(pop_simulation_phase());
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
      state->module_set_container->reset(state->ms_to_visit);
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
              tr.todo->module_set_container = msc_original_;
            }
            tr.todo->ms_to_visit = tr.todo->module_set_container->get_full_ms_list();
            tr.todo->elapsed_time = phase_timer.get_elapsed_us() + state->elapsed_time;
            push_simulation_phase(tr.todo);
          }if(tr.result.get() != NULL){
            HYDLA_LOGGER_PHASE("%% push result");
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
  }
  if(!error_str.empty()){
    std::cout << error_str;
  }
  
  HYDLA_LOGGER_PHASE("%% simulation ended");
  return result_root_;
}

phase_result_const_sptr_t SequentialSimulator::get_result_root()
{
  return result_root_;
}

} // simulator
} // hydla