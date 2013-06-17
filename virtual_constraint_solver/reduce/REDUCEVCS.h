#ifndef _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_


#include <boost/scoped_ptr.hpp>

#include "../../simulator/ValueVisitor.h"
#include "../../symbolic_simulator/SymbolicValue.h"
#include "REDUCEVCSType.h"
#include "REDUCELink.h"
#include "vcs_reduce_source.h"



namespace hydla {
namespace vcs {
namespace reduce {

class REDUCEVCS : 
    public virtual_constraint_solver_t, hydla::simulator::ValueVisitor
{
public:

  typedef hydla::simulator::symbolic::SymbolicValue symbolic_value_t;

  //REDUCEVCS(Mode m, REDUCELink* cl, int approx_precision);
  
  /**
   * @param m 時刻tに関する従属変数の定義に使用する
   */
  REDUCEVCS(const hydla::simulator::Opts &opts, variable_range_map_t &m);

  virtual ~REDUCEVCS();
  
  /**
   * ソルバのモードを変更する
   */
  virtual void change_mode(hydla::simulator::symbolic::Mode m, int approx_precision);
 
  /**
   * 与えられた変数表と定数表を元に，制約ストアの初期化をおこなう
   */
  virtual bool reset(const variable_map_t& vm, const parameter_map_t& pm);

  /**
   * 変数に連続性を設定する
   */
  virtual void set_continuity(const std::string &name, const int& dc);

  /**
   * 制約を追加する．
   */
  virtual void add_constraint(const constraints_t& constraints);

  /**
   * 制約を追加する．
   */
  virtual void add_constraint(const node_sptr& constraint);

  /**
   * 制約ストアが無矛盾かを判定する．
   * @return 充足可能な場合の記号定数条件列，充足不可能な場合の記号定数条件列（それぞれ存在しない場合は空の列を返す）
   */
  virtual CheckConsistencyResult check_consistency();

  /**
   * 現在の制約ストアから変数表を作成する
   */
  virtual create_result_t create_maps();

  /**
   * 変数表を用いて制約ストアを上書きする．
   */
  virtual void reset_constraint(const variable_map_t& vm, const bool& send_derivatives);

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
   * ガード制約を追加する
   */
  virtual void add_guard(const node_sptr&);

  /**
   * 現在の制約ストアを文字列で取得する
   */
  virtual std::string get_constraint_store();

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
   * TODO: 今のところ何もしないようなのでダミー実装
   */
  virtual void approx_vm(variable_range_map_t& vm);

  // deleted
  //  //SymbolicValueを指定された精度で数値に変換する
  //  virtual std::string get_real_val(const value_t &val, int precision, hydla::symbolic_simulator::OutputFormat opfmt);

private:
  /**
   * Variableの生成
   */
  const node_sptr make_variable(const std::string &name, const int& dc, const bool& is_prev = false) const;

  /**
   * acceptを経由してvalue_tをsymbolic_value_tにダウンキャストする
   */
  const symbolic_value_t get_symbolic_value_t(value_t value);

  /**
   * ValueVisitorの実装
   */
  virtual void visit(symbolic_value_t& value);


  hydla::simulator::symbolic::Mode mode_;

  REDUCELink cl_;

  /**
   * ValueVisior::visit()から得た値を一時格納する
   */
  symbolic_value_t visited_;

};

} // namespace reduce
} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_VCS_REDUCE_VCS_H_
