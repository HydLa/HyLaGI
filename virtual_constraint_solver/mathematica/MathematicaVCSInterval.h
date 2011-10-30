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
  typedef hydla::simulator::constraints_t        constraints_t;
  /**
   * @param approx_precision �ߎ����鐸�x �l�����̏ꍇ�͋ߎ����s��Ȃ�
   */
  MathematicaVCSInterval(MathLink* ml, int approx_precision);

  virtual ~MathematicaVCSInterval();

  
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
   * �^����ꂽmap�����ɁC�e�ϐ��̘A������ݒ肷��D
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  // check_consistency�̋��ʕ���
  VCSResult check_consistency_sub(const constraints_t &);
  

  /**
   * �����l�����Mathematica�ɓn��
   * 
   */
  void send_init_cons(PacketSender& ps, const continuity_map_t& continuity_map);

  /*
   * �����𑗐M����
   */
  void send_time(const time_t& time);
  
  //�L���萔�̃��X�g�𑗂�
  void send_pars() const;

  mutable MathLink* ml_;
  continuity_map_t continuity_map_;
  int approx_precision_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_
