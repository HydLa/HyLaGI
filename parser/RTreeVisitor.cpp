#include "RTreeVisitor.h"

#include "Logger.h"
#include <assert.h>
#include <iostream>
//#include <string>

using namespace hydla::parse_tree;

namespace hydla {
namespace parse_tree {

//ˆø”‚Íƒ_ƒ~[
RTreeVisitor::RTreeVisitor(int a)
{
	expr_ = "";
	std::cout << "rtv was created" << std::endl;
}

RTreeVisitor::~RTreeVisitor()
{
		std::cout << "rtv was destroyed" << std::endl;
}

//c++“®ì—ûK
void RTreeVisitor::sandbox(){
//	std::cout << "sandbox called" << std::endl;
}

// §–ñ®‚ğl‚ª“Ç‚ß‚éstring‚É‚µ‚Ä•Ô‚·
std::string RTreeVisitor::get_expr(const node_sptr& node){
	accept(node);
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
// ’è‹`
void RTreeVisitor::visit(boost::shared_ptr<ConstraintDefinition> node)  {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<ProgramDefinition> node)     {assert(0);}

// ŒÄ‚Ño‚µ
void RTreeVisitor::visit(boost::shared_ptr<ConstraintCaller> node)      {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<ProgramCaller> node)         {assert(0);}

// §–ñ®
void RTreeVisitor::visit(boost::shared_ptr<Constraint> node)            {assert(0);}

// Ask§–ñ BinaryNode
void RTreeVisitor::visit(boost::shared_ptr<Ask> node){
//	expr_ += "Ask: ";
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}

// Tell§–ñ UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Tell> node)                  {
//	expr_ += "(tell:) ";
	expr_ += node->get_node_type_presymbol();
	accept(node->get_child());
	expr_ += node->get_node_type_presymbol();
}

// ”äŠr‰‰Zq ASYMMETRIC_BINARY_NODE
void RTreeVisitor::visit(boost::shared_ptr<Equal> node)                 {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<UnEqual> node)               {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<Less> node)                  {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<LessEqual> node)             {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<Greater> node)               {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<GreaterEqual> node)          {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}

// ˜_—‰‰Zq
void RTreeVisitor::visit(boost::shared_ptr<LogicalAnd> node)            {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<LogicalOr> node)             {assert(0);}
  
// Zp“ñ€‰‰Zq BINARY_NODE
void RTreeVisitor::visit(boost::shared_ptr<Plus> node)                  {
	expr_ += "(";
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
	expr_ += ")";
}
void RTreeVisitor::visit(boost::shared_ptr<Subtract> node)              {
	expr_ += "(";
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
	expr_ += ")";
}
void RTreeVisitor::visit(boost::shared_ptr<Times> node)                 {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
void RTreeVisitor::visit(boost::shared_ptr<Divide> node)                {
	accept(node->get_lhs());
	expr_ += node->get_node_type_symbol();
	accept(node->get_rhs());
}
  
// Zp’P€‰‰Zq UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Negative> node)              {
    expr_ += node->get_node_type_presymbol();
    accept(node->get_child());
    expr_ += node->get_node_type_postsymbol();
}
void RTreeVisitor::visit(boost::shared_ptr<Positive> node)              {assert(0);}
  
// §–ñŠK‘w’è‹`‰‰Zq
void RTreeVisitor::visit(boost::shared_ptr<Weaker> node)                {assert(0);}
void RTreeVisitor::visit(boost::shared_ptr<Parallel> node)              {assert(0);}

// ‘Š‰‰Zq
void RTreeVisitor::visit(boost::shared_ptr<Always> node)                {assert(0);}
  
// ”÷•ª UNARYNODE df(x,t)‚Æ•\‹L
void RTreeVisitor::visit(boost::shared_ptr<Differential> node)          {
    expr_ += "df(";
    accept(node->get_child());
    expr_ += ",t)";
}

// ¶‹ÉŒÀ UNARYNODE
void RTreeVisitor::visit(boost::shared_ptr<Previous> node)              {
    expr_ += "prev(";
    accept(node->get_child());
    expr_ += ")";
}
  
// •Ï” FactorNode
void RTreeVisitor::visit(boost::shared_ptr<Variable> node)              {
    expr_ += node->get_name();
}

// ”š FactorNode
void RTreeVisitor::visit(boost::shared_ptr<Number> node)                {
    expr_ += node->get_number();
}

} //namespace hydla
} //namespace parse_tree
