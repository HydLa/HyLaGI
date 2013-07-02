#ifndef _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
#define _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_

/**
 * プログラム間の依存性の問題から，
 * このヘッダーおよびこのヘッダーからインクルードされるヘッダーにおいて
 * ソルバー依存のヘッダー(mathematicaやrealpaver等の固有のヘッダー)を
 * インクルードしてはならない
 *
 * この制約はVirtualConstraintSolverを継承したクラスの定義ヘッダーにも適用される
 */

#include <iostream>



#include <vector>

#include <boost/function.hpp>

#include "Types.h"
#include "VariableMap.h"

namespace hydla {
namespace vcs {

/**
 *  真・偽・不明・求解不能 
 */
enum VCSResult {
  VCSR_TRUE,
  VCSR_FALSE, 
  VCSR_UNKNOWN,
  VCSR_SOLVER_ERROR,
};

template<typename VariableT, typename ValueT, typename TimeT>
class VirtualConstraintSolver
{
public:  
  typedef VariableT                                          variable_t;
  typedef ValueT                                             value_t;
  typedef TimeT                                              time_t;
  typedef hydla::simulator::VariableMap<variable_t, value_t> variable_map_t;
  typedef boost::shared_ptr<hydla::parse_tree::Ask>          ask_node_sptr;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::simulator::positive_asks_t                  positive_asks_t;
  typedef hydla::simulator::negative_asks_t                  negative_asks_t;
  typedef boost::function<void (const time_t& time, 
                                const variable_map_t& vm)>   output_function_t;
  typedef hydla::simulator::module_set_sptr                  module_set_sptr;
  typedef std::vector<module_set_sptr>                       module_set_list_t;
  typedef hydla::simulator::not_adopted_tells_list_t         not_adopted_tells_list_t;


  typedef struct IntegrateResult 
  {
    typedef struct NextPhaseResult 
    {
      time_t         time;
      variable_map_t variable_map;
      bool           is_max_time;
    } next_phase_result_t;
    typedef std::vector<next_phase_result_t> next_phase_result_list_t;
    
    next_phase_result_list_t states;
  } integrate_result_t;

  VirtualConstraintSolver()
  {}

  virtual ~VirtualConstraintSolver()
  {}

  /**
   * 制約ストアの初期化をおこなう
   */
  virtual bool reset() = 0;

  /**
   * 与えられた変数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm) = 0;  

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual bool create_variable_map(variable_map_t& vm) = 0;

  /**
   * 制約を追加する
   */
  virtual VCSResult add_constraint(const tells_t& collected_tells) = 0;
  
  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask) = 0;

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list) = 0;

  /**
   * 結果の出力関数を設定する
   */
  virtual void set_output_func(const time_t& max_interval, 
                               const output_function_t& func) 
  {
    max_interval_ = max_interval;
    output_func_  = func;
  }

  /**
   * 結果の出力関数の設定をリセットし，初期状態に戻す
   */
  virtual void reset_output_func() {
    output_func_.clear();
  }
  
  //変数表に，時刻を適用する．Symbolic専用
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){}


protected:
  void output(const time_t& time, const variable_map_t& vm) {    
    if(!output_func_.empty()) {
      output_func_(time, vm);
    }
  }

  time_t            max_interval_;
  output_function_t output_func_;
};

/*
  template<typename VariableT, typename ValueT, typename TimeT>
  std::ostream& operator<<(
  std::ostream& s, 
  const VirtualConstraintSolver<VariableT, ValueT, TimeT>::integrate_result_t & t)
  {
  s << "#*** integrate result ***\n";

  next_phase_result_list_t::const_iterator ps_it = 
  BOOST_FOREACH(next_phase_result_list_t::value_type& i, t.states)
  {
  s << "---- next_phase_result ----"
  s << "- time -\n" 
  << i.time 
  << "\n"
  << "- variable_map -" 
  << i.variable_map 
  << "\n";
  }

  s << std::endl << "---- changed asks ----\n";
  BOOST_FOREACH(changed_asks_t::value_type& i, t.changed_asks)
  {
  s << "ask_type : "
  << ask_list_it->first 
  << ", "
  << "ask_id : " << ask_list_it->second
  << "\n";
  }
  return s;

  }
*/


} //namespace vcs
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
