#ifndef _INCLUDED_HYDLA_SOLVER_MATHEMATICA_MATHEMATICA_SOLVER_H_
#define _INCLUDED_HYDLA_SOLVER_MATHEMATICA_MATHEMATICA_SOLVER_H_

#include <boost/scoped_ptr.hpp>

#include "mathlink_helper.h"
#include "math_source.h"

class MathLink;

namespace hydla {
namespace backend {
namespace mathematica {


class MathLink : public SymbolicLink
{
public:

  MathLink(const hydla::simulator::Opts &opts);

  virtual ~MathLink();

  /**
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   * 出現する変数と定数の集合の情報も記憶する
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);
  

  /**
   * 現在の制約ストアから変数表と定数表を作成する
   */
  virtual create_result_t create_maps();
  
  virtual void approx_vm(variable_map_t& vm);
  
  virtual bool approx_val(const value_t& val, value_range_t& range, bool force_approx);
  
  void linear_approx(const value_t& val, value_t& approxed, value_range_t& itv, int precision);
  
  virtual void reset_constraint(const variable_map_t& vm, const bool& send_derivatives);
  virtual bool reset_parameters(const parameter_map_t& pm);

  virtual void add_guard(const node_sptr&);

  virtual void set_conditions(const node_sptr& constraint);
  

  /**
   * 次の離散変化時刻を求める
   * @param discrete_cause 離散変化の原因となりうる条件
   */
  virtual PP_time_result_t calculate_next_PP_time(
    const constraints_t& discrete_cause,
    const time_t& current_time,
    const time_t& max_time);

  /**
   * 変数表に時刻を適用する
   */
  virtual void apply_time_to_vm(const variable_map_t& in_vm, variable_map_t& out_vm, const time_t& time);

  /**
   * nodeを簡約する
   */
  virtual ConditionsResult node_simplify(node_sptr &node);

  /**
   * SymbolicTimeを簡約する
   */
  virtual void simplify(time_t &time);

  /**
   * SymbolicTimeを比較する
   */
  virtual bool less_than(const time_t &lhs, const time_t &rhs);

  /**
   * SymbolicValueの時間をずらす
   */
  virtual value_t shift_expr_time(const value_t &val, const time_t &time);

  /**
   * HAConverter用：
   */
  bool check_include_bound(value_t v1, value_t v2, parameter_map_t pm1, parameter_map_t pm2);

  /**
   * lhsとrhsが同値かどうかを判定する
   */
  virtual bool equivalent(node_sptr &lhs, node_sptr &rhs);
private:

  /**
   * 記号定数の条件のマップを受け取る．事前条件は，ml_がマップを表すリストのトップに来ていること．
   * 終了時，ml_はマップの次のオブジェクトに移動する
   */
  void receive_parameter_map(parameter_map_t &map);

  void send_parameter_map(const parameter_map_t &parameter_map, PacketSender& ps);


};

} // namespace mathematica
} // namespace solver
} // namespace hydla 

#endif // include guard
