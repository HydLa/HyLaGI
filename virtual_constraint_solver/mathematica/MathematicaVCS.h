#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "MathVCSType.h"
#include "mathlink_helper.h"
#include "vcs_math_source.h"

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCS : 
    public virtual_constraint_solver_t
{
public:

  //MathematicaVCS(Mode m, MathLink* ml, int approx_precision);
  MathematicaVCS(const hydla::simulator::Opts &opts);

  virtual ~MathematicaVCS();

  /**
   * ���U�ω����[�h�C�A���ω����[�h�̐؂�ւ��������Ȃ�
   */
  virtual void change_mode(hydla::symbolic_simulator::Mode m, int approx_precision);

  /**
   * �ꎞ�I�Ȑ���̒ǉ����J�n����
   */
  virtual void start_temporary();

  /**
   * �ꎞ�I�Ȑ���̒ǉ����I������
   * start��C���̊֐����Ăяo���܂łɒǉ���������͂��ׂĖ����������Ƃɂ���
   */
  virtual void end_temporary();

  /**
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   * �o������ϐ��ƒ萔�̏W���̏����L������
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * ���݂̐���X�g�A����ϐ��\�ƒ萔�\���쐬����
   */
  virtual create_result_t create_maps();
  
  /**
   * �����ǉ�����D
   */
  virtual void add_constraint(const constraints_t& constraints);
  virtual void add_constraint(const node_sptr& constraint);

  virtual void add_guard(const node_sptr&);

  virtual void set_false_conditions(const node_sptr& constraint);
  
  virtual FalseConditionsResult find_false_conditions(node_sptr& node);

  /**
   * ����X�g�A�����������𔻒肷��D
   * @return �[���\�ȏꍇ�̋L���萔������C�[���s�\�ȏꍇ�̋L���萔������i���ꂼ�ꑶ�݂��Ȃ��ꍇ�͋�̗��Ԃ��j
   */
  virtual check_consistency_result_t check_consistency();


  /**
   * ���̗��U�ω����������߂�
   * @param discrete_cause ���U�ω��̌����ƂȂ肤�����
   */
  virtual PP_time_result_t calculate_next_PP_time(
    const constraints_t& discrete_cause,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * �ϐ��\�Ɏ�����K�p����
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  /**
   * �ϐ��ɘA������ݒ肷��
   */
  virtual void set_continuity(const std::string &name, const int& dc);

  /**
   * SymbolicTime���Ȗ񂷂�
   */
  virtual void simplify(time_t &time);

  /**
   * SymbolicTime���r����
   */
  virtual bool less_than(const time_t &lhs, const time_t &rhs);

  /**
   * SymbolicValue�̎��Ԃ����炷
   */
  virtual value_t shift_expr_time(const value_t &val, const time_t &time);

private:

  /**
   * �L���萔�̏����̃}�b�v���󂯎��D���O�����́Cml_���}�b�v��\�����X�g�̃g�b�v�ɗ��Ă��邱�ƁD
   * �I�����Cml_�̓}�b�v�̎��̃I�u�W�F�N�g�Ɉړ�����
   */
  void receive_parameter_map(parameter_map_t &map);

  /**
   * find_false_conditions�œ����������������
   * node_sprt�`���ŕԂ�
   * ���O������I�����̏�Ԃ�receive_parameter_map�Ɠ���
   */
  node_sptr receive_condition_node();
  
  hydla::symbolic_simulator::Mode      mode_;
  

  MathLink ml_;
  bool is_temporary_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
