#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_

#include "mathlink_helper.h"

#include "MathVCSType.h"
#include "MathConstraintStorePoint.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCSPoint : 
    public virtual_constraint_solver_t
{
public:
  MathematicaVCSPoint(MathLink* ml);

  virtual ~MathematicaVCSPoint();

  /**
   * ����X�g�A�̏������������Ȃ�
   */
  virtual bool reset();

  /**
   * �^����ꂽ�ϐ��\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm);

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_variable_map(variable_map_t& vm);

  /**
   * �����ǉ�����
   */
  virtual Trivalent add_constraint(const tells_t& collected_tells);
  
  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual Trivalent check_entailment(const ask_node_sptr& negative_ask);

  /**
   * ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
   */
  virtual bool integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time);

private:
  MathLink* ml_;
  MathConstraintStorePoint cons_store_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
