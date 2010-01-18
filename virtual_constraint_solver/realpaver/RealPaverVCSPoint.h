#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_POINT_H_

#include <ostream>

#include "mathlink_helper.h"
#include "RPVCSType.h"
#include "RPConstraintStore.h"

namespace hydla {
namespace vcs {
namespace realpaver {

class RealPaverVCSPoint : 
    public virtual_constraint_solver_t
{
public:

  RealPaverVCSPoint(MathLink* ml);

  virtual ~RealPaverVCSPoint();

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset();

  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& variable_map);

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_variable_map(variable_map_t& variable_map);

  /**
   * �����ǉ�����
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells);
  
  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask);

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual bool integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * ������Ԃ̏o�͂������Ȃ�
   */
  std::ostream& dump(std::ostream& s) const;

private:
  //void send_cs() const;
  //void send_cs_vars() const;

  ConstraintStore constraint_store_;
};

std::ostream& operator<<(std::ostream& s, const RealPaverVCSPoint& vcs)
{
  return vcs.dump(s);
}

} // namespace realapver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_POINT_H_
