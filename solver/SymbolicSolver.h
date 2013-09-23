#ifndef _INCLUDED_HYDLA_SOLVER_SOLVER_H_
#define _INCLUDED_HYDLA_SOLVER_SOLVER_H_

/**
 * プログラム間の依存性の問題から，
 * このヘッダーおよびこのヘッダーからインクルードされるヘッダーにおいて
 * ソルバー依存のヘッダー(mathematicaやrealpaver等の固有のヘッダー)を
 * インクルードしてはならない
 *
 * この制約はSymbolicSolverを継承したクラスの定義ヘッダーにも適用される
 */

#include <iostream>
#include <vector>

#include <boost/function.hpp>

#include "PhaseSimulator.h"
#include "../symbolic_simulator/SymbolicTypes.h"

namespace hydla {
namespace solver {

/**
 *  数式処理シミュレーションのために使うソルバー
 *  TODO:Solverを継承しても齟齬が起きないようにする
 */

typedef hydla::simulator::symbolic::parameter_map_t         parameter_map_t;
typedef std::vector<parameter_map_t>                       parameter_maps_t;
struct CheckConsistencyResult{
  parameter_maps_t true_parameter_maps, false_parameter_maps; 
};

class SymbolicSolver
{
public:
  typedef hydla::simulator::symbolic::variable_t              variable_t;
  typedef hydla::simulator::symbolic::value_t                 value_t;
  typedef hydla::simulator::symbolic::parameter_t             parameter_t;
  typedef hydla::simulator::symbolic::value_range_t           value_range_t;
  typedef hydla::simulator::symbolic::time_t                  time_t;
  typedef hydla::simulator::symbolic::variable_map_t          variable_map_t;
  typedef hydla::simulator::tells_t                          tells_t;
  typedef hydla::parse_tree::node_sptr                       node_sptr;
  typedef hydla::simulator::constraints_t                    constraints_t;

  typedef boost::shared_ptr<hydla::parse_tree::Ask>          ask_node_sptr;
  typedef hydla::simulator::positive_asks_t                  positive_asks_t;
  typedef hydla::simulator::negative_asks_t                  negative_asks_t;
  typedef hydla::simulator::not_adopted_tells_list_t         not_adopted_tells_list_t;
  typedef hydla::simulator::symbolic::variable_set_t          variable_set_t;
  typedef hydla::simulator::symbolic::parameter_set_t         parameter_set_t;
  
  typedef boost::function<void (const time_t& time, 
                                const variable_map_t& vm)>   output_function_t;
  typedef hydla::simulator::module_set_sptr                  module_set_sptr;
  typedef std::vector<module_set_sptr>                       module_set_list_t;

  typedef hydla::simulator::continuity_map_t                 continuity_map_t;

  typedef enum{
    CONDITIONS_TRUE,
    CONDITIONS_FALSE,
    CONDITIONS_VARIABLE_CONDITIONS
  } ConditionsResult;
  
  
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
    typedef std::vector<variable_map_t> result_maps_t;
    result_maps_t result_maps;
  } create_result_t;

  SymbolicSolver()
  {}

  virtual ~SymbolicSolver()
  {}

  /**
   * 離散変化モード，連続変化モードの切り替えをおこなう
   */
  virtual void change_mode(hydla::simulator::symbolic::Mode m, int approx_precision){}

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
  
  /**
   * 変数表を用いて制約ストアを上書きする．
   */
  virtual void reset_constraint(const variable_map_t& vm, const bool& send_derivatives){assert(0);}
  
  /**
   * reset the condition of parameters based on the given parameter map
   */
  virtual bool reset_parameters(const parameter_map_t& pm){assert(0); return false;}

  virtual void add_guard(const node_sptr&){assert(0);}

  /**
   * 制約モジュール集合が矛盾する条件をセットする
   */
  virtual void set_conditions(const node_sptr& constraint){assert(0);}

  /**
   * 矛盾する条件をあらかじめ調べる関数
   * 引数に得た矛盾する条件を入れる
   * @return
   * CONDITIONS_TRUE                : 必ず矛盾              条件 : 何も入れない
   * CONDITIONS_FALSE               : 矛盾しない            条件 : 何も入れない
   * CONDITIONS_VARIABLE_CONDITIONS : 条件によって矛盾する  条件 : 条件を入れる
   */
  virtual ConditionsResult find_conditions(node_sptr& node){assert(0); return CONDITIONS_FALSE;}

  /**
   * 制約ストアが無矛盾かを判定する．
   * @return 充足可能な場合の記号定数条件列，充足不可能な場合の記号定数条件列（それぞれ存在しない場合は空の列を返す）
   */
  virtual CheckConsistencyResult check_consistency(){assert(0); return CheckConsistencyResult();}
  
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
  
  //nodeを簡約する
  virtual ConditionsResult node_simplify(node_sptr &node){assert(0);return CONDITIONS_FALSE;}
  // 同値判定を行う
  virtual bool equivalent(node_sptr &lhs, node_sptr &rhs){assert(0);return false;}


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

  virtual void approx_vm(variable_map_t& vm){assert(0);}
  
  virtual void linear_approx(const value_t& val, value_t& approxed, value_range_t& range, int precision){assert(0);}

  virtual bool approx_val(const value_t& val, value_range_t& range, bool force_approx = false){assert(0); return false;}

  /* 
   * 出現する変数の集合を設定する．
   * resetしても初期化されない．
   */
  void set_variable_set(variable_set_t& v){
    variable_set_=&v;
  }
  
  /* 
   * 出現する記号定数の集合を設定する．
   * resetしても初期化されない．
   */
  void set_parameter_set(parameter_set_t& p){
    parameter_set_ = &p;
  }

  /*
   * HAConverter用：
   */
  virtual bool check_include_bound(value_t v1, value_t v2, parameter_map_t pm1, parameter_map_t pm2){assert(0);}

  protected:
  
  variable_t* get_variable(const std::string &name, int derivative_count) const{
    variable_t variable(name, derivative_count);
    variable_set_t::iterator it = std::find(variable_set_->begin(), variable_set_->end(), variable);
    if(it == variable_set_->end()) return NULL;
    return &(*it);
  }

  parameter_t* get_parameter(const std::string &name, int derivative_count, int id) const {
    for(parameter_set_t::iterator it = parameter_set_->begin(); it != parameter_set_->end();it++){
      parameter_t& param = *it;
      if(param.get_variable()->get_name() == name && param.get_variable()->get_derivative_count() == derivative_count && param.get_phase()->id == id){
        return &param;
      }
    }
    assert(0);
    return NULL;
  }
  
  variable_set_t* variable_set_;

  parameter_set_t* parameter_set_;
};

} //namespace solver
} //namespace hydla 

#endif // include guard
