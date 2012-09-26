#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "Timer.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {


template<typename PhaseResultType>
class SequentialSimulator:public Simulator<PhaseResultType>{
  public:
  typedef PhaseResultType                                   phase_result_t;
  typedef typename boost::shared_ptr<phase_result_t>        phase_result_sptr;
  typedef typename boost::shared_ptr<const phase_result_t>  phase_result_const_sptr;
  typedef PhaseSimulator<PhaseResultType>                   phase_simulator_t;
  typedef typename phase_result_t::phase_result_sptr_t      phase_result_sptr_t;
  typedef typename std::vector<phase_result_sptr_t >                  phase_result_sptrs_t;

  typedef typename phase_result_t::variable_map_t variable_map_t;
  typedef typename phase_result_t::variable_t     variable_t;
  typedef typename phase_result_t::parameter_t     parameter_t;
  typedef typename phase_result_t::value_t        value_t;
  typedef typename phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;


  SequentialSimulator(Opts &opts):Simulator<phase_result_t>(opts){
  }
  
  virtual ~SequentialSimulator(){}
  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual void simulate()
  {
    std::string error_str;
    while(!state_stack_.empty()) {
      if(Simulator<phase_result_t>::opts_->time_measurement)
	hydla::timer::Timer::update_phase_time();
      phase_result_sptr state(pop_phase_result());
      bool consistent;

        if( Simulator<phase_result_t>::opts_->max_step >= 0 && state->step > Simulator<phase_result_t>::opts_->max_step){
	  state->parent->cause_of_termination = simulator::STEP_LIMIT;
          continue;
	}

      try{
        state->module_set_container->reset(state->visited_module_sets);
        phase_result_sptrs_t phases = Simulator<phase_result_t>::phase_simulator_->simulate_phase(state, consistent);
        if(!phases.empty()){
          if(Simulator<phase_result_t>::opts_->nd_mode){
            for(typename phase_result_sptrs_t::iterator it = phases.begin();it != phases.end();it++){
              if((*it)->parent != Simulator<phase_result_t>::result_root_){
                (*it)->module_set_container = Simulator<phase_result_t>::msc_no_init_;
              }
              else{
                (*it)->module_set_container = (*it)->parent->module_set_container;
              }
	      if(Simulator<phase_result_t>::opts_->time_measurement) hydla::timer::Timer::push_new_phase_time();
              push_phase_result(*it);
            }
          }else{
            if(phases[0]->parent != Simulator<phase_result_t>::result_root_){
              phases[0]->module_set_container = Simulator<phase_result_t>::msc_no_init_;
            }else{
              phases[0]->module_set_container = phases[0]->parent->module_set_container;
            }
	      if(Simulator<phase_result_t>::opts_->time_measurement) hydla::timer::Timer::push_new_phase_time();
            push_phase_result(phases[0]);
          }
        }
      }catch(const std::runtime_error &se){
        error_str = se.what();
        HYDLA_LOGGER_PHASE(se.what());
      }
    }
    if(Simulator<phase_result_t>::opts_->output_format == fmtMathematica){
      Simulator<phase_result_t>::output_result_tree_mathematica();
    }
    else{
      Simulator<phase_result_t>::output_result_tree();
    }
    std::cout << error_str;
  }
  
  virtual void initialize(const parse_tree_sptr& parse_tree){
      Simulator<phase_result_t>::initialize(parse_tree);
      //Simulator::initialize(parse_tree);
      Simulator<phase_result_t>::state_id_ = 1;
      //������Ԃ�����ăX�^�b�N�ɓ����
      phase_result_sptr state(Simulator<phase_result_t>::create_new_phase_result());
      state->phase        = simulator::PointPhase;
      state->step         = 0;
      state->current_time = value_t("0");
      state->module_set_container = Simulator<phase_result_t>::msc_original_;
      state->parent = Simulator<phase_result_t>::result_root_;
      push_phase_result(state);
  }


  /**
   * ��ԃL���[�ɐV���ȏ�Ԃ�ǉ�����
   */
  virtual void push_phase_result(const phase_result_sptr& state)
  {
    state->id = Simulator<phase_result_t>::state_id_++;
    HYDLA_LOGGER_PHASE("%% SequentialSimulator::push_phase_result\n");
    HYDLA_LOGGER_PHASE("%% state Phase: ", state->phase);
    HYDLA_LOGGER_PHASE("%% state id: ", state->id);
    HYDLA_LOGGER_PHASE("%% state time: ", state->current_time);
    HYDLA_LOGGER_PHASE("--- parent state variable map ---\n", state->parent->variable_map);
    HYDLA_LOGGER_PHASE("--- state parameter map ---\n", state->parameter_map);
    state_stack_.push(state);
  }

  /**
   * ��ԃL���[�����Ԃ��ЂƂ��o��
   */
  phase_result_sptr pop_phase_result()
  {
    phase_result_sptr state(state_stack_.top());
    state_stack_.pop();
    return state;
  }
  
  /**
   * �ePhaseResult�ɐU���Ă���ID
   */
  int phase_id_;
  
  /**
   * �V�~�����[�V�������Ŏg�p�����ϐ��\�̌��^
   */
  variable_map_t variable_map_;

  
  /*
   * �V�~�����[�V�������Ɏg�p�����ϐ��ƋL���萔�̏W��
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  //int state_id_;
  

  struct SimulationState {
    phase_result_sptr phase_result;
    /// �t�F�[�Y���ňꎞ�I�ɒǉ����鐧��D���򏈗��ȂǂɎg�p
    constraints_t temporary_constraints;
    module_set_container_sptr module_set_container;
    /// ����ς݂̃��W���[���W����ێ����Ă����D���򏈗����C�����W���𕡐��񒲂ׂ邱�Ƃ������悤��
    std::set<module_set_sptr> visited_module_sets;
  };

  
  /**
   * �e��Ԃ�ۑ����Ă������߂̃X�^�b�N
   */
  std::stack<phase_result_sptr> state_stack_;
  
  
  /**
   * �V�~�����[�V�����ΏۂƂȂ�p�[�X�c���[
   */
  parse_tree_sptr parse_tree_;
  
  bool is_safe_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
