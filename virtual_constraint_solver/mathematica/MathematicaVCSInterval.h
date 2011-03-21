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
   * @param approx_precision 近似する精度 値が負の場合は近似を行わない
   */
  MathematicaVCSInterval(MathLink* ml, int approx_precision);

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
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * 制約を追加する
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks);
  
  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks);

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list);

  /**
   * 内部状態の出力をおこなう
   */
  std::ostream& dump(std::ostream& s) const;
  
  
  /**
   * 変数表に対して与えられた時刻を適用する
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

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
  void send_init_cons(PacketSender& ps, 
                      const max_diff_map_t& max_diff_map, 
                      bool use_approx);

  /**
   * 与えられたaskのガード制約を送信する
   */
  void send_ask_guards(PacketSender& ps, 
                       const hydla::simulator::ask_set_t& asks) const;

  /**
  * 時刻を送信する
  */
  void send_time(const time_t& time);

  /**
   * 変数表に未定義の変数を追加する
   */
  void add_undefined_vars_to_vm(variable_map_t& vm);

  /**
   * 採用していないモジュール内にある制約を送信する
   */
  void send_not_adopted_tells(PacketSender& ps, const not_adopted_tells_list_t& na_tells_list) const;
  
  /**
   * 定数制約を送る
   */
  void send_parameter_cons() const;
  
  //記号定数のリストを送る
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
