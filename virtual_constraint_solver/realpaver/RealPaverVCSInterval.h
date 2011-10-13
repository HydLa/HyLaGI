#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_INTERVAL_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_INTERVAL_H_

#include <ostream>

#include "mathlink_helper.h"

#include "RealPaverBaseVCS.h"
#include "RPConstraintStoreInterval.h"
#include "RPConstraintBuilder.h"


namespace hydla {
namespace vcs {
namespace realpaver {

typedef std::set<rp_constraint> ctr_set_t;
typedef boost::shared_ptr<hydla::parse_tree::Ask> ask_sptr;

class RealPaverVCSInterval : 
  public RealPaverBaseVCS
{
public:

  RealPaverVCSInterval(MathLink* ml);

  virtual ~RealPaverVCSInterval();

  virtual RealPaverBaseVCS* clone();

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
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list);

  /**
   * ������Ԃ̏o�͂������Ȃ�
   */
  std::ostream& dump(std::ostream& s) const;

  virtual void add_single_constraint(const node_sptr& constraint_node,
    const bool neg_expression);

  virtual void set_precision(const double p);

private:
  void find_next_point_phase_states(std::vector<rp_box>& results,
    const ask_sptr ask, var_name_map_t vars, 
    const time_t& current_time,
    const time_t& max_time, hydla::simulator::AskState state);

  void output_trajectory(const ask_sptr ask, var_name_map_t vars,
    const time_t& current_time, const double next_time,
    hydla::simulator::AskState state);

  static void clear_ctr_set(ctr_set_t& ctrs);

  ConstraintStoreInterval constraint_store_;
  double prec_;
  MathLink* ml_;
};

std::ostream& operator<<(std::ostream& s, const RealPaverVCSInterval& vcs);

} // namespace realapver
} // namespace vcs
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_REALPAVER_VCS_INTERVAL_H_
