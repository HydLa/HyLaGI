#include "SequentialSimulator.h"
#include "Timer.h"
#include "SymbolicOutputter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"

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
    
    simulation_phase_t state(pop_phase_result());
    phase_result_sptr_t& pr = state.phase_result;
    bool consistent;

    if( opts_->max_phase >= 0 && pr->step >= opts_->max_phase){
      pr->parent->cause_of_termination = simulator::STEP_LIMIT;
      continue;
    }
    
    
    HYDLA_LOGGER_PHASE("--- Next Phase---");
    HYDLA_LOGGER_PHASE("%% PhaseType: ", pr->phase);
    HYDLA_LOGGER_PHASE("%% id: ", pr->id);
    HYDLA_LOGGER_PHASE("%% step: ", pr->step);
    HYDLA_LOGGER_PHASE("%% time: ", *pr->current_time);
    HYDLA_LOGGER_PHASE("--- temporary_constraints ---\n", state.temporary_constraints);
    HYDLA_LOGGER_PHASE("--- parameter map ---\n", pr->parameter_map);

    try{
      state.module_set_container->reset(state.visited_module_sets);
      simulation_phases_t phases = phase_simulator_->simulate_phase(state, consistent);

      if(!phases.empty()){
        if(opts_->nd_mode){
          for(simulation_phases_t::iterator it = phases.begin();it != phases.end();it++){
            if(it->phase_result->parent != result_root_){
              it->module_set_container = msc_no_init_;
            }
            else{
              // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
              it->module_set_container = msc_original_;
            }
            push_simulation_phase(*it);
          }
        }else{
          if(phases[0].phase_result->parent != result_root_){
            phases[0].module_set_container = msc_no_init_;
          }else{
              // TODO これだと，最初のPPで分岐が起きた時のモジュール集合がおかしくなるはず
            phases[0].module_set_container = msc_original_;
          }
          push_simulation_phase(phases[0]);
        }
        
      }
      
      
      HYDLA_LOGGER_PHASE("%% Result: ", phases.size(), "Phases\n");
      for(unsigned int i=0; i<phases.size();i++){
        if(opts_->dump_in_progress){
          hydla::output::SymbolicOutputter outputter;
        }
        pr = phases[i].phase_result;
        HYDLA_LOGGER_PHASE("--- Phase", i ," ---");
        HYDLA_LOGGER_PHASE("%% PhaseType: ", pr->phase);
        HYDLA_LOGGER_PHASE("%% id: ", pr->id);
        HYDLA_LOGGER_PHASE("%% step: ", pr->step);
        HYDLA_LOGGER_PHASE("%% time: ", *pr->current_time);
        HYDLA_LOGGER_PHASE("--- parameter map ---\n", pr->parameter_map);
      }
    }catch(const std::runtime_error &se){
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
    simulation_phase_t state(create_new_simulation_phase());
    phase_result_sptr_t pr = state.phase_result;
    
    pr->phase        = simulator::PointPhase;
    pr->step         = 0;
    pr->current_time = value_t(new hydla::symbolic_simulator::SymbolicValue("0"));
    state.module_set_container = msc_original_;
    pr->parent = result_root_;
    pr->phase_timer.reset();
    pr->calculate_closure_timer.reset();
    push_simulation_phase(state);
}



} // simulator
} // hydla