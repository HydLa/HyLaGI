#include "SequentialSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../virtual_constraint_solver/TimeOutError.h"

namespace hydla {
namespace simulator {


SequentialSimulator::SequentialSimulator(Opts &opts):Simulator(opts){
}

SequentialSimulator::~SequentialSimulator(){}
/**
 * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
 */
SequentialSimulator::phase_result_const_sptr_t SequentialSimulator::simulate()
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
      try{
        state->module_set_container->reset(state->visited_module_sets);
        simulation_phases_t phases = phase_simulator_->simulate_phase(state, consistent);

        if(!phases.empty()){
          if(opts_->nd_mode){
            for(simulation_phases_t::iterator it = phases.begin();it != phases.end();it++){
              if((*it)->phase_result->parent != result_root_){
                (*it)->module_set_container = msc_no_init_;
              }
              else{
                // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
                (*it)->module_set_container = msc_original_;
              }
              push_simulation_phase(*it);
            }
          }else{
            if(phases[0]->phase_result->parent != result_root_){
              phases[0]->module_set_container = msc_no_init_;
            }else{
                // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
              phases[0]->module_set_container = msc_original_;
            }
            push_simulation_phase(phases[0]);
          }
        }
        
        HYDLA_LOGGER_PHASE("%% Result: ", phases.size(), "Phases\n");
        for(unsigned int i=0; i<phases.size();i++){
          phase_result_sptr_t& pr = phases[i]->phase_result;
          if(opts_->dump_in_progress){
            hydla::output::SymbolicTrajPrinter printer;
            printer.output_one_phase(phases[i]->phase_result);
          }
          HYDLA_LOGGER_PHASE("--- Phase", i ," ---");
          HYDLA_LOGGER_PHASE("%% PhaseType: ", pr->phase);
          HYDLA_LOGGER_PHASE("%% id: ", pr->id);
          HYDLA_LOGGER_PHASE("%% step: ", pr->step);
          HYDLA_LOGGER_PHASE("%% time: ", *pr->current_time);
          HYDLA_LOGGER_PHASE("--- parameter map ---\n", pr->parameter_map);
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
      catch(const hydla::vcs::TimeOutError &te)
      {
        phase_result_sptr_t& pr = state->phase_result;
        HYDLA_LOGGER_PHASE(te.what());
        if(pr->children.empty()){
          pr->cause_of_termination = TIME_OUT_REACHED;
        }else{
          for(unsigned int i=0;i<pr->children.size();i++){
            pr->children[i]->cause_of_termination = TIME_OUT_REACHED;
          }
        }
      }
    }catch(const std::runtime_error &se)
    {
      error_str = se.what();
      HYDLA_LOGGER_PHASE(se.what());
    }
  }
  /*
  TODO: 以前の状態に戻すべき
  if(Simulator<phase_result_t>::opts_->output_format == fmtMathematica){
    Simulator<phase_result_t>::output_result_tree_mathematica();
  }
  else{
    Simulator<phase_result_t>::output_result_tree();
  }
  if(Simulator<phase_result_t>::opts_->time_measurement){
    Simulator<phase_result_t>::output_result_tree_time();
  }
  */
  if(!error_str.empty()){
    std::cout << error_str;
  }
  return result_root_;
}

void SequentialSimulator::initialize(const parse_tree_sptr& parse_tree){
    Simulator::initialize(parse_tree);
    //初期状態を作ってスタックに入れる
    simulation_phase_sptr_t state(new simulation_phase_t());
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