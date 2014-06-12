#pragma once

#include "Link.h"

#include "DefaultTreeVisitor.h"
#include "PhaseSimulator.h"

namespace hydla{
namespace backend{

typedef hydla::simulator::value_t         value_t;
typedef hydla::simulator::variable_t      variable_t;
typedef hydla::simulator::variable_map_t  variable_map_t;
typedef hydla::simulator::parameter_map_t parameter_map_t;
typedef hydla::simulator::ValueRange      value_range_t;
typedef hydla::simulator::parameter_t     parameter_t;
typedef hydla::simulator::variable_set_t  variable_set_t;
typedef hydla::simulator::ConstraintStore constraint_store_t;
typedef Link::VariableForm        variable_form_t;
typedef hydla::symbolic_expression::node_sptr      node_sptr;
typedef hydla::simulator::CheckConsistencyResult check_consistency_result_t;
typedef std::vector<variable_map_t>       create_vm_t;

struct MidpointRadius
{
  value_t              midpoint;
  value_t              radius;
};

/**
 * 離散変化に関する情報
 */
struct DCInformation
{
  value_t time;         /// 離散変化時刻
  std::vector<int> ids; /// 原因となった条件のID
  bool on_time;         /// 計算した時刻に条件が成り立つかどうか
};

struct DCCandidate 
{
  /// minimum time and the id of its condition
  DCInformation             minimum;
  /// non-minimum pairs of times and ids
  std::vector<DCInformation> non_minimums;
  /// condition for parameter in this case
  hydla::simulator::parameter_map_t parameter_map;
};

typedef std::vector<DCCandidate> pp_time_result_t;

/**
 * 離散変化条件として渡す構造体
 */
typedef struct DCCause
{
  symbolic_expression::node_sptr node;
  int id;
  DCCause(symbolic_expression::node_sptr n, int i):node(n), id(i){}
} dc_cause_t;
typedef std::vector<dc_cause_t> dc_causes_t;
  

class Backend : public hydla::symbolic_expression::DefaultTreeVisitor
{
  public:

  Backend(Link *link);
  virtual ~Backend();
  /** set value to variable
   *  @param name name of variable
   *  @param value value to be set
  template<typename T>
    int set(const char* name, const T value){return link_->set(name, value);}
  */

  /** call of function
   *  @param name name of function
   *  @param arg_cnt count of arguments of function
   *  @param args_fmt  format for following arguments
   *  @param ret_fmt format for following return-values (after args)
   *  @return 0 for success, otherwise non-zero value
   *
   *  format is like below
   *    r: MidpointRadius: midpoint_radius form (receive only)
   *    i: int: integer
   *    b: bool: boolean value
   *    s: const char*: symbol (send only)
   *    e(n,p,z,t): symbolic_expression::node_sptr: expression (Variables are handled like n:x,c:x (ignoring prev), p:prev[x], x[0], x[t], needed only for sending)
   *    dc: dc_causes_t : causes of discrete changes
   *    vl(n, p, z, t): value_t: value (following n,p,z and t are only for sending)
   *    cs(n, p, z, t): constraint_store_t: constraint store
   *    cc: check_consistency_result_t (receive only)
   *    cv: create_vm_t (receive only)
   *    mv[0](n, p, z, t): variable_map_t: variable map (If '0' is appended, derivatives are not sent. Characters after them are the same as 'e')
   *    mp: parameter_map_t : parameter map (just like variable map)
   *    cp: pp_time_result_t (receive only)
   *    p: parameter_t (send only)
   *    v(n, p, z, t): variable_t: variable (Characters after them are the same as 'e') (send only)
   *  example: call("Add", "ii", "i", &lhs, &rhs, &res)
   *  Caution: In Mathematica, '_' cannot be used as name of symbols
   *           REDUCE doesn't distinguish whether characters are in upper cases or not.
   */
  int call(const char* name, int arg_cnt, const char* args_fmt, const char* ret_fmt, ...);

  void set_variable_set(variable_set_t& v){
    for(variable_set_t::iterator it = v.begin(); it != v.end(); it++)
      {
        std::string name = var_prefix + it->get_name();
        int diff = it->get_differential_count();
        call("addVariable", 2, "si", "", name.c_str(), &diff);
      }
  }

  private:
  static const std::string var_prefix;
  /// throw an exception for an invalid format
  void invalid_fmt(const char* fmt, int idx);
  void invalid_ret();

  std::string remove_prefix(const std::string &original, const std::string &prefix);

  int send_variable_map(const variable_map_t& vm, const variable_form_t &form, const bool &send_derivative);
  int send_parameter_map(const parameter_map_t& pm);

  int send_value(const hydla::simulator::value_t& val, const variable_form_t &var);


  int receive_map(variable_map_t &vm);
  void receive_bool(bool &b);
  int receive_parameter_map(parameter_map_t &pm);
  pp_time_result_t receive_cp();
  check_consistency_result_t receive_cc();
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
  int send_node(const hydla::symbolic_expression::node_sptr& node, const variable_form_t &form);

  void send_dc_causes(dc_causes_t &dc_causes);

  /**
   * 変数の送信
   */
  int send_variable(const variable_t &var, const variable_form_t& variable_arg);
  int send_variable(const std::string& variable_name, int diff_count, const variable_form_t& variable_arg);


  int read_args_fmt(const char* args_fmt, const int& idx, void* arg);
  int read_ret_fmt(const char* ret_fmt, const int& idx, void* ret);

  
  bool get_form(const char &form_c, variable_form_t &form);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Less> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::LogicalOr> node);

  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Power> node);
  
  // コマンド文
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Scan> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::True> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::False> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node);

  // 否定
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Not> node);
  
  // 関数
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Function> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::UnsupportedFunction> node);
  
  
  // 円周率
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node);
  // 自然対数の底
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::E> node);
  
    
  // 変数
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Number> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Float> node);

  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node);
  
  // 無限大
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Infinity> node);
  
  // Differentialノードを何回通ったか
  int differential_count_;

  /// Prevノードの下にいるかどうか
  int in_prev_;

  /** 
   * whether to send negation of logical operators
   * (needed for backend which cannot use logical negation easily
   */
  bool apply_not_;

  MidpointRadius receive_midpoint_radius();

  void put_converted_function(const std::string& name, int arg_cnt);


  boost::shared_ptr<Link> link_;

  variable_form_t variable_arg_;

  //valと関係演算子を元に、rangeを設定する
  void set_range(const hydla::simulator::value_t &val, hydla::simulator::range_t &range, const int& relop);
};

} // namespace backend
} // namespace hydla
