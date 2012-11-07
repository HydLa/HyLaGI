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
#include "../symbolic_simulator/SymbolicTypes.h"

namespace hydla {
namespace vcs {

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
  typedef std::map<variable_t*, value_range_t>               variable_range_map_t;
  typedef hydla::symbolic_simulator::parameter_map_t         parameter_map_t;
  typedef std::vector<parameter_map_t>                       parameter_maps_t;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::parse_tree::node_sptr                       node_sptr;
  typedef hydla::simulator::constraints_t                    constraints_t;

  typedef boost::shared_ptr<hydla::parse_tree::Ask>          ask_node_sptr;
  typedef hydla::simulator::positive_asks_t                  positive_asks_t;
  typedef hydla::simulator::negative_asks_t                  negative_asks_t;
  typedef hydla::simulator::changed_asks_t                   changed_asks_t;
  typedef hydla::simulator::not_adopted_tells_list_t         not_adopted_tells_list_t;
  typedef hydla::symbolic_simulator::variable_set_t                   variable_set_t;
  typedef hydla::symbolic_simulator::parameter_set_t                   parameter_set_t;
  
  typedef boost::function<void (const time_t& time, 
                                const variable_map_t& vm)>   output_function_t;
  typedef hydla::simulator::module_set_sptr                  module_set_sptr;
  typedef std::vector<module_set_sptr>                       module_set_list_t;

  typedef hydla::simulator::continuity_map_t                 continuity_map_t;


  typedef struct CheckConsistencyResult{
    parameter_maps_t true_parameter_maps, false_parameter_maps; 
  }check_consistency_result_t;

  
  /**
   * calculate_next_PP_time�ŕԂ��\����
   */
  typedef struct PPTimeResult
  {
    typedef struct NextPhaseResult 
    {
      time_t         time;
      parameter_map_t parameter_map;
      bool           is_max_time;
    } candidate_t;
    typedef std::vector<candidate_t> candidate_list_t;
    candidate_list_t candidates;
  } PP_time_result_t;
  
  /**
   * create_maps�ŕԂ��\����
   * �ϐ��\�̗�Ƃ���
   */
  typedef struct CreateResult
  {
    typedef std::vector<variable_range_map_t> result_maps_t;
    result_maps_t result_maps;
  } create_result_t;

  SymbolicVirtualConstraintSolver()
  {}

  virtual ~SymbolicVirtualConstraintSolver()
  {}

  /**
   * ���U�ω����[�h�C�A���ω����[�h�̐؂�ւ��������Ȃ�
   */
  virtual void change_mode(hydla::symbolic_simulator::Mode m, int approx_precision){}

  /**
   * �ꎞ�I�Ȑ���̒ǉ����J�n����
   */
  virtual void start_temporary(){assert(0);}

  /**
   * �ꎞ�I�Ȑ���̒ǉ����I������
   * start��C���̊֐����Ăяo���܂łɒǉ���������͂��ׂĖ����������Ƃɂ���
   */
  virtual void end_temporary(){assert(0);}

  /**
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   * �o������ϐ��ƒ萔�̏W���̏����L������
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm){assert(0); return false;}

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual create_result_t create_maps(){assert(0); return create_result_t();}
  
  /**
   * �����ǉ�����D
   */
  virtual void add_constraint(const constraints_t& constraints){assert(0);}
  virtual void add_constraint(const node_sptr& constraint){assert(0);}

  virtual void add_guard(const node_sptr&){assert(0);}
  
  virtual bool check_easy_consistency(){assert(0);}

  /**
   * ����X�g�A�����������𔻒肷��D
   * @return �[���\�ȏꍇ�̋L���萔������C�[���s�\�ȏꍇ�̋L���萔������i���ꂼ�ꑶ�݂��Ȃ��ꍇ�͋�̗��Ԃ��j
   */
  virtual check_consistency_result_t check_consistency(){assert(0); return CheckConsistencyResult();}
  
  /**
   * �ϐ��ɘA������ݒ肷��
   */
  virtual void set_continuity(const std::string &name, const int& dc){assert(0);}
  
  /**
   * ���̗��U�ω����������߂�
   * @param discrete_cause ���U�ω��̌����ƂȂ肤�����
   */
  virtual PP_time_result_t calculate_next_PP_time(
    const constraints_t& discrete_cause,
    const time_t& current_time,
    const time_t& max_time){assert(0);return PP_time_result_t();}
    

  // ���݂̐���X�g�A�𕶎���Ŏ擾����
  virtual std::string get_constraint_store(){return "this solver doesn't implement get_constraint_store";}
  
  //SymbolicTime���Ȗ񂷂�
  virtual void simplify(time_t &time){assert(0);}
  //SymbolicTime���r����
  virtual bool less_than(const time_t &lhs, const time_t &rhs){assert(0); return false;}
  //SymbolicValue�̎��Ԃ����炷
  virtual value_t shift_expr_time(const value_t& val, const time_t &time){assert(0); return value_t();}
  
  /* 
   * �ϐ��\�Ɏ�����K�p����
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){}

  /* 
   * �o������ϐ��̏W����ݒ肷��D
   * reset���Ă�����������Ȃ��D
   */
  void set_variable_set(variable_set_t& v){
    variable_set_=&v;
    original_range_map_.clear();
    for(variable_set_t::iterator it = variable_set_->begin(); it != variable_set_->end(); it++){
      original_range_map_[&(*it)] = value_range_t();
    }
  }

  /* 
   * �o������L���萔�̏W����ݒ肷��D
   * reset���Ă�����������Ȃ��D
   */
  void set_parameter_set(parameter_set_t& p){
    parameter_set_=&p;
  }

  protected:
  
  variable_t* get_variable(const std::string &name, int derivative_count) const{
    variable_t variable(name, derivative_count);
    variable_set_t::iterator it = std::find(variable_set_->begin(), variable_set_->end(), variable);
    if(it == variable_set_->end()) return NULL;
    return &(*it);
  }

  parameter_t* get_parameter(const std::string &name, int derivative_count, int id) const {
    for(parameter_set_t::iterator it = parameter_set_->begin(); it != parameter_set_->end();it++){
      if(it->get_variable()->get_name() == name && it->get_variable()->get_derivative_count() == derivative_count && it->get_phase()->id == id){
        return &(*it);
      }
    }
    assert(0);
    return NULL;
  }
  
  
  
  /**
   * create_result�̌��ʂ̗v�f�̌��^
   */
  variable_range_map_t original_range_map_;

  variable_set_t* variable_set_;
  parameter_set_t* parameter_set_;
};

std::ostream& operator<<(std::ostream& s, const SymbolicVirtualConstraintSolver::variable_range_map_t& vm);
} //namespace vcs
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
