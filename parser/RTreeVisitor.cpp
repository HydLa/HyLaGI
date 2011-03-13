#include "RTreeVisitor.h"

#include "Logger.h"
#include <assert.h>
#include <iostream>
//#include <string>

using namespace hydla::parse_tree;

namespace hydla {
namespace parse_tree {

//引数はダミー
RTreeVisitor::RTreeVisitor(std::string caller)
{
    caller_ = caller;
	expr_ = "";
//	std::cout << "rtv was created" << std::endl;
}

RTreeVisitor::RTreeVisitor(int a)
{
	expr_ = "";
//	std::cout << "rtv was created" << std::endl;
}

RTreeVisitor::~RTreeVisitor()
{
//		std::cout << "rtv was destroyed" << std::endl;
}

//c++動作練習
void RTreeVisitor::sandbox(){
//	std::cout << "sandbox called" << std::endl;
}

// 制約式を人が読めるstringにして返す
std::string RTreeVisitor::get_expr(const node_sptr& node){
	accept(node);
	std::string ret = expr_;
	expr_ ="";
	return ret;
}

//acceptを外部で行う場合
std::string RTreeVisitor::get_expr(){
	std::string ret = expr_;
	expr_ ="";
	return ret;
}


std::string RTreeVisitor::get_guard(const boost::shared_ptr<hydla::parse_tree::Ask>& ask){
	accept(ask->get_guard());
	std::string ret = expr_;
	expr_ ="";
	return ret;
}

std::string RTreeVisitor::get_ask_rhs(const boost::shared_ptr<hydla::parse_tree::Ask>& ask){
	accept(ask->get_child());
	std::string ret = expr_;
	expr_ ="";
	return ret;
}
// 定義
void RTreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {assert(0);}

// 呼び出し
void RTreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {assert(0);}

// 制約式
void RTreeVisitor::visit(boost::shared_ptr<Constraint> node)            {
	if(expr_==""){
		expr_ += "{Constraint, ";
	}else{
		expr_ += ", {Constraint, ";
	}
	accept(node->get_child());
	expr_ += "}";
}

// Ask制約 BinaryNode
/*
* 旧
void RTreeVisitor::visit(boost::shared_ptr<Ask> node){
//	expr_ += "Ask: ";
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
*/
//ms用
void RTreeVisitor::visit(boost::shared_ptr<Ask> node){
	expr_ += "{Ask, ";
	accept(node->get_lhs());
	expr_ += ", ";	
	accept(node->get_rhs());
	expr_ += "}";
}

// Tell制約 UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Tell> node)                  {
// 旧 reduce_outputに呼び出されたとき
	if(caller_=="reduce_output"){
		accept(node->get_child());
	}else{
		expr_ += "{Tell, ";
		accept(node->get_child());
		expr_ += "}";
	}
}

// 比較演算子 ASYMMETRIC_BINARY_NODE
void RTreeVisitor::visit(boost::shared_ptr<Equal> node)                 {
	accept(node->get_lhs());
	expr_ += "=";
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<UnEqual> node)               {
	accept(node->get_lhs());
	expr_ += "!=";
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<Less> node)                  {
	accept(node->get_lhs());
	expr_ += "<";
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<LessEqual> node)             {
	accept(node->get_lhs());
	expr_ += "<=";
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<Greater> node)               {
	accept(node->get_lhs());
	expr_ += ">";
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<GreaterEqual> node)          {
	accept(node->get_lhs());
	expr_ += ">=";
	accept(node->get_rhs());
}

// 論理演算子
void RTreeVisitor::visit(boost::shared_ptr<LogicalAnd> node)            {
	expr_ += "{LogicalAnd, ";
	accept(node->get_lhs());
	expr_ += ", ";
	accept(node->get_rhs());
	expr_ += "}";
}
void RTreeVisitor::visit(boost::shared_ptr<LogicalOr> node)             {
	expr_ += "{LogicalOr, ";
	accept(node->get_lhs());
	expr_ += ", ";
	accept(node->get_rhs());
	expr_ += "}";

}
  
// 算術二項演算子 BINARY_NODE
void RTreeVisitor::visit(boost::shared_ptr<Plus> node)                  {
	expr_ += "(";
	accept(node->get_lhs());
	expr_ += "+";
	accept(node->get_rhs());
	expr_ += ")";
}
void RTreeVisitor::visit(boost::shared_ptr<Subtract> node)              {
	expr_ += "(";
	accept(node->get_lhs());
	expr_ += "-";
	accept(node->get_rhs());
	expr_ += ")";
}
void RTreeVisitor::visit(boost::shared_ptr<Times> node)                 {
	accept(node->get_lhs());
	expr_ += "*";
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<Divide> node)                {
	accept(node->get_lhs());
	expr_ += "/";
	accept(node->get_rhs());
}
  
// 算術単項演算子 UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Negative> node)              {
    expr_ += "-";
    accept(node->get_child());
}
void RTreeVisitor::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// 制約階層定義演算子
void RTreeVisitor::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// 時相演算子
void RTreeVisitor::visit(boost::shared_ptr<Always> node)                {
    expr_ += "{Always, ";
    accept(node->get_child());
    expr_ += "}";
}
  
// 微分 UNARYNODE df(x,t)と表記
void RTreeVisitor::visit(boost::shared_ptr<Differential> node)          {
    expr_ += "df(";
    accept(node->get_child());
    expr_ += ",t)";
}

// 左極限 UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Previous> node)              {
    expr_ += "prev(";
    accept(node->get_child());
    expr_ += ")";
}
  
// 変数 FactorNode
void RTreeVisitor::visit(boost::shared_ptr<Variable> node)              {
    expr_ += node->get_name();
}

// 数字 FactorNode
void RTreeVisitor::visit(boost::shared_ptr<Number> node)                {
    expr_ += node->get_number();
}

} //namespace hydla
} //namespace parse_tree
