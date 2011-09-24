#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_INTERVAL_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_INTERVAL_H_

#include <map>
#include <set>
#include <vector>

#include "REDUCELink.h"
#include "REDUCEVCSType.h"
#include "REDUCEStringSender.h"

#include "../../parser/SExpParser.h"

using namespace hydla::parser;

namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCSInterval : 
    public virtual_constraint_solver_t
{
public:
  typedef SExpParser::const_tree_iter_t const_tree_iter_t;

  struct ConstraintStore 
  {
    typedef std::map<REDUCEVariable, value_t>      init_vars_t;
    typedef hydla::simulator::constraints_t        constraints_t;

    init_vars_t   init_vars;
    constraints_t constraints;
  };
  
  typedef ConstraintStore constraint_store_t;


  /**
   * @param approx_precision 近似する精度 値が負の場合は近似を行わない
   */
  REDUCEVCSInterval(REDUCELink* cl, int approx_precision);

  virtual ~REDUCEVCSInterval();

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
   * 制約を追加する．
   */
  virtual void add_constraint(const constraints_t& constraints);
  
  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks);

  /**
   * 制約ストアが無矛盾かを判定する．
   * 引数で制約を渡された場合は一時的に制約ストアに追加する．
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * 内部状態の出力をおこなう
   */
  std::ostream& dump(std::ostream& s) const;
  
  
  /**
   * 変数表に対して与えられた時刻を適用する
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

private:
  typedef REDUCEStringSender::max_diff_map_t max_diff_map_t;

  void send_cs(REDUCEStringSender& rss) const;

  void send_vars(REDUCEStringSender& rss, const max_diff_map_t& max_diff_map);

  // check_consistencyの共通部分
  VCSResult check_consistency_sub();

  /**
   * 初期値制約をMathematicaに渡す
   * 
   * Mathematicaに送る制約の中に出現する変数の
   * 最大微分回数未満の初期値制約のみ送信をおこなう
   */
  void send_init_cons(REDUCEStringSender& rss, 
                      const max_diff_map_t& max_diff_map, 
                      bool use_approx);

  /**
   * 与えられたaskのガード制約を送信する
   */
  void send_ask_guards(REDUCEStringSender& rss, 
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
  void send_not_adopted_tells(REDUCEStringSender& rss, const not_adopted_tells_list_t& na_tells_list) const;
  
  /**
   * 定数制約を送る
   */
  void send_parameter_cons() const;
  
  //記号定数のリストを送る
  void send_pars() const;

  mutable REDUCELink* cl_;
  max_diff_map_t max_diff_map_;
  constraint_store_t constraint_store_;
  constraints_t tmp_constraints_;  //一時的に制約を追加する対象
  parameter_map_t parameter_map_;
  REDUCEValue added_condition_;  //check_consistencyで追加される条件
  int approx_precision_;
};


std::ostream& operator<<(std::ostream& s, 
                         const REDUCEVCSInterval::constraint_store_t& c);

} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_INTERVAL_H_
