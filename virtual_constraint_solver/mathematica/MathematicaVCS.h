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
  MathematicaVCS(const hydla::symbolic_simulator::Opts &opts);

  virtual ~MathematicaVCS();
  
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
   * �����ǉ�����D
   */
  virtual void add_constraint(const constraints_t& constraints);
  
  
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
    const constraints_t& constraints,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * �^����ꂽmap�����ɁC�e�ϐ��̘A������ݒ肷��D
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);


  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  //SymbolicValue���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  virtual std::string get_real_val(const value_t &val, int precision, hydla::symbolic_simulator::OutputFormat opfmt);
  //SymbolicTime���Ȗ񂷂�
  virtual void simplify(time_t &time);
  //SymbolicTime���r����
  virtual bool less_than(const time_t &lhs, const time_t &rhs);
  //SymbolicValue�̎��Ԃ����炷
  virtual value_t shift_expr_time(const value_t &val, const time_t &time);

private:
  hydla::symbolic_simulator::Mode      mode_;

  MathLink ml_;
  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
