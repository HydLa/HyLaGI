#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_

#include <map>
#include <set>
#include <vector>

#include "mathlink_helper.h"

#include "MathVCSType.h"
#include "PacketSender.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCSInterval : 
    public virtual_constraint_solver_t
{
public:
  struct ConstraintStore 
  {
  
    typedef std::map<MathVariable, value_t>        init_vars_t;
    typedef hydla::simulator::constraints_t        constraints_t;

    init_vars_t   init_vars;
    constraints_t constraints;
  };
  
  typedef ConstraintStore constraint_store_t;

  /**
   * @param approx_precision �ߎ����鐸�x �l�����̏ꍇ�͋ߎ����s��Ȃ�
   */
  MathematicaVCSInterval(MathLink* ml, int approx_precision);

  virtual ~MathematicaVCSInterval();

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset();

  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& variable_map);
  
  /**
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  
  /**
   * �����ǉ�����D���łɐ���X�g�A�����������𔻒肷��D
   */
  virtual void add_constraint(const constraints_t& constraints);
  
  /**
   * ����X�g�A�����������𔻒肷��D
   * �����Ő����n���ꂽ�ꍇ�͈ꎞ�I�ɐ���X�g�A�ɒǉ�����D
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);


  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * ������Ԃ̏o�͂������Ȃ�
   */
  std::ostream& dump(std::ostream& s) const;
  
  /**
   * �ϐ��\�ɑ΂��ė^����ꂽ������K�p����
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

private:
  typedef std::map<std::string, int> max_diff_map_t;

  void send_cs(PacketSender& ps) const;

  void send_vars(PacketSender& ps, const max_diff_map_t& max_diff_map);
  
  // check_consistency�̋��ʕ���
  VCSResult check_consistency_sub();
  

  /**
   * �����l�����Mathematica�ɓn��
   * 
   * Mathematica�ɑ��鐧��̒��ɏo������ϐ���
   * �ő�����񐔖����̏����l����̂ݑ��M�������Ȃ�
   */
  void send_init_cons(PacketSender& ps, 
                      const max_diff_map_t& max_diff_map, 
                      bool use_approx);
  /**
  * �����𑗐M����
  */
  void send_time(const time_t& time);

  /**
   * �萔����𑗂�
   */
  void send_parameter_cons() const;
  
  //�L���萔�̃��X�g�𑗂�
  void send_pars() const;

  mutable MathLink* ml_;
  max_diff_map_t       max_diff_map_;
  constraint_store_t constraint_store_;
  constraints_t tmp_constraints_;  //�ꎞ�I�ɐ����ǉ�����Ώ�
  parameter_map_t parameter_map_;
  MathValue added_condition_;  //check_consistency�Œǉ���������
  int approx_precision_;
};


std::ostream& operator<<(std::ostream& s, 
                         const MathematicaVCSInterval::constraint_store_t& c);

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_
