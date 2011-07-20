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

  /**
   * ������Ԃ̏o�͂������Ȃ�
   */
  std::ostream& dump(std::ostream& s) const;

private:
  typedef std::map<std::string, int> max_diff_map_t;

  void reset_sub(const variable_map_t& vm, std::set<MathValue>& and_cons_set,
    MathematicaExpressionConverter& mec, const bool& is_current);
  void send_cs() const;
  void send_ps() const;
  void send_store(const constraint_store_t& store) const;
  void send_cs_vars() const;
  void send_pars() const;
  void receive_constraint_store(constraint_store_t& store);
  /**
   * check_consistency �̋��ʕ���
   */
  VCSResult check_consistency_sub();

  /**
   * ���A�����Ɋւ��鐧���������
   */
  void add_left_continuity_constraint(PacketSender& ps, max_diff_map_t& max_diff_map);

  /**
   * ����X�g�A��true�ł��邩�ǂ���
   */
  bool cs_is_true()
  {
    return constraint_store_.first.size()==1 && 
      (*constraint_store_.first.begin()).size() == 1 &&
      (*(*constraint_store_.first.begin()).begin()).get_string()=="True";
  }

  mutable MathLink* ml_;
  max_diff_map_t max_diff_map_;          //���݂̐���X�g�A�ɏo�����钆�ōő�̔����񐔂��L�^���Ă����\�D
  constraint_store_t constraint_store_;
  constraint_store_t parameter_store_;
  constraints_t tmp_constraints_;  //�ꎞ�I�ɐ����ǉ�����Ώ�
  std::set<std::string> par_names_; //�ꎞ���̂�
};

std::ostream& operator<<(std::ostream& s, const MathematicaVCSPoint& m);

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_POINT_H_
