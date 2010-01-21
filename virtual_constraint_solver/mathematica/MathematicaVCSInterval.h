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

  void send_cs(PacketSender& ps) const;
  void send_cs_vars() const;

  /**
   * 変数の最大微分回数をもとめる
   */
  void create_max_diff_map(PacketSender& ps, max_diff_map_t& max_diff_map);

  void send_vars(PacketSender& ps, const max_diff_map_t& max_diff_map);

  /**
   * 初期値制約をMathematicaに渡す
   * 
   * Mathematicaに送る制約の中に出現する変数の
   * 最大微分回数未満の初期値制約のみ送信をおこなう
   */
  void send_init_cons(PacketSender& ps, const max_diff_map_t& max_diff_map);

  /**
   * 与えられたaskのガード制約を送信する
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
