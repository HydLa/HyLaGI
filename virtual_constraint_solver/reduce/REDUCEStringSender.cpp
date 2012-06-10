#include "REDUCEStringSender.h"

#include "Logger.h"
#include <assert.h>
#include <iostream>

#include "VariableNameEncoder.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace vcs {
namespace reduce {

/** 空集合を表すREDUCE入力用文字列 "{}" */
const std::string REDUCEStringSender::empty_list_string("{}");

/** REDUCEに送る際に変数名につける接頭語 "usrvar" */
const std::string REDUCEStringSender::var_prefix("usrvar");

/** REDUCEに送る際に定数名につける接頭語 */
const std::string REDUCEStringSender::par_prefix("p");

REDUCEStringSender::REDUCEStringSender() :
  cl_(NULL),
  differential_count_(0),
  in_prev_(false),
  apply_not_(false)
{}

REDUCEStringSender::REDUCEStringSender(REDUCELink& cl) :
  cl_(&cl),
  differential_count_(0),
  in_prev_(false),
  apply_not_(false)
{}

REDUCEStringSender::~REDUCEStringSender(){}


// Ask制約 BinaryNode
void REDUCEStringSender::visit(boost::shared_ptr<Ask> node){
  // ask制約は送れない
  assert(0);
}

// Tell制約 UNARYNODE
void REDUCEStringSender::visit(boost::shared_ptr<Tell> node)                  {
  // tell制約は送れない
  assert(0);
}

#define START_P "("
#define END_P   ")"

#define DEFINE_VISIT_BINARY(NODE_NAME, FUNC_NAME)                       \
  void REDUCEStringSender::visit(boost::shared_ptr<NODE_NAME> node)     \
  {                                                                     \
  cl_->send_string(#FUNC_NAME START_P);                                 \
  accept(node->get_lhs());                                              \
  cl_->send_string(", ");                                               \
  accept(node->get_rhs());                                              \
  cl_->send_string(END_P);                                              \
}

#define DEFINE_VISIT_BINARY_WITH_NOT_FUNC(NODE_NAME, FUNC_NAME, NOT_FUNC_NAME)     \
  void REDUCEStringSender::visit(boost::shared_ptr<NODE_NAME> node)                \
  {                                                                                \
  if(!apply_not_) cl_->send_string(#FUNC_NAME START_P);                            \
  else cl_->send_string(#NOT_FUNC_NAME START_P);                                   \
  accept(node->get_lhs());                                                         \
  cl_->send_string(", ");                                                          \
  accept(node->get_rhs());                                                         \
  cl_->send_string(END_P);                                                         \
}

#define DEFINE_VISIT_UNARY(NODE_NAME, FUNC_NAME)                        \
  void REDUCEStringSender::visit(boost::shared_ptr<NODE_NAME> node)     \
  {                                                                     \
  cl_->send_string(#FUNC_NAME START_P);                                 \
  accept(node->get_child());                                            \
  cl_->send_string(END_P);                                              \
}

#define DEFINE_VISIT_FACTOR(NODE_NAME, FACTOR_NAME)                     \
  void REDUCEStringSender::visit(boost::shared_ptr<NODE_NAME> node)     \
  {                                                                     \
  cl_->send_string(#FACTOR_NAME);                                       \
}

// 比較演算子 ASYMMETRIC_BINARY_NODE
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(Equal, equal, neq)
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(UnEqual, neq, equal)
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(Less, lessp, geq)
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(LessEqual, leq, greaterp)
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(Greater, greaterp, leq)
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(GreaterEqual, geq, lessp)


// 論理演算子
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(LogicalAnd, and, or)
DEFINE_VISIT_BINARY_WITH_NOT_FUNC(LogicalOr, or, and)

  
// 算術二項演算子 BINARY_NODE
DEFINE_VISIT_BINARY(Plus, plus)
DEFINE_VISIT_BINARY(Subtract, difference)
DEFINE_VISIT_BINARY(Times, times)
DEFINE_VISIT_BINARY(Divide, quotient)
DEFINE_VISIT_BINARY(Power, expt)

  
// 算術単項演算子 UNARYNODE
DEFINE_VISIT_UNARY(Negative, -)
void REDUCEStringSender::visit(boost::shared_ptr<Positive> node)              {
  accept(node->get_child());
}
  
// 微分 UNARYNODE df(x,t)と表記
void REDUCEStringSender::visit(boost::shared_ptr<Differential> node)          {
  differential_count_++;
  accept(node->get_child());
  differential_count_--;  
}

// 左極限 UNARYNODE
void REDUCEStringSender::visit(boost::shared_ptr<Previous> node)              {
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}

// 否定
void REDUCEStringSender::visit(boost::shared_ptr<Not> node)                   {
  apply_not_ = !apply_not_;
  accept(node->get_child());
  apply_not_ = !apply_not_;
}

/*
// 三角関数
DEFINE_VISIT_UNARY(Sin, sin)
DEFINE_VISIT_UNARY(Cos, cos)
DEFINE_VISIT_UNARY(Tan, tan)
// 逆三角関数
DEFINE_VISIT_UNARY(Asin, asin)
DEFINE_VISIT_UNARY(Acos, acos)
DEFINE_VISIT_UNARY(Atan, atan)
// 対数
DEFINE_VISIT_BINARY(Log, logb) // 使えない？
DEFINE_VISIT_UNARY(Ln, log)
*/

// 円周率
DEFINE_VISIT_FACTOR(Pi, pi)
// 自然対数の底
DEFINE_VISIT_FACTOR(E, e)


  
// 変数 FactorNode
void REDUCEStringSender::visit(boost::shared_ptr<Variable> node)              {
  var_info_t new_var = boost::make_tuple(node->get_name(),
                                         differential_count_,
                                         in_prev_ && !ignore_prev_,
                                         node->get_init_var());

  put_var(new_var);
}

// 数字 FactorNode
void REDUCEStringSender::visit(boost::shared_ptr<Number> node)                {
  cl_->send_string(node->get_number());
}

// 記号定数
void REDUCEStringSender::visit(boost::shared_ptr<Parameter> node)
{
  HYDLA_LOGGER_REST("#*** Begin REDUCEStringSender::visit(boost::shared_ptr<Parameter> node ***");
  HYDLA_LOGGER_REST("put: Parameter : ", node->get_name());
  put_par(par_prefix + node->get_name());
  HYDLA_LOGGER_REST("#*** End REDUCEStringSender::visit(boost::shared_ptr<Parameter> node ***");
}

// t
void REDUCEStringSender::visit(boost::shared_ptr<SymbolicT> node)
{
  cl_->send_string("t");
}


void REDUCEStringSender::put_var(const var_info_t var)
{
  HYDLA_LOGGER_REST("#*** Begin REDUCEStringSender::put_var ***");
  VariableNameEncoder vne;
  std::string name(REDUCEStringSender::var_prefix + vne.LowerEncode(var.get<0>()));
  int diff_count = var.get<1>();
  bool prev      = var.get<2>();
  bool init_var  = var.get<3>();

  HYDLA_LOGGER_REST(
    "name: ", name,
    "\tdiff_count: ", diff_count,
    "\tprev: ", prev,
    "\tinit_var: ", init_var);


  std::ostringstream var_str;

  if(init_var){
    var_str << "init";
    var_str << name;
    if(diff_count > 0){
      var_str << "_"
              << diff_count;
    }
    var_str << "lhs";
  }else{
    if (diff_count > 0 && prev){
      var_str << "prev(df("
              << name
              << ",t,"    
              << diff_count
              << "))";
    }
    else if (diff_count > 0){
      var_str << "df("
              << name
              << ",t,"    
              << diff_count
              << ")";
    }
    else if (prev) {
      var_str << "prev("
              << name
              << ")";
    }
    else {
      var_str << name;
    }
  }

  cl_->send_string(var_str.str());
  HYDLA_LOGGER_REST("var_str: ", var_str.str());

  // putした変数の情報を保持
  vars_.insert(var);

  HYDLA_LOGGER_REST("vars_size(): ",
                    vars_.size());
  HYDLA_LOGGER_REST("#*** End REDUCEStringSender::put_var ***");

}

void REDUCEStringSender::put_par(const std::string &name)
{
  HYDLA_LOGGER_REST("#*** Begin REDUCEStringSender::put_par ***");
  HYDLA_LOGGER_REST("put_par: ", "name: ", name);

  cl_->send_string(name);

  // putした変数の情報を保持
  pars_.insert(name);

  HYDLA_LOGGER_REST("#*** End REDUCEStringSender::put_par ***");

}

/**
 * ある式(ノード)をputする
 * @param node putしたい式(ノード)
 */

void REDUCEStringSender::put_node(const node_sptr& node, bool ignore_prev)
{
  HYDLA_LOGGER_REST("#*** Begin REDUCEStringSender::put_node ***");
  differential_count_ = 0;
  in_prev_ = false;
  ignore_prev_ = ignore_prev;
  accept(node);
  HYDLA_LOGGER_REST("#*** End REDUCEStringSender::put_node ***");
}

/**
 * ある式のリストをputする
 */
void REDUCEStringSender::put_nodes(const std::vector<node_sptr>& constraints)
{
  HYDLA_LOGGER_REST("#*** Begin REDUCEStringSender::put_nodes ***");
  cl_->send_string("{");
  for(std::vector<node_sptr>::const_iterator it = constraints.begin(); it != constraints.end(); it++){
    if(it!=constraints.begin()) cl_->send_string(",");
    put_node(*it);
  }
  cl_->send_string("}");
  HYDLA_LOGGER_REST("#*** End REDUCEStringSender::put_nodes: ***");
}

/**
 * 変数の一覧を送信．
 */
void REDUCEStringSender::put_vars(bool ignore_prev)
{
  HYDLA_LOGGER_REST("#*** Begin REDUCEStringSender::put_vars ***");
  HYDLA_LOGGER_REST("var size:", vars_.size());

  cl_->send_string("{");
  vars_const_iterator it  = vars_begin();
  vars_const_iterator end = vars_end();
  for(; it!=end; ++it) {
    if(it!=vars_begin()) cl_->send_string(",");
    //    put_var(*it);
    put_var(boost::make_tuple(it->get<0>(),
                              it->get<1>(),
                              it->get<2>() && !ignore_prev,
                              it->get<3>()));
  }
  cl_->send_string("}");

  HYDLA_LOGGER_REST("#*** End REDUCEStringSender::put_vars ***");
}

void REDUCEStringSender::put_pars()
{
  HYDLA_LOGGER_REST("#*** Begin REDUCEStringSender::put_pars ***");
  HYDLA_LOGGER_REST("par size:", pars_.size());
  cl_->send_string("{");
  for(std::set<std::string>::iterator it = pars_.begin(); it!=pars_.end(); ++it) {
    if(it!=pars_.begin()) cl_->send_string(",");
    put_par(*it);
  }
  cl_->send_string("}");

  HYDLA_LOGGER_REST("#*** End REDUCEStringSender::put_pars ***");
}

/**
 * 内部情報(特に変数情報)をリセットする．
 * 式のputをやり直したいときなどに
 */
void REDUCEStringSender::clear()
{
  differential_count_ = 0;
  in_prev_ = false;

  vars_.clear();
}

namespace {

struct MaxDiffMapDumper
{
  template<typename T>
  MaxDiffMapDumper(T it, T end)
  {
    for(; it!=end; ++it) {
      s << "name: " << it->first
        << "diff: " << it->second
        << "\n";
    }
  }

  std::stringstream s;
};

}

void REDUCEStringSender::create_max_diff_map(max_diff_map_t& max_diff_map) 
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEStringSender::create_max_diff_map ***");
  vars_const_iterator vars_it  = vars_begin();
  vars_const_iterator vars_end_it = vars_end();
  for(; vars_it!=vars_end_it; ++vars_it) {
    std::string name(vars_it->get<0>());
    int derivative_count = vars_it->get<1>();

    max_diff_map_t::iterator it = max_diff_map.find(name);
    if(it==max_diff_map.end()) {
      max_diff_map.insert(
        std::make_pair(name, derivative_count));
    }
    else if(it->second < derivative_count) {
      it->second = derivative_count;
    }
  }

  HYDLA_LOGGER_VCS(
    "--- max diff map ---\n",
    MaxDiffMapDumper(max_diff_map.begin(),
                     max_diff_map.end()).s.str());
  HYDLA_LOGGER_VCS("#*** End REDUCEStringSender::create_max_diff_map ***");

}

} //namespace reduce
} //namespace vcs
} //namespace hydla
