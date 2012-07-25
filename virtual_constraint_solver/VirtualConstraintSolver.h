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

#include <iostream>



#include <vector>

#include <boost/function.hpp>

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
  typedef boost::function<void (const time_t& time, 
                                const variable_map_t& vm)>   output_function_t;
  typedef hydla::simulator::module_set_sptr                  module_set_sptr;
  typedef std::vector<module_set_sptr>                       module_set_list_t;
  typedef hydla::simulator::not_adopted_tells_list_t         not_adopted_tells_list_t;


  typedef struct IntegrateResult 
  {
    typedef struct NextPhaseResult 
    {
      time_t         time;
      variable_map_t variable_map;
      bool           is_max_time;
    } next_phase_result_t;
    typedef std::vector<next_phase_result_t> next_phase_result_list_t;
    
    next_phase_result_list_t states;
    changed_asks_t          changed_asks;
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
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list) = 0;

  /**
   * ���ʂ̏o�͊֐���ݒ肷��
   */
  virtual void set_output_func(const time_t& max_interval, 
                               const output_function_t& func) 
  {
    max_interval_ = max_interval;
    output_func_  = func;
  }

  /**
   * ���ʂ̏o�͊֐��̐ݒ�����Z�b�g���C������Ԃɖ߂�
   */
  virtual void reset_output_func() {
    output_func_.clear();
  }
  
  //�ϐ��\�ɁC������K�p����DSymbolic��p
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){}


protected:
  void output(const time_t& time, const variable_map_t& vm) {    
    if(!output_func_.empty()) {
      output_func_(time, vm);
    }
  }

  time_t            max_interval_;
  output_function_t output_func_;
};

/*
  template<typename VariableT, typename ValueT, typename TimeT>
  std::ostream& operator<<(
  std::ostream& s, 
  const VirtualConstraintSolver<VariableT, ValueT, TimeT>::integrate_result_t & t)
  {
  s << "#*** integrate result ***\n";

  next_phase_result_list_t::const_iterator ps_it = 
  BOOST_FOREACH(next_phase_result_list_t::value_type& i, t.states)
  {
  s << "---- next_phase_result ----"
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
