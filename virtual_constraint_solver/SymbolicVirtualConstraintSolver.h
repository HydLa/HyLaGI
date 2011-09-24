#ifndef _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
#define _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_

/**
 * プログラム間の依存性の問題から，
 * このヘッダーおよびこのヘッダーからインクルードされるヘッダーにおいて
 * ソルバー依存のヘッダー(mathematicaやrealpaver等の固有のヘッダー)を
 * インクルードしてはならない
 *
 * この制約はSymbolicVirtualConstraintSolverを継承したクラスの定義ヘッダーにも適用される
 */

#include <iostream>
#include <vector>

#include <boost/function.hpp>

#include "Types.h"
#include "VariableMap.h"
#include "../symbolic_simulator/SymbolicTypes.h"

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

/**
 *  数式処理シミュレーションのために使うソルバー
 *  TODO:VCSを継承しても齟齬が起きないようにする
 */


class SymbolicVirtualConstraintSolver
{
public:  
  typedef hydla::symbolic_simulator::variable_t              variable_t;
  typedef hydla::symbolic_simulator::value_t                 value_t;
  typedef hydla::symbolic_simulator::parameter_t             parameter_t;
  typedef hydla::symbolic_simulator::value_range_t           value_range_t;
  typedef hydla::symbolic_simulator::time_t                  time_t;
  typedef hydla::symbolic_simulator::variable_map_t          variable_map_t;
  typedef hydla::symbolic_simulator::parameter_map_t         parameter_map_t;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::simulator::constraints_t                    constraints_t;

  typedef hydla::simulator::appended_asks_t                  appended_asks_t;
  typedef boost::shared_ptr<hydla::parse_tree::Ask>          ask_node_sptr;
  typedef hydla::simulator::positive_asks_t                  positive_asks_t;
  typedef hydla::simulator::negative_asks_t                  negative_asks_t;
  typedef hydla::simulator::changed_asks_t                   changed_asks_t;
  typedef hydla::simulator::not_adopted_tells_list_t         not_adopted_tells_list_t;
  
  typedef boost::function<void (const time_t& time, 
                                const variable_map_t& vm)>   output_function_t;
  typedef hydla::simulator::module_set_sptr                  module_set_sptr;
  typedef std::vector<module_set_sptr>                       module_set_list_t;

  typedef hydla::simulator::continuity_map_t                 continuity_map_t;

  typedef struct IntegrateResult 
  {
    typedef struct NextPhaseState 
    {
      time_t         time;
      variable_map_t variable_map;
      parameter_map_t parameter_map;
      bool           is_max_time;
    } next_phase_state_t;
    typedef std::vector<next_phase_state_t> next_phase_state_list_t;
    next_phase_state_list_t states;
  } integrate_result_t;
  
  
  typedef struct CreateResult
  {
    typedef struct Maps
    {
      variable_map_t variable_map;
      parameter_map_t parameter_map;
    } maps_t;
    typedef std::vector<maps_t> result_maps_t;
    result_maps_t result_maps;
  } create_result_t;

  SymbolicVirtualConstraintSolver()
  {}

  virtual ~SymbolicVirtualConstraintSolver()
  {}

  /**
   * 離散変化モード，連続変化モードの切り替えをおこなう
   */
  virtual void change_mode(hydla::symbolic_simulator::Mode m, int approx_precision){}

  /**
   * 制約ストアの初期化をおこなう
   */
  virtual bool reset(){assert(0); return false;}


  /**
   * 与えられた変数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm){assert(0); return false;}

  /**
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm){assert(0); return false;}

  /**
   * 現在の制約ストアから数表を作成する
   */
  virtual bool create_maps(create_result_t & create_result){assert(0); return false;}
  
  /**
   * 制約を追加する．そしてその制約に出現する変数（とその微分値）のうち，最大の微分回数をあらわすmapを返す
   */
  virtual void add_constraint(const constraints_t& constraints){assert(0); return;}
  
  
  virtual VCSResult add_constraint(const tells_t& tells, const appended_asks_t& appended_asks){assert(0); return VCSR_FALSE;}
  
  /**
   * 制約ストアが無矛盾かを判定する．
   * 引数で制約を渡された場合は一時的に制約ストアに追加する．
   */
  virtual VCSResult check_consistency(const constraints_t& constraints){assert(0); return VCSR_FALSE;}
  virtual VCSResult check_consistency(){assert(0); return VCSR_FALSE;}
  
  
  /**
   * 与えられたmapを元に，各変数の連続性を設定する．
   */
  virtual void set_continuity(const continuity_map_t& continuity_map){assert(0);}
  
  /**
   * 現在の制約ストアから与えたaskが導出可能かどうか
   */
  virtual VCSResult check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks){return VCSR_SOLVER_ERROR;}

  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const constraints_t& discrete_cause,
    const time_t& current_time,
    const time_t& max_time){assert(0);return VCSR_FALSE;}
    
    
  /**
   * askの導出状態が変化するまで積分をおこなう
   */
  virtual VCSResult integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list,
    const appended_asks_t& appended_asks)
    {assert(0); return VCSR_SOLVER_ERROR;}

  //SymbolicValueを指定された精度で数値に変換する
  virtual std::string get_real_val(const value_t &val, int precision){return "get_real_val is unavailable";}
  //SymbolicTimeを簡約する
  virtual void simplify(time_t &time){assert(0);}
  //SymbolicTimeを比較する
  virtual bool less_than(const time_t &lhs, const time_t &rhs){assert(0); return false;}
  //SymbolicValueの時間をずらす
  virtual value_t shift_expr_time(const value_t& val, const time_t &time){assert(0); value_t tmp; return tmp;}
  
  //変数表に時刻を適用する
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){}
};


} //namespace vcs
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
