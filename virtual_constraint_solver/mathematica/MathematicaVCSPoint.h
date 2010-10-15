#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_

#include <ostream>

#include "mathlink_helper.h"
#include "MathVCSType.h"
#include "PacketSender.h"

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCSPoint : 
    public virtual_constraint_solver_t
{
public:
  typedef std::set<PacketSender::var_info_t> constraint_store_vars_t;
  
  typedef std::pair<std::set<std::set<MathValue> >, 
                    constraint_store_vars_t> constraint_store_t;

  MathematicaVCSPoint(MathLink* ml);

  virtual ~MathematicaVCSPoint();

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
  virtual VCSResult integrate(
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
  typedef std::map<std::string, int> max_diff_map_t;

  void send_cs() const;
  void send_cs_vars() const;

  /**
   * �ϐ��̍ő�����񐔂����Ƃ߂�
   */
  void create_max_diff_map(PacketSender& ps, max_diff_map_t& max_diff_map);

  /**
   * ���A�����Ɋւ��鐧���������
   */
  void add_left_continuity_constraint(PacketSender& ps, max_diff_map_t& max_diff_map);

  mutable MathLink* ml_;
  constraint_store_t constraint_store_;
};

std::ostream& operator<<(std::ostream& s, const MathematicaVCSPoint& m);

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
