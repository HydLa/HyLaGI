#pragma once

#include "Link.h"

#include "DefaultTreeVisitor.h"
#include "PhaseSimulator.h"

namespace hydla {
namespace backend {

typedef simulator::value_t value_t;
typedef simulator::variable_t variable_t;
typedef simulator::variable_map_t variable_map_t;
typedef simulator::parameter_map_t parameter_map_t;
typedef simulator::ValueRange value_range_t;
typedef simulator::parameter_t parameter_t;
typedef simulator::variable_set_t variable_set_t;
typedef simulator::ConstraintStore constraint_store_t;
typedef simulator::find_min_time_result_t find_min_time_result_t;
typedef simulator::CompareMinTimeResult compare_min_time_result_t;
typedef symbolic_expression::node_sptr node_sptr;
typedef simulator::CheckConsistencyResult check_consistency_result_t;
typedef std::vector<variable_map_t> create_vm_t;

struct MidpointRadius {
  value_t midpoint;
  value_t radius;
};

struct CalculateTLinearResult {
  node_sptr exp;
  MidpointRadius mid_rad;
};

class Backend : public symbolic_expression::DefaultTreeVisitor {

  enum VariableForm { VF_NONE, VF_IGNORE_PREV, VF_PREV, VF_TIME, VF_ZERO };

public:
  Backend(Link *link);
  virtual ~Backend();
  /** set value to variable
   *  @param name name of variable
   *  @param value value to be set
  template<typename T>
    int set(const char* name, const T value) { return link_->set(name, value); }
  */

  /** call of function
   *  @param name name of function
   *  @param trace If true, the trace will be logged
   *  @param arg_cnt count of arguments of function
   *  @param args_fmt  format for the following arguments
   *  @param ret_fmt format for the following return-values (after args)
   *  @return 0 for success, otherwise non-zero value
   *
   *
   *    b: bool: boolean value
   *    cp: compare_min_time_result_t (receive only)
   *    db: double
   *    gm: map<constraint_t, bool> : map of guard satisfaction
   *    i: int: integer
   *    s: const char*: symbol (send only)
   *    e(n, p, c, z, t): symbolic_expression::node_sptr: expression (Variables
   * are handled like n:x, c:x (ignoring prev), p:prev[x], z:x[0], t:x[t],
   * needed only for sending) vl(n, p, z, t): value_t: value (following n,p,z
   * and t are only for sending) cs(n, p, z, t): constraint_store_t: constraint
   * store cc: check_consistency_result_t (receive only) cv: create_vm_t
   * (receive only) mv[0](n, p, z, t): variable_map_t: variable map (If '0' is
   * appended, derivatives are not sent. Characters after them are the same as
   * 'e') lp: std::list<parameter_t> : send only mp: parameter_map_t : send only
   *    mps: std::vector<parameter_map_t> : receive only
   *    r: MidpointRadius: midpoint_radius form (receive only)
   *    ct: CalculateTLinearResult: (receive only)
   *    f: find_min_time_result_t (receive only)
   *    p: parameter_t (send only)
   *    tl: std::vector<simulator::TimeListElement> (send only)
   *    v(n, p, z, t): variable_t: variable (Characters after them are the same
   * as 'e') (send only) vs(n, p, z, t): variable_set_t: variable set example:
   * call("add", true, 2, "ii", "i", &lhs, &rhs, &res) Caution: In Mathematica,
   * '_' cannot be used as name of symbols REDUCE doesn't distinguish whether
   * characters are in upper cases or not.
   */
  int call(const char *name, bool trace, int arg_cnt, const char *args_fmt,
           const char *ret_fmt, ...);

  void set_variable_set(variable_set_t &v) {
    call("resetVariables", false, 0, "", "");
    for (variable_set_t::iterator it = v.begin(); it != v.end(); it++) {
      std::string name = var_prefix + it->get_name();
      int diff = it->get_differential_count();
      call("addVariable", false, 2, "si", "", name.c_str(), &diff);
    }
  }

  void reset();

private:
  static const std::string var_prefix, prev_prefix, par_prefix;
  /// throw an exception for an invalid format
  void invalid_fmt(const char *fmt, int idx);
  void invalid_ret();

  std::string remove_prefix(const std::string &original,
                            const std::string &prefix);

  int send_variable_map(const variable_map_t &vm, const VariableForm &form,
                        const bool &send_derivative);
  int send_parameter_map(const parameter_map_t &pm);

  void send_parameter_list(const std::list<parameter_t> &par_list);

  void send_value(const simulator::value_t &val, const VariableForm &vf);
  void send_time_list(const std::vector<simulator::TimeListElement> &list);
  void send_cs(const simulator::ConstraintStore &cs, const VariableForm &vf);

  int receive_map(variable_map_t &vm);
  void receive_bool(bool &b);
  int receive_parameter_maps(std::vector<parameter_map_t> &pm);
  compare_min_time_result_t receive_compare_min_time_result();
  check_consistency_result_t receive_cc();
  CalculateTLinearResult receive_ct();
  create_vm_t receive_cv();
  constraint_store_t receive_cs();

  value_t receive_value();
  symbolic_expression::node_sptr receive_function();
  symbolic_expression::node_sptr receive_node();

  /**
   * 与えられたノードの送信をおこなう
   *
   * ノードの送信をおこなう際は直接visit関数を呼ばずに，
   * 必ずこの関数を経由すること
   */
  int send_node(const symbolic_expression::node_sptr &node,
                const VariableForm &form);

  /**
   * 変数の送信
   */
  void send_variable(const variable_t &var, const VariableForm &variable_arg);
  void send_variable(const std::string &variable_name, int diff_count,
                     const VariableForm &variable_arg);

  int read_args_fmt(const char *args_fmt, const int &idx, void *arg);
  int read_ret_fmt(const char *ret_fmt, const int &idx, void *ret);

  bool get_form(const char &form_c, VariableForm &form);

  // Ask制約
  virtual void visit(std::shared_ptr<symbolic_expression::Ask> node);

  // Exists
  virtual void visit(std::shared_ptr<symbolic_expression::Exists> node);

  // Tell制約
  virtual void visit(std::shared_ptr<symbolic_expression::Tell> node);

  // 比較演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Equal> node);
  virtual void visit(std::shared_ptr<symbolic_expression::UnEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Less> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LessEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Greater> node);
  virtual void visit(std::shared_ptr<symbolic_expression::GreaterEqual> node);

  // 論理演算子
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalAnd> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalOr> node);

  // 算術二項演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Plus> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Subtract> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Times> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Divide> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Power> node);

  // コマンド文
  virtual void visit(std::shared_ptr<symbolic_expression::PrintPP> node);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintIP> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Scan> node);

  virtual void visit(std::shared_ptr<symbolic_expression::True> node);
  virtual void visit(std::shared_ptr<symbolic_expression::False> node);

  // 算術単項演算子
  virtual void visit(std::shared_ptr<symbolic_expression::Negative> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Positive> node);

  // 微分
  virtual void visit(std::shared_ptr<symbolic_expression::Differential> node);

  // 左極限
  virtual void visit(std::shared_ptr<symbolic_expression::Previous> node);

  // 否定
  virtual void visit(std::shared_ptr<symbolic_expression::Not> node);

  // 関数
  virtual void visit(std::shared_ptr<symbolic_expression::Function> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::UnsupportedFunction> node);

  // 円周率
  virtual void visit(std::shared_ptr<symbolic_expression::Pi> node);
  // 自然対数の底
  virtual void visit(std::shared_ptr<symbolic_expression::E> node);

  // 変数
  virtual void visit(std::shared_ptr<symbolic_expression::Variable> node);

  // 数字
  virtual void visit(std::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Float> node);

  // 記号定数
  virtual void visit(std::shared_ptr<symbolic_expression::Parameter> node);

  // t
  virtual void visit(std::shared_ptr<symbolic_expression::SymbolicT> node);

  // 無限大
  virtual void visit(std::shared_ptr<symbolic_expression::Infinity> node);

  virtual void visit(std::shared_ptr<symbolic_expression::ImaginaryUnit> node);

  virtual void
  visit(std::shared_ptr<symbolic_expression::ExpressionListElement> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Range> node);

  VariableForm adapt_variable_form(VariableForm orig, bool in_prev);

  // Differentialノードを何回通ったか
  int differential_count_;

  /// Prevノードの下にいるかどうか
  int in_prev_;

  MidpointRadius receive_midpoint_radius();

  find_min_time_result_t receive_find_min_time_result();

  void put_converted_function(const std::string &name, int arg_cnt);

  std::shared_ptr<Link> link_;

  VariableForm variable_arg_;

  // valと関係演算子を元に、rangeを設定する
  void set_range(const simulator::value_t &val, simulator::range_t &range,
                 const int &relop);
};

} // namespace backend
} // namespace hydla
