#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "REDUCEVCSType.h"
#include "REDUCELink.h"
#include "vcs_reduce_source.h"



namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCS : 
    public virtual_constraint_solver_t
{
public:


  //REDUCEVCS(Mode m, REDUCELink* cl, int approx_precision);
  REDUCEVCS(const hydla::symbolic_simulator::Opts &opts, variable_map_t &vm);

  virtual ~REDUCEVCS();
  
  //�\���o�̃��[�h��ύX����
  virtual void change_mode(hydla::symbolic_simulator::Mode m, int approx_precision);

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset();

  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm);
 
  /**
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);
  

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_maps(create_result_t &create_result);
  
  /**
   * �����ǉ�����
   */
  virtual void add_constraint(const constraints_t& constraints);

  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks);

  /**
   * ����X�g�A�����������𔻒肷��D
   * �����Ő����n���ꂽ�ꍇ�͈ꎞ�I�ɐ���X�g�A�ɒǉ�����D
   */
  virtual VCSResult check_consistency(const constraints_t& constraints);
  virtual VCSResult check_consistency();

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time);



  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  //SymbolicValue���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  virtual std::string get_real_val(const value_t &val, int precision);
  //SymbolicTime���Ȗ񂷂�
  virtual void simplify(time_t &time);
  //SymbolicTime���r����
  virtual bool less_than(const time_t &lhs, const time_t &rhs);
  //SymbolicValue�̎��Ԃ����炷
  virtual value_t shift_expr_time(const value_t &val, const time_t &time);

private:
  hydla::symbolic_simulator::Mode      mode_;

  REDUCELink cl_;
  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_
