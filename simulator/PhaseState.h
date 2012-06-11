#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "DefaultVariable.h"
#include "ValueRange.h"
#include "./VariableMap.h"
#include "./Types.h"

namespace hydla {
namespace simulator {

template<typename value> class DefaultParameter;

/**
 * ����i�K�ł̏����̏�Ԃ�\���N���X
 */
template<typename ValueType>
struct PhaseState {
  typedef ValueType                                         value_t;
  typedef ValueRange<value_t>                               range_t;
  typedef DefaultVariable                                   variable_t;
  typedef DefaultParameter<value_t>                         parameter_t;
  typedef value_t                                           time_t;
  typedef VariableMap<variable_t*, value_t>             variable_map_t;
  typedef VariableMap<parameter_t*, range_t>            parameter_map_t;
  typedef boost::shared_ptr<PhaseState>                     phase_state_sptr_t;
  typedef std::vector<phase_state_sptr_t >                  phase_state_sptrs_t;

  Phase                     phase;
  int id;
  time_t                    current_time;
  time_t                    end_time;
  variable_map_t            variable_map;
  parameter_map_t           parameter_map;
  expanded_always_id_t      expanded_always_id;
  positive_asks_t           positive_asks;
  changed_asks_t            changed_asks;
  /// �t�F�[�Y���ňꎞ�I�ɒǉ����鐧��D���򏈗��ȂǂɎg�p
  constraints_t temporary_constraints;
  module_set_container_sptr module_set_container;
  /// ����ς݂̃��W���[���W����ێ����Ă����D���򏈗����C�����W���𕡐��񒲂ׂ邱�Ƃ������悤��
  std::set<module_set_sptr> visited_module_sets;
  /// �V�~�����[�V�������s�X�e�b�v���DIP ����x�I���邲�Ƃ�1��������
  int step;

  /// �t�F�[�Y�̏I����ԁD���̃t�F�[�Y�������Ƃ��݈̂Ӗ�������
  CauseOfTermination cause_of_termination;
  /// ���̃t�F�[�Y
  phase_state_sptrs_t children;
  /// �O�̃t�F�[�Y
  phase_state_sptr_t parent;
};


} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_STATE_H_

