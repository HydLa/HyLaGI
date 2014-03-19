#ifndef _INCLUDED_HYDLA_BACKEND_BACKEND_H_
#define _INCLUDED_HYDLA_BACKEND_BACKEND_H_
#include "Link.h"

#include "DefaultTreeVisitor.h"
#include "ParseTree.h"
#include "PhaseResult.h"
#include "Simulator.h"
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
typedef hydla::parse_tree::node_sptr      node_sptr;
typedef hydla::simulator::CheckConsistencyResult check_consistency_result_t;
typedef std::vector<variable_map_t>       create_vm_t;

struct MidpointRadius
{
  value_t              midpoint;
  value_t              radius;
};

struct TimeIdsPair
{
  value_t time;
  std::vector<int> ids;
};

/**
 * calculate_next_PP_timeで返す構造体
 */
typedef struct NextPhaseResult 
{
  /// minimum time and the id of its condition
  TimeIdsPair             minimum;
  /// non-minimum pairs of times and ids
  std::vector<TimeIdsPair> non_minimums;
  /// condition for parameter in this case
  hydla::simulator::parameter_map_t parameter_map;
} candidate_t;

typedef std::vector<candidate_t> pp_time_result_t;

/**
 * 離散変化条件として渡す構造体
 */
typedef struct DCCause
{
  node_sptr node;
  int id;
  DCCause(node_sptr n, int i):node(n), id(i){}
} dc_cause_t;
typedef std::vector<dc_cause_t> dc_causes_t;
  

class Backend : public hydla::parse_tree::DefaultTreeVisitor
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
   *    e(n,p,z,t): node_sptr: expression (Variables are handled like n:x, p:prev[x], x[0], x[t], needed only for sending)
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
        std::string name = it->get_name();
        int diff = it->get_differential_count();
        call("addVariable", 2, "si", "", ("usrVar" + name).c_str(), &diff);
      }
  }

  void set_parameter_map(parameter_map_t& p){
    for(parameter_map_t::iterator it = p.begin(); it != p.end(); it++)
      {
        call("addParameter", 1, "p", "", &(it->first));
      }
  }


  private:
  static const std::string prev_prefix;
  static const std::string par_prefix;
  static const std::string var_prefix;
  /// throw an exception for an invalid format
  void invalid_fmt(const char* fmt, int idx);
  void invalid_ret();

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
  node_sptr receive_function();
  node_sptr receive_node();

  /**
   * 与えられたノードの送信をおこなう
   *
   * ノードの送信をおこなう際は直接visit関数を呼ばずに，
   * 必ずこの関数を経由すること
   */
  int send_node(const hydla::parse_tree::node_sptr& node, const variable_form_t &form);

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
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalOr> node);

  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);
  
  // コマンド文
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Scan> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::True> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::False> node);

  // 算術単項演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);

  // 微分
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  // 否定
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Not> node);
  
  // 関数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnsupportedFunction> node);
  
  
  // 円周率
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  // 自然対数の底
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  
    
  // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Float> node);

  // 記号定数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);
  
  // 無限大
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Infinity> node);
  
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

#endif // include guard
