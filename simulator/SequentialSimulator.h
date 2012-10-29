#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "Timer.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {


class SequentialSimulator: public Simulator{
  public:
  typedef PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>           phase_result_const_sptr_t;
  typedef PhaseSimulator                                    phase_simulator_t;
  typedef phase_result_t::phase_result_sptr_t               phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;
  
  typedef SimulationPhase                                   simulation_phase_t;
  typedef std::vector<SimulationPhase>                      simulation_phases_t;

  typedef phase_result_t::variable_map_t variable_map_t;
  typedef phase_result_t::variable_t     variable_t;
  typedef phase_result_t::parameter_t    parameter_t;
  typedef phase_result_t::value_t        value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;


  SequentialSimulator(Opts &opts):Simulator(opts){
  }
  
  virtual ~SequentialSimulator(){}
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual phase_result_const_sptr_t simulate()
  {
    std::string error_str;
    while(!state_stack_.empty()) {
      
      simulation_phase_t state(pop_phase_result());
      phase_result_sptr_t pr = state.phase_result;
      bool consistent;

      if( opts_->max_phase >= 0 && pr->step >= opts_->max_phase){
        pr->parent->cause_of_termination = simulator::STEP_LIMIT;
        continue;
      }

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
        for(int i=0; i<phases.size();i++){
          HYDLA_LOGGER_PHASE("--- Phase", i ," ---");
          HYDLA_LOGGER_PHASE("%% PhaseType: ", pr->phase);
          HYDLA_LOGGER_PHASE("%% id: ", pr->id);
          HYDLA_LOGGER_PHASE("%% time: ", pr->current_time);
          HYDLA_LOGGER_PHASE("--- parameter map ---\n", pr->parameter_map);
        }
      }catch(const std::runtime_error &se){
        error_str = se.what();
        HYDLA_LOGGER_PHASE(se.what());
      }
    }
    /*
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
  
  virtual void initialize(const parse_tree_sptr& parse_tree){
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

  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;

  
  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  
  
  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;
  
  bool is_safe_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
