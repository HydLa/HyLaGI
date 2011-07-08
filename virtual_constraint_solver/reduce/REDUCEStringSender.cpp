#include "REDUCEStringSender.h"

#include "Logger.h"
#include <assert.h>
#include <iostream>

using namespace hydla::parse_tree;

namespace hydla {
namespace vcs {
namespace reduce {

REDUCEStringSender::REDUCEStringSender(REDUCELink& cl) :
  cl_(cl),
  differential_count_(0),
  in_prev_(false),
  in_prev_point_(false)
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

// 比較演算子 ASYMMETRIC_BINARY_NODE
void REDUCEStringSender::visit(boost::shared_ptr<Equal> node)                 {
	accept(node->get_lhs());
  cl_.send_string("=");
	accept(node->get_rhs());
}
void REDUCEStringSender::visit(boost::shared_ptr<UnEqual> node)               {
	accept(node->get_lhs());
  cl_.send_string("!=");
	accept(node->get_rhs());
}
void REDUCEStringSender::visit(boost::shared_ptr<Less> node)                  {
	accept(node->get_lhs());
  cl_.send_string("<");
	accept(node->get_rhs());
}
void REDUCEStringSender::visit(boost::shared_ptr<LessEqual> node)             {
	accept(node->get_lhs());
  cl_.send_string("<=");
	accept(node->get_rhs());
}
void REDUCEStringSender::visit(boost::shared_ptr<Greater> node)               {
	accept(node->get_lhs());
  cl_.send_string(">");
	accept(node->get_rhs());
}
void REDUCEStringSender::visit(boost::shared_ptr<GreaterEqual> node)          {
	accept(node->get_lhs());
  cl_.send_string(">=");
	accept(node->get_rhs());
}

// 論理演算子
void REDUCEStringSender::visit(boost::shared_ptr<LogicalAnd> node)            {
  cl_.send_string("{LogicalAnd, ");
	accept(node->get_lhs());
  cl_.send_string(", ");
	accept(node->get_rhs());
  cl_.send_string("}");
}
void REDUCEStringSender::visit(boost::shared_ptr<LogicalOr> node)             {
  cl_.send_string("{LogicalOr, ");
	accept(node->get_lhs());
  cl_.send_string(", ");
	accept(node->get_rhs());
  cl_.send_string("}");
}
  
// 算術二項演算子 BINARY_NODE
void REDUCEStringSender::visit(boost::shared_ptr<Plus> node)                  {
  cl_.send_string("(");
	accept(node->get_lhs());
  cl_.send_string("+");
	accept(node->get_rhs());
  cl_.send_string(")");
}
void REDUCEStringSender::visit(boost::shared_ptr<Subtract> node)              {
  cl_.send_string("(");
	accept(node->get_lhs());
  cl_.send_string("-");
	accept(node->get_rhs());
  cl_.send_string(")");
}
void REDUCEStringSender::visit(boost::shared_ptr<Times> node)                 {
	accept(node->get_lhs());
  cl_.send_string("*");
	accept(node->get_rhs());
}
void REDUCEStringSender::visit(boost::shared_ptr<Divide> node)                {
	accept(node->get_lhs());
  cl_.send_string("/");
	accept(node->get_rhs());
}
void REDUCEStringSender::visit(boost::shared_ptr<Power> node)                {
	accept(node->get_lhs());
  cl_.send_string("^");
	accept(node->get_rhs());
}
  
// 算術単項演算子 UNARYNODE
void REDUCEStringSender::visit(boost::shared_ptr<Negative> node)              {
  cl_.send_string("-");
  accept(node->get_child());
}
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
void REDUCEStringSender::visit(boost::shared_ptr<Not> node)
{
  // TODO

// cl_.send_string("{Not, ");
  accept(node->get_child());
// cl_.send_string("}");
}
  
// 変数 FactorNode
void REDUCEStringSender::visit(boost::shared_ptr<Variable> node)              {
  var_info_t new_var =
    boost::make_tuple(node->get_name(),
                      differential_count_,
                      in_prev_ && !ignore_prev_);

  put_var(new_var);
}

// 数字 FactorNode
void REDUCEStringSender::visit(boost::shared_ptr<Number> node)                {
  cl_.send_string(node->get_number());
}

// 記号定数
void REDUCEStringSender::visit(boost::shared_ptr<Parameter> node)
{
  cl_.send_string(node->get_name());
}

// t
void REDUCEStringSender::visit(boost::shared_ptr<SymbolicT> node)
{
  cl_.send_string("t");
}


void REDUCEStringSender::put_var(const var_info_t var, bool init_var)
{
  std::string name(var.get<0>());
  int diff_count = var.get<1>();
  bool prev      = var.get<2>();

  HYDLA_LOGGER_REST(
    "REDUCEStringSender::put_var: ",
    "name: ", name,
    "\tdiff_count: ", diff_count,
    "\tprev: ", prev,
    "\tinit_var: ", init_var);


  std::ostringstream var_str;

  if(init_var) var_str << "init";

  if (diff_count > 0 && prev){
    var_str << "prev(df("
            << name
            << ",t,"    
            << diff_count
            << "))";
  }
  else if (diff_count > 0){
    if(init_var) {
      var_str << name
              << "_"
              << diff_count;
    }
    else {
      var_str << "df("
              << name
              << ",t,"    
              << diff_count
              << ")";
    }
  }
  else if (prev) {
    var_str << "prev("
            << name
            << ")";
  }
  else {
    var_str << name;
  }

  if(init_var) var_str << "lhs";

  cl_.send_string(var_str.str());
  HYDLA_LOGGER_REST("var_str: ", var_str.str());

  // putした変数の情報を保持
  vars_.insert(var);

}

/**
 * ある式(ノード)をputする
 * @param node putしたい式(ノード)
 */

void REDUCEStringSender::put_node(const node_sptr& node,
                            bool ignore_prev,
                            bool entailed)
{
  differential_count_ = 0;
  in_prev_ = false;
  in_prev_point_ = false;
  ignore_prev_ = ignore_prev;
  if(!entailed){
    // TODO
/*
    HYDLA_LOGGER_REST("put: Not");
    cl_.send_string("{Not, ");
    accept(node);
    cl_.send_string("}");
*/
  }
  accept(node);
}

/**
 * 変数の一覧を送信．
 */
void REDUCEStringSender::put_vars(bool ignore_prev)
{
  HYDLA_LOGGER_REST(
    "---- REDUCEStringSender::put_vars ----\n",
    "var size:", vars_.size());

  cl_.send_string("{");
  vars_const_iterator it  = vars_begin();
  vars_const_iterator end = vars_end();
  for(; it!=end; ++it) {
    if(it!=vars_begin()) cl_.send_string(",");
    put_var(boost::make_tuple(
              it->get<0>(),
              it->get<1>(),
              it->get<2>() && !ignore_prev));
  }
  cl_.send_string("}");

}

/**
 * 内部情報(特に変数情報)をリセットする．
 * 式のputをやり直したいときなどに
 */
void REDUCEStringSender::clear()
{
  differential_count_ = 0;
  in_prev_ = false;
  in_prev_point_ = false;

  vars_.clear();
}


} //namespace reduce
} //namespace vcs
} //namespace hydla
