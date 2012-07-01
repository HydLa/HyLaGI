#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace simulator {


template<typename PhaseStateType>
class SequentialSimulator:public Simulator<PhaseStateType>{
  public:
  typedef PhaseStateType                                   phase_state_t;
  typedef typename boost::shared_ptr<phase_state_t>        phase_state_sptr;
  typedef typename boost::shared_ptr<const phase_state_t>  phase_state_const_sptr;
  typedef PhaseSimulator<PhaseStateType>                   phase_simulator_t;
  typedef typename phase_state_t::phase_state_sptr_t      phase_state_sptr_t;
  typedef typename std::vector<phase_state_sptr_t >                  phase_state_sptrs_t;

  typedef typename phase_state_t::variable_map_t variable_map_t;
  typedef typename phase_state_t::variable_t     variable_t;
  typedef typename phase_state_t::parameter_t     parameter_t;
  typedef typename phase_state_t::value_t        value_t;
  typedef typename phase_state_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;


  SequentialSimulator(Opts &opts):Simulator<phase_state_t>(opts){
  }
  
  virtual ~SequentialSimulator(){}
  /**
   * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
   */
  virtual void simulate()
  {
    while(!state_stack_.empty()) {
      phase_state_sptr state(pop_phase_state());
      bool consistent;
      try{
        if( Simulator<phase_state_t>::opts_->max_step >= 0 && state->step > Simulator<phase_state_t>::opts_->max_step)
          continue;
        state->module_set_container->reset(state->visited_module_sets);
        phase_state_sptrs_t phases = Simulator<phase_state_t>::phase_simulator_->simulate_phase_state(state, consistent);
        if(!phases.empty()){
          if(Simulator<phase_state_t>::opts_->nd_mode){
            for(typename phase_state_sptrs_t::iterator it = phases.begin();it != phases.end();it++){
              if(consistent){
                (*it)->module_set_container = Simulator<phase_state_t>::msc_no_init_;
              }
              else{
                (*it)->module_set_container = (*it)->parent->module_set_container;
              }
              push_phase_state(*it);
            }
          }else{
            if(consistent){
              phases[0]->module_set_container = Simulator<phase_state_t>::msc_no_init_;
            }else{
              phases[0]->module_set_container = phases[0]->parent->module_set_container;
            }
            push_phase_state(phases[0]);
          }
        }
      }catch(const std::runtime_error &se){
        std::cout << se.what() << std::endl;
        HYDLA_LOGGER_REST(se.what());
      }
    }
    if(Simulator<phase_state_t>::opts_->output_format == fmtMathematica){
      Simulator<phase_state_t>::output_result_tree_mathematica();
    }
    else{
      Simulator<phase_state_t>::output_result_tree();
    }
  }
  
  virtual void initialize(const parse_tree_sptr& parse_tree){
      Simulator<phase_state_t>::initialize(parse_tree);
      //Simulator::initialize(parse_tree);
      Simulator<phase_state_t>::state_id_ = 1;
      //初期状態を作ってスタックに入れる
      phase_state_sptr state(Simulator<phase_state_t>::create_new_phase_state());
      state->phase        = simulator::PointPhase;
      state->step         = 0;
      state->current_time = value_t("0");
      state->module_set_container = Simulator<phase_state_t>::msc_original_;
      state->parent = Simulator<phase_state_t>::result_root_;
      push_phase_state(state);
  }


  /**
   * 状態キューに新たな状態を追加する
   */
  virtual void push_phase_state(const phase_state_sptr& state)
  {
    state->id = Simulator<phase_state_t>::state_id_++;
    HYDLA_LOGGER_PHASE("%% SequentialSimulator::push_phase_state\n");
    HYDLA_LOGGER_PHASE("%% state Phase: ", state->phase);
    HYDLA_LOGGER_PHASE("%% state id: ", state->id);
    HYDLA_LOGGER_PHASE("%% state time: ", state->current_time);
    HYDLA_LOGGER_PHASE("--- parent state variable map ---\n", state->parent->variable_map);
    HYDLA_LOGGER_PHASE("--- state parameter map ---\n", state->parameter_map);
    state_stack_.push(state);
  }

  /**
   * 状態キューから状態をひとつ取り出す
   */
  phase_state_sptr pop_phase_state()
  {
    phase_state_sptr state(state_stack_.top());
    state_stack_.pop();
    return state;
  }
  
  /**
   * 各PhaseResultに振っていくID
   */
  int phase_id_;
  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;

  
  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  //int state_id_;
  

  struct SimulationState {
    phase_state_sptr phase_state;
    /// フェーズ内で一時的に追加する制約．分岐処理などに使用
    constraints_t temporary_constraints;
    module_set_container_sptr module_set_container;
    /// 判定済みのモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
    std::set<module_set_sptr> visited_module_sets;
  };

  
  /**
   * 各状態を保存しておくためのスタック
   */
  std::stack<phase_state_sptr> state_stack_;
  
  
  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;
  
  bool is_safe_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
