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
  virtual bool create_variable_map(variable_map_t& vm);
  
  /**
   * �����ǉ�����
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks);
  
  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask);

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list);



  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  //SymbolicValue���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  virtual std::string get_real_val(const value_t &val, int precision);
  //SymbolicTime���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  virtual std::string get_real_val(const time_t &val, int precision);
  //element_value���w�肳�ꂽ���x�Ő��l�ɕϊ�����
  virtual std::string get_real_val(const element_value_t &val, int precision);
  //SymbolicTime���Ȗ񂷂�
  virtual void simplify(time_t &time);
  //SymbolicTime���r����
  virtual bool less_than(const time_t &lhs, const time_t &rhs);

private:
  hydla::symbolic_simulator::Mode      mode_;

  MathLink ml_;
  boost::scoped_ptr<virtual_constraint_solver_t> vcs_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
