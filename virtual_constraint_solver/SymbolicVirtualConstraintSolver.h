#ifndef _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
#define _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_

/**
 * �v���O�����Ԃ̈ˑ����̖�肩��C
 * ���̃w�b�_�[����т��̃w�b�_�[����C���N���[�h�����w�b�_�[�ɂ�����
 * �\���o�[�ˑ��̃w�b�_�[(mathematica��realpaver���̌ŗL�̃w�b�_�[)��
 * �C���N���[�h���Ă͂Ȃ�Ȃ�
 *
 * ���̐����SymbolicVirtualConstraintSolver���p�������N���X�̒�`�w�b�_�[�ɂ��K�p�����
 */

#include <iostream>



#include <vector>

#include <boost/function.hpp>

#include "Types.h"
#include "VariableMap.h"
#include "../symbolic_simulator/SymbolicTypes.h"

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

/**
 *  ���������V�~�����[�V�����̂��߂Ɏg���\���o�[
 *  TODO:VCS���p�����Ă��ꗂ��N���Ȃ��悤�ɂ���
 */


class SymbolicVirtualConstraintSolver
{
public:  
  typedef hydla::symbolic_simulator::variable_t              variable_t;
  typedef hydla::symbolic_simulator::value_t                 value_t;
  typedef hydla::symbolic_simulator::parameter_t             parameter_t;
  typedef hydla::symbolic_simulator::value_range_t           value_range_t;
  typedef hydla::symbolic_simulator::time_t                  time_t;
  typedef hydla::symbolic_simulator::variable_map_t          variable_map_t;
  typedef hydla::symbolic_simulator::parameter_map_t         parameter_map_t;
  typedef boost::shared_ptr<hydla::parse_tree::Ask>          ask_node_sptr;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::simulator::appended_asks_t                  appended_asks_t;
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
    typedef struct NextPhaseState 
    {
      time_t         time;
      variable_map_t variable_map;
      bool           is_max_time;
    } next_phase_state_t;
    typedef std::vector<next_phase_state_t> next_phase_state_list_t;
    
    next_phase_state_list_t states;
    changed_asks_t          changed_asks;
  } integrate_result_t;

  SymbolicVirtualConstraintSolver()
  {}

  virtual ~SymbolicVirtualConstraintSolver()
  {}

  /**
   * ���U�ω����[�h�C�A���ω����[�h�̐؂�ւ��������Ȃ�
   */
  virtual void change_mode(hydla::symbolic_simulator::Mode m, int approx_precision){}

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset() = 0;


  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm) = 0;


  /**
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm){assert(0); return false;}

  /**
   * ���݂̐���X�g�A����ϐ��\�ƒ萔�\���쐬����
   */
  virtual bool create_variable_map(variable_map_t& vm, parameter_map_t& pm){assert(0); return false;}
  
  /**
   * �����ǉ�����
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks) = 0;
  
  
  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks) = 0;

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

  //SymbolicValue���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  virtual std::string get_real_val(const value_t &val, int precision){return "get_real_val is unavailable";}
  //SymbolicTime���Ȗ񂷂�
  virtual void simplify(time_t &time){assert(0);}
  //SymbolicTime���r����
  virtual bool less_than(const time_t &lhs, const time_t &rhs){assert(0); return false;}
  //SymbolicValue�̎��Ԃ����炷
  virtual value_t shift_expr_time(const value_t& val, const time_t &time){assert(0); value_t tmp; return tmp;}
  
  
  //�ϐ��\�Ɏ�����K�p����
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){}
};


} //namespace vcs
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
