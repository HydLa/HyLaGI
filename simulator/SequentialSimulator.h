#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace simulator {


template<typename PhaseStateType>
class SequentialSimulator:public Simulator<PhaseStateType>{
  public:


  SequentialSimulator(Opts &opts):Simulator(opts){
  }
  
  virtual ~SequentialSimulator(){}
  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual void simulate()
  {
  
    while(!state_stack_.empty() && (is_safe_ || opts_.exclude_error)) {
      phase_state_sptr state(pop_phase_state());
      bool has_next = false;
      try{
        if( opts_.max_step >= 0 && state->step > opts_.max_step)
          continue;
        state->module_set_container->reset(state->visited_module_sets);
        while(state->module_set_container->go_next() && (is_safe_ || opts_.exclude_error)){
          is_safe_ = true;
          phase_state_sptrs_t phases = phase_simulator_->simulate_phase_state(state->module_set_container->get_module_set(), state);
          if(!state->parent->children.empty() || !phases.empty()){
            state->module_set_container->mark_nodes();
            if(opts_.nd_mode){
              for(phase_state_sptrs_t::iterator it = phases.begin();it != phases.end();it++){
                // TODO:���ꂾ�Ǝ���0��PP�ŏꍇ���������������Ƃ��Ƀo�O��
                (*it)->module_set_container = msc_no_init_;
                push_phase_state(*it);
              }
            }else if(!phases.empty()){
              phases[0]->module_set_container = msc_no_init_;
              push_phase_state(phases[0]);
            }
            has_next = true;
            break;
          }
          else{
            state->module_set_container->mark_current_node();
          }
          state->positive_asks.clear();
        }
      }catch(const std::runtime_error &se){
        std::cout << se.what() << std::endl;
        HYDLA_LOGGER_REST(se.what());
      }

      //�������ȉ���⃂�W���[���W�������݂��Ȃ��ꍇ
      if(!has_next){
        state->cause_of_termination = simulator::INCONSISTENCY;
      }
    }
    if(opts_.output_format == fmtMathematica){
      output_result_tree_mathematica();
    }
    else{
      output_result_tree();
    }
  }
  
  virtual void initialize(const parse_tree_sptr& parse_tree){
      Simulator::initialize(parse_tree);
      result_root_.reset(new phase_state_t());
      state_id_ = 1;
      //������Ԃ�����ăX�^�b�N�ɓ����
      phase_state_sptr state(create_new_phase_state());
      state->phase        = simulator::PointPhase;
      state->step         = 0;
      state->current_time = value_t("0");
      state->module_set_container = msc_original_;
      state->parent = result_root_;
      push_phase_state(state);
  }


  /**
   * ��ԃL���[�ɐV���ȏ�Ԃ�ǉ�����
   */
  virtual void push_phase_state(const phase_state_sptr& state) 
  {
    state->id = state_id_++;
    HYDLA_LOGGER_PHASE("%% Simulator::push_phase_state\n");
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
  phase_state_sptr pop_phase_state()
  {
    phase_state_sptr state(state_stack_.top());
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
  

  typedef struct SimulationState {
    phase_state_sptr phase_state;
    /// �t�F�[�Y���ňꎞ�I�ɒǉ����鐧��D���򏈗��ȂǂɎg�p
    constraints_t temporary_constraints;
    module_set_container_sptr module_set_container;
    /// ����ς݂̃��W���[���W����ێ����Ă����D���򏈗����C�����W���𕡐��񒲂ׂ邱�Ƃ������悤��
    std::set<module_set_sptr> visited_module_sets;
  };

  /**
   * �e��Ԃ�ۑ����Ă������߂̃X�^�b�N
   */
  std::stack<phase_state_sptr> state_stack_;
  
  
  /**
   * �V�~�����[�V�����ΏۂƂȂ�p�[�X�c���[
   */
  parse_tree_sptr parse_tree_;
  
  bool is_safe_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_