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
#include "../symbolic_simulator/SymbolicTypes.h"

namespace hydla {
namespace vcs {

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
  typedef std::map<variable_t*, value_range_t>               variable_range_map_t;
  typedef hydla::symbolic_simulator::parameter_map_t         parameter_map_t;
  typedef std::vector<parameter_map_t>                       parameter_maps_t;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::parse_tree::node_sptr                       node_sptr;
  typedef hydla::simulator::constraints_t                    constraints_t;

  typedef boost::shared_ptr<hydla::parse_tree::Ask>          ask_node_sptr;
  typedef hydla::simulator::positive_asks_t                  positive_asks_t;
  typedef hydla::simulator::negative_asks_t                  negative_asks_t;
  typedef hydla::simulator::changed_asks_t                   changed_asks_t;
  typedef hydla::simulator::not_adopted_tells_list_t         not_adopted_tells_list_t;
  typedef hydla::symbolic_simulator::variable_set_t                   variable_set_t;
  typedef hydla::symbolic_simulator::parameter_set_t                   parameter_set_t;
  
  typedef boost::function<void (const time_t& time, 
                                const variable_map_t& vm)>   output_function_t;
  typedef hydla::simulator::module_set_sptr                  module_set_sptr;
  typedef std::vector<module_set_sptr>                       module_set_list_t;

  typedef hydla::simulator::continuity_map_t                 continuity_map_t;


  typedef struct CheckConsistencyResult{
    parameter_maps_t true_parameter_maps, false_parameter_maps; 
  }check_consistency_result_t;

  
  /**
   * calculate_next_PP_timeで返す構造体
   */
  typedef struct PPTimeResult
  {
    typedef struct NextPhaseResult 
    {
      time_t         time;
      parameter_map_t parameter_map;
      bool           is_max_time;
    } candidate_t;
    typedef std::vector<candidate_t> candidate_list_t;
    candidate_list_t candidates;
  } PP_time_result_t;
  
  /**
   * create_mapsで返す構造体
   * 変数表の列とする
   */
  typedef struct CreateResult
  {
    typedef std::vector<variable_range_map_t> result_maps_t;
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
   * 一時的な制約の追加を開始する
   */
  virtual void start_temporary(){assert(0);}

  /**
   * 一時的な制約の追加を終了する
   * start後，この関数を呼び出すまでに追加した制約はすべて無かったことにする
   */
  virtual void end_temporary(){assert(0);}

  /**
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   * 出現する変数と定数の集合の情報も記憶する
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm){assert(0); return false;}

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual create_result_t create_maps(){assert(0); return create_result_t();}
  
  /**
   * 制約を追加する．
   */
  virtual void add_constraint(const constraints_t& constraints){assert(0);}
  virtual void add_constraint(const node_sptr& constraint){assert(0);}

  virtual void add_guard(const node_sptr&){assert(0);}
  
  virtual bool check_easy_consistency(){assert(0);}

  /**
   * 制約ストアが無矛盾かを判定する．
   * @return 充足可能な場合の記号定数条件列，充足不可能な場合の記号定数条件列（それぞれ存在しない場合は空の列を返す）
   */
  virtual check_consistency_result_t check_consistency(){assert(0); return CheckConsistencyResult();}
  
  /**
   * 変数に連続性を設定する
   */
  virtual void set_continuity(const std::string &name, const int& dc){assert(0);}
  
  /**
   * 次の離散変化時刻を求める
   * @param discrete_cause 離散変化の原因となりうる条件
   */
  virtual PP_time_result_t calculate_next_PP_time(
    const constraints_t& discrete_cause,
    const time_t& current_time,
    const time_t& max_time){assert(0);return PP_time_result_t();}
    

  // 現在の制約ストアを文字列で取得する
  virtual std::string get_constraint_store(){return "this solver doesn't implement get_constraint_store";}
  
  //SymbolicTimeを簡約する
  virtual void simplify(time_t &time){assert(0);}
  //SymbolicTimeを比較する
  virtual bool less_than(const time_t &lhs, const time_t &rhs){assert(0); return false;}
  //SymbolicValueの時間をずらす
  virtual value_t shift_expr_time(const value_t& val, const time_t &time){assert(0); return value_t();}
  
  /* 
   * 変数表に時刻を適用する
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time){}

  /* 
   * 出現する変数の集合を設定する．
   * resetしても初期化されない．
   */
  void set_variable_set(variable_set_t& v){
    variable_set_=&v;
    original_range_map_.clear();
    for(variable_set_t::iterator it = variable_set_->begin(); it != variable_set_->end(); it++){
      original_range_map_[&(*it)] = value_range_t();
    }
  }

  /* 
   * 出現する記号定数の集合を設定する．
   * resetしても初期化されない．
   */
  void set_parameter_set(parameter_set_t& p){
    parameter_set_=&p;
  }

  protected:
  
  variable_t* get_variable(const std::string &name, int derivative_count) const{
    variable_t variable(name, derivative_count);
    variable_set_t::iterator it = std::find(variable_set_->begin(), variable_set_->end(), variable);
    if(it == variable_set_->end()) return NULL;
    return &(*it);
  }

  parameter_t* get_parameter(const std::string &name, int derivative_count, int id) const {
    for(parameter_set_t::iterator it = parameter_set_->begin(); it != parameter_set_->end();it++){
      if(it->get_variable()->get_name() == name && it->get_variable()->get_derivative_count() == derivative_count && it->get_phase()->id == id){
        return &(*it);
      }
    }
    assert(0);
    return NULL;
  }
  
  
  
  /**
   * create_resultの結果の要素の原型
   */
  variable_range_map_t original_range_map_;

  variable_set_t* variable_set_;
  parameter_set_t* parameter_set_;
};

std::ostream& operator<<(std::ostream& s, const SymbolicVirtualConstraintSolver::variable_range_map_t& vm);
} //namespace vcs
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_VIRTUAL_CONSTRAINT_SOLVER_H_
