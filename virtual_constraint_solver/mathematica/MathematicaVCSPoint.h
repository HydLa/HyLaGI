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
  virtual VCSResult integrate(
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
  typedef std::map<std::string, int> max_diff_map_t;

  void send_cs() const;
  void send_cs_vars() const;

  /**
   * 変数の最大微分回数をもとめる
   */
  void create_max_diff_map(PacketSender& ps, max_diff_map_t& max_diff_map);

  /**
   * 左連続性に関する制約を加える
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
