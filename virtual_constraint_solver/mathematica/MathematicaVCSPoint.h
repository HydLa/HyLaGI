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
   * ���݂̐���X�g�A����ϐ��\���쐬����
   */
  virtual bool create_maps(create_result_t & create_result);
  
  /**
   * ����X�g�A�����������𔻒肷��D
   * �����Ő����n���ꂽ�ꍇ�͈ꎞ�I�ɐ���X�g�A�ɒǉ�����D
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);
  

  /**
   * �^����ꂽmap�����ɁC�e�ϐ��̘A������ݒ肷��D
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  void receive_constraint_store(constraint_store_t& store);
  virtual void add_left_continuity_constraint(const continuity_map_t& continuity_map, PacketSender &ps);
  
  /**
   * ������o������ϐ��⍶�A������ƂƂ��ɑ��M����
   */
  void send_constraint(const constraints_t& constraints);
  
  /**
   * check_consistency �̎�M����
   */
  VCSResult check_consistency_receive();

  
  mutable MathLink* ml_;
  continuity_map_t continuity_map_;
};


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
