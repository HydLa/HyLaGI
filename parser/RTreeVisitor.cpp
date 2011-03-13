#include "RTreeVisitor.h"

#include "Logger.h"
#include <assert.h>
#include <iostream>
//#include <string>

using namespace hydla::parse_tree;

namespace hydla {
namespace parse_tree {

//�����̓_�~�[
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

//c++������K
void RTreeVisitor::sandbox(){
//	std::cout << "sandbox called" << std::endl;
}

// ���񎮂�l���ǂ߂�string�ɂ��ĕԂ�
std::string RTreeVisitor::get_expr(const node_sptr& node){
	accept(node);
	std::string ret = expr_;
	expr_ ="";
	return ret;
}

//accept���O���ōs���ꍇ
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
// ��`
void RTreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {assert(0);}

// �Ăяo��
void RTreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {assert(0);}

// ����
void RTreeVisitor::visit(boost::shared_ptr<Constraint> node)            {
	if(expr_==""){
		expr_ += "{Constraint, ";
	}else{
		expr_ += ", {Constraint, ";
	}
	accept(node->get_child());
	expr_ += "}";
}

// Ask���� BinaryNode
/*
* ��
void RTreeVisitor::visit(boost::shared_ptr<Ask> node){
//	expr_ += "Ask: ";
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
*/
//ms�p
void RTreeVisitor::visit(boost::shared_ptr<Ask> node){
	expr_ += "{Ask, ";
	accept(node->get_lhs());
	expr_ += ", ";	
	accept(node->get_rhs());
	expr_ += "}";
}

// Tell���� UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Tell> node)                  {
// �� reduce_output�ɌĂяo���ꂽ�Ƃ�
	if(caller_=="reduce_output"){
		accept(node->get_child());
	}else{
		expr_ += "{Tell, ";
		accept(node->get_child());
		expr_ += "}";
	}
}

// ��r���Z�q ASYMMETRIC_BINARY_NODE
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

// �_�����Z�q
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
  
// �Z�p�񍀉��Z�q BINARY_NODE
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
  
// �Z�p�P�����Z�q UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Negative> node)              {
    expr_ += "-";
    accept(node->get_child());
}
void RTreeVisitor::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// ����K�w��`���Z�q
void RTreeVisitor::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// �������Z�q
void RTreeVisitor::visit(boost::shared_ptr<Always> node)                {
    expr_ += "{Always, ";
    accept(node->get_child());
    expr_ += "}";
}
  
// ���� UNARYNODE df(x,t)�ƕ\�L
void RTreeVisitor::visit(boost::shared_ptr<Differential> node)          {
    expr_ += "df(";
    accept(node->get_child());
    expr_ += ",t)";
}

// ���Ɍ� UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Previous> node)              {
    expr_ += "prev(";
    accept(node->get_child());
    expr_ += ")";
}
  
// �ϐ� FactorNode
void RTreeVisitor::visit(boost::shared_ptr<Variable> node)              {
    expr_ += node->get_name();
}

// ���� FactorNode
void RTreeVisitor::visit(boost::shared_ptr<Number> node)                {
    expr_ += node->get_number();
}

} //namespace hydla
} //namespace parse_tree
