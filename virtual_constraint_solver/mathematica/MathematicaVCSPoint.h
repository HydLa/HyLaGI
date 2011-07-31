#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_

#include <ostream>

#include "mathlink_helper.h"
#include "MathVCSType.h"
#include "PacketSender.h"
#include "MathematicaExpressionConverter.h"

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
   * �^����ꂽ�ϐ��\�ƒ萔�\�����ɁC����X�g�A�̏������������Ȃ�
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_maps(create_result_t & create_result);

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


private:
  typedef std::map<std::string, int> max_diff_map_t;
  void receive_constraint_store(constraint_store_t& store);
  
  /**
   * ������o������ϐ��⍶�A������ƂƂ��ɑ��M����
   */
  void send_constraint(const constraints_t& constraints);
  
  /**
   * check_consistency �̎�M����
   */
  VCSResult check_consistency_receive();

  /**
   * ���A�����Ɋւ��鐧���������
   */
  void add_left_continuity_constraint(PacketSender& ps, max_diff_map_t& max_diff_map);

  mutable MathLink* ml_;
};


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
