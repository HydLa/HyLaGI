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
   * 制約ストアの初期化をおこなう
   */
  virtual bool reset();

  /**
   * 与えられた変数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& variable_map);

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual bool create_variable_map(variable_map_t& variable_map);

  /**
   * 制約を追加する
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells);
  
  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask);

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual bool integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * 内部状態の出力をおこなう
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
