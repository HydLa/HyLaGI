#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "MathVCSType.h"
#include "mathlink_helper.h"
#include "vcs_math_source.h"

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

class MathematicaVCS : 
    public virtual_constraint_solver_t
{
public:

  //MathematicaVCS(Mode m, MathLink* ml, int approx_precision);
  MathematicaVCS(const hydla::simulator::Opts &opts);

  virtual ~MathematicaVCS();

  /**
   * 離散変化モード，連続変化モードの切り替えをおこなう
   */
  virtual void change_mode(hydla::symbolic_simulator::Mode m, int approx_precision);

  /**
   * 一時的な制約の追加を開始する
   */
  virtual void start_temporary();

  /**
   * 一時的な制約の追加を終了する
   * start後，この関数を呼び出すまでに追加した制約はすべて無かったことにする
   */
  virtual void end_temporary();

  /**
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   * 出現する変数と定数の集合の情報も記憶する
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * 現在の制約ストアから変数表と定数表を作成する
   */
  virtual create_result_t create_maps();
  
  /**
   * 制約を追加する．
   */
  virtual void add_constraint(const constraints_t& constraints);
  virtual void add_constraint(const node_sptr& constraint);

  virtual void add_guard(const node_sptr&);

  virtual void set_false_conditions(const node_sptr& constraint);
  
  virtual FalseConditionsResult find_false_conditions(node_sptr& node);

  /**
   * 制約ストアが無矛盾かを判定する．
   * @return 充足可能な場合の記号定数条件列，充足不可能な場合の記号定数条件列（それぞれ存在しない場合は空の列を返す）
   */
  virtual check_consistency_result_t check_consistency();


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
   * 変数に連続性を設定する
   */
  virtual void set_continuity(const std::string &name, const int& dc);

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

private:

  /**
   * 記号定数の条件のマップを受け取る．事前条件は，ml_がマップを表すリストのトップに来ていること．
   * 終了時，ml_はマップの次のオブジェクトに移動する
   */
  void receive_parameter_map(parameter_map_t &map);

  /**
   * find_false_conditionsで得た矛盾する条件を
   * node_sprt形式で返す
   * 事前条件や終了時の状態はreceive_parameter_mapと同じ
   */
  node_sptr receive_condition_node();
  
  hydla::symbolic_simulator::Mode      mode_;
  

  MathLink ml_;
  bool is_temporary_;
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_VCS_H_
