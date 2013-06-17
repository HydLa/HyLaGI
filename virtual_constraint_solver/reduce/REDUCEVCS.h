#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "../../simulator/ValueVisitor.h"
#include "../../symbolic_simulator/SymbolicValue.h"
#include "REDUCEVCSType.h"
#include "REDUCELink.h"
#include "vcs_reduce_source.h"



namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCS : 
    public virtual_constraint_solver_t, hydla::simulator::ValueVisitor
{
public:

  typedef hydla::simulator::symbolic::SymbolicValue symbolic_value_t;

  //REDUCEVCS(Mode m, REDUCELink* cl, int approx_precision);
  
  /**
   * @param m ����t�Ɋւ���]���ϐ��̒�`�Ɏg�p����
   */
  REDUCEVCS(const hydla::simulator::Opts &opts, variable_range_map_t &m);

  virtual ~REDUCEVCS();
  
  /**
   * �\���o�̃��[�h��ύX����
   */
  virtual void change_mode(hydla::simulator::symbolic::Mode m, int approx_precision);
 
  /**
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * �ϐ��ɘA������ݒ肷��
   */
  virtual void set_continuity(const std::string &name, const int& dc);

  /**
   * �����ǉ�����D
   */
  virtual void add_constraint(const constraints_t& constraints);

  /**
   * �����ǉ�����D
   */
  virtual void add_constraint(const node_sptr& constraint);

  /**
   * ����X�g�A�����������𔻒肷��D
   * @return �[���\�ȏꍇ�̋L���萔������C�[���s�\�ȏꍇ�̋L���萔������i���ꂼ�ꑶ�݂��Ȃ��ꍇ�͋�̗��Ԃ��j
   */
  virtual CheckConsistencyResult check_consistency();

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual create_result_t create_maps();

  /**
   * �ϐ��\��p���Đ���X�g�A���㏑������D
   */
  virtual void reset_constraint(const variable_map_t& vm, const bool& send_derivatives);

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
   * �K�[�h�����ǉ�����
   */
  virtual void add_guard(const node_sptr&);

  /**
   * ���݂̐���X�g�A�𕶎���Ŏ擾����
   */
  virtual std::string get_constraint_store();

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
   * TODO: ���̂Ƃ��뉽�����Ȃ��悤�Ȃ̂Ń_�~�[����
   */
  virtual void approx_vm(variable_range_map_t& vm);

  // deleted
  //  //SymbolicValue���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  //  virtual std::string get_real_val(const value_t &val, int precision, hydla::symbolic_simulator::OutputFormat opfmt);

private:
  /**
   * Variable�̐���
   */
  const node_sptr make_variable(const std::string &name, const int& dc, const bool& is_prev = false) const;

  /**
   * accept���o�R����value_t��symbolic_value_t�Ƀ_�E���L���X�g����
   */
  const symbolic_value_t get_symbolic_value_t(value_t value);

  /**
   * ValueVisitor�̎���
   */
  virtual void visit(symbolic_value_t& value);


  hydla::simulator::symbolic::Mode mode_;

  REDUCELink cl_;

  /**
   * ValueVisior::visit()���瓾���l���ꎞ�i�[����
   */
  symbolic_value_t visited_;

};

} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_
