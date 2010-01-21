#ifndef _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
#define _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_

/**
 * �v���O�����Ԃ̈ˑ����̖�肩��C
 * ���̃w�b�_�[����т��̃w�b�_�[����C���N���[�h�����w�b�_�[�ɂ�����
 * �\���o�[�ˑ��̃w�b�_�[(mathematica��realpaver���̌ŗL�̃w�b�_�[)��
 * �C���N���[�h���Ă͂Ȃ�Ȃ�
 *
 * ���̐����VirtualConstraintSolver���p�������N���X�̒�`�w�b�_�[�ɂ��K�p�����
 */

#include <vector>

#include "Types.h"
#include "VariableMap.h"

namespace hydla {
namespace vcs {

/**
 *  �^�E�U�E�s���E����s�\ 
 */
enum VCSResult {
  VCSR_TRUE,
  VCSR_FALSE, 
  VCSR_UNKNOWN,
  VCSR_SOLVER_ERROR,
};

template<typename VariableT, typename ValueT, typename TimeT>
class VirtualConstraintSolver
{
public:  
  typedef VariableT                                          variable_t;
  typedef ValueT                                             value_t;
  typedef TimeT                                              time_t;
  typedef hydla::simulator::VariableMap<variable_t, value_t> variable_map_t;
  typedef boost::shared_ptr<hydla::parse_tree::Ask>          ask_node_sptr;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::simulator::positive_asks_t                  positive_asks_t;
  typedef hydla::simulator::negative_asks_t                  negative_asks_t;
  typedef hydla::simulator::changed_asks_t                   changed_asks_t;

  typedef struct IntegrateResult 
  {
    typedef struct NextPhaseState 
    {
      time_t         time;
      variable_map_t variable_map;
      bool           is_max_time;
    } next_phase_state_t;
    typedef std::vector<next_phase_state_t> next_phase_state_list_t;
  
    std::vector<next_phase_state_t> states;
    changed_asks_t                  changed_asks;
  } integrate_result_t;

  VirtualConstraintSolver()
  {}

  virtual ~VirtualConstraintSolver()
  {}

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset() = 0;

  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm) = 0;  

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_variable_map(variable_map_t& vm) = 0;

  /**
   * �����ǉ�����
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells) = 0;
  
  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask) = 0;

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time) = 0;
  

};

/*
  template<typename VariableT, typename ValueT, typename TimeT>
  std::ostream& operator<<(
  std::ostream& s, 
  const VirtualConstraintSolver<VariableT, ValueT, TimeT>::integrate_result_t & t)
  {
  s << "#*** integrate result ***\n";

  next_phase_state_list_t::const_iterator ps_it = 
  BOOST_FOREACH(next_phase_state_list_t::value_type& i, t.states)
  {
  s << "---- next_phase_state ----"
  s << "- time -\n" 
  << i.time 
  << "\n"
  << "- variable_map -" 
  << i.variable_map 
  << "\n";
  }

  s << std::endl << "---- changed asks ----\n";
  BOOST_FOREACH(changed_asks_t::value_type& i, t.changed_asks)
  {
  s << "ask_type : "
  << ask_list_it->first 
  << ", "
  << "ask_id : " << ask_list_it->second
  << "\n";
  }
  return s;

  }
*/


} //namespace vcs
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
