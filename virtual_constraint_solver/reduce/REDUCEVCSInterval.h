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
   * 制約ストアが無矛盾かを判定する．
   * 引数で制約を渡された場合は一時的に制約ストアに追加する．
   */
  virtual VCSResult check_consistency();
  virtual VCSResult check_consistency(const constraints_t& constraints);

  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const node_sptr &node);

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time);  
  
  /**
   * 変数表に対して与えられた時刻を適用する
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  /**
   * 
   */
  virtual void set_continuity(const continuity_map_t& continuity_map);

private:
  // check_consistencyの受信部分
  VCSResult check_consistency_receive();

  /**
   * 初期値制約をREDUCEに渡す
   */
  void send_init_cons(REDUCEStringSender& rss, const continuity_map_t& continuity_map);

  /**
   * 制約を出現する変数や初期値制約とともに送信する
   */
  void send_constraint(const constraints_t& constraints);

  /**
  * 時刻を送信する
  */
  void send_time(const time_t& time);

  /**
   * 変数表に未定義の変数を追加する
   */
  void add_undefined_vars_to_vm(variable_map_t& vm);
  
  mutable REDUCELink* cl_;
  continuity_map_t continuity_map_;
  int approx_precision_;
};


} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_INTERVAL_H_
