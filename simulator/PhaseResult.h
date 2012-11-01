#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "DefaultVariable.h"
#include "ValueRange.h"
#include "./Types.h"
#include "ModuleSet.h"

#include "Timer.h"

namespace hydla {
namespace simulator {

class DefaultParameter;

/**
 * ����t�F�[�Y�̏���\���N���X
 */

struct PhaseResult {
  typedef boost::shared_ptr<PhaseResult>                    phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;


  typedef boost::shared_ptr<Value>                          value_t;
  typedef ValueRange                                        range_t;
  typedef DefaultVariable                                   variable_t;
  typedef DefaultParameter                                  parameter_t;
  typedef value_t                                           time_t;
  typedef std::map<variable_t*, value_t>                   variable_map_t;
  typedef std::map<parameter_t*, range_t>                   parameter_map_t;

  Phase                     phase;
  int id;
  time_t                    current_time, end_time;
  variable_map_t            variable_map;
  parameter_map_t           parameter_map;
  expanded_always_t         expanded_always;
  positive_asks_t           positive_asks;
  changed_asks_t            changed_asks;
  int step;
  hydla::ch::module_set_sptr module_set;

  /// �t�F�[�Y�̏I��������\���D
  CauseOfTermination cause_of_termination;
  /// ���̃t�F�[�Y
  phase_result_sptrs_t children;
  /// �O�̃t�F�[�Y
  phase_result_sptr_t parent;

  /// �t�F�[�Y�̏����ɂ�����������
  timer::Timer phase_timer;
  /// �t�F�[�Y�̏�������calculate_closure�ɂ�����������
  timer::Timer calculate_closure_timer;

};

std::ostream& operator<<(std::ostream& s, const PhaseResult::parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const PhaseResult::variable_map_t& vm);

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_