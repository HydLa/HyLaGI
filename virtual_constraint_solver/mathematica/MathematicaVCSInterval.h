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
    typedef std::map<MathVariable, value_t>      init_vars_t;
    typedef hydla::simulator::tells_t              constraints_t;
    typedef std::set<MathVariable>                 cons_vars_t;
    typedef std::map<std::string, int>             init_vars_max_diff_map_t;

    init_vars_t   init_vars;
    constraints_t constraints;
    cons_vars_t   cons_vars;
    init_vars_max_diff_map_t init_vars_max_diff_map;
  };
  
  typedef ConstraintStore constraint_store_t;

//   typedef std::pair<std::set<std::set<MathValue> >, 
//                     std::set<MathVariable> > constraint_store_t;


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
   * �����ǉ�����
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks);
  
  /**
   * ���݂̐���X�g�A����^����ask�����o�\���ǂ���
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks);

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
  void send_init_cons(PacketSender& ps, 
                      const max_diff_map_t& max_diff_map, 
                      bool use_approx);

  /**
   * �^����ꂽask�̃K�[�h����𑗐M����
   */
  void send_ask_guards(PacketSender& ps, 
                       const hydla::simulator::ask_set_t& asks) const;

  /**
  * �����𑗐M����
  */
  void send_time(const time_t& time);

  /**
   * �ϐ��\�ɖ���`�̕ϐ���ǉ�����
   */
  void add_undefined_vars_to_vm(variable_map_t& vm);

  /**
   * �̗p���Ă��Ȃ����W���[�����ɂ��鐧��𑗐M����
   */
  void send_not_adopted_tells(PacketSender& ps, const not_adopted_tells_list_t& na_tells_list) const;
  
  /**
   * �萔����𑗂�
   */
  void send_parameter_cons() const;
  
  //�L���萔�̃��X�g�𑗂�
  void send_pars() const;

  mutable MathLink* ml_;
  constraint_store_t constraint_store_;
  parameter_map_t parameter_map_;
  int approx_precision_;
};


std::ostream& operator<<(std::ostream& s, 
                         const MathematicaVCSInterval::constraint_store_t& c);

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_INTERVAL_H_
