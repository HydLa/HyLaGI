#ifndef _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_
#define _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace simulator {

class SequentialSimulator: public Simulator{
public:
  SequentialSimulator(Opts &opts);
  
  virtual ~SequentialSimulator();
  /**
   * �^����ꂽ����⃂�W���[���W�������ɃV�~�����[�V�������s�������Ȃ�
   */
  virtual phase_result_const_sptr_t simulate();
  
  
  /**
   * @return the result of profiling
   */
  entire_profile_t get_profile(){return profile_vector_;}

protected:

  /**
   * Todo�L���[�ɐV����Todo��ǉ�����
   * TODO: ���̊֐����CPhaseSimulator��������Ă΂��悤�ɂ���iTodo��ID�̐������̂��߁j
   */
  virtual void push_simulation_todo(const simulation_todo_sptr_t& todo);

  /**
   * ��ԃL���[�����Ԃ��ЂƂ��o��
   * ���̊֐��Ŏ�肾����simulation_todo_sptr_t�ɂ��Ă͕K���V�~�����[�V�������s�����Ƃ�O��Ƃ��Ă���D
   * (�v���t�@�C�����O���ʂ̐������̂��߁j
   */
  simulation_todo_sptr_t pop_simulation_phase();
  
  /**
   * �V�~�����[�V�������Todo�����Ă����R���e�i
   */
  todo_container_t todo_stack_;
  
  /**
   * �eTodo�ɐU���Ă���ID
   */
  int todo_id_;

  /**
   * �eTodo�ɑΉ�����v���t�@�C�����O�̌���
   */
  entire_profile_t profile_vector_;
};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_SEQUENTIAL_SIMULATOR_H_