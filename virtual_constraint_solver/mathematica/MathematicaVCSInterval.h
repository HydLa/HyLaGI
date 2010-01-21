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
    typedef std::map<MathVariable, MathValue>      init_vars_t;
    typedef hydla::simulator::tells_t              constraints_t;
    typedef std::set<MathVariable>                 cons_vars_t;

    init_vars_t   init_vars;
    constraints_t constraints;
    cons_vars_t   cons_vars;
  };
  
  typedef ConstraintStore constraint_store_t;

//   typedef std::pair<std::set<std::set<MathValue> >, 
//                     std::set<MathVariable> > constraint_store_t;


  MathematicaVCSInterval(MathLink* ml);

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

  void send_cs(PacketSender& ps) const;
  void send_cs_vars() const;

  /**
   * �ϐ��̍ő�����񐔂����Ƃ߂�
   */
  void create_max_diff_map(PacketSender& ps, max_diff_map_t& max_diff_map);

  void send_vars(PacketSender& ps, const max_diff_map_t& max_diff_map);

  /**
   * �����l�����Mathematica�ɓn��
   * 
   * Mathematica�ɑ��鐧��̒��ɏo������ϐ���
   * �ő�����񐔖����̏����l����̂ݑ��M�������Ȃ�
   */
  void send_init_cons(PacketSender& ps, const max_diff_map_t& max_diff_map);

  /**
   * �^����ꂽask�̃K�[�h����𑗐M����
   */
  void send_ask_guards(PacketSender& ps, 
                       const hydla::simulator::ask_set_t& asks) const;

  mutable MathLink* ml_;
  constraint_store_t constraint_store_;
};


std::ostream& operator<<(std::ostream& s, 
                         const MathematicaVCSInterval::constraint_store_t& c);

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_
