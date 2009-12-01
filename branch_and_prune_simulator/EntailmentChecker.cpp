#include "EntailmentChecker.h"

#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace bp_simulator {


EntailmentChecker::EntailmentChecker() :
  in_differential_equality_(false),
  in_differential_(false)
{}

EntailmentChecker::~EntailmentChecker()
{}

// Tell§–ñ
void EntailmentChecker::visit(boost::shared_ptr<Tell> node)                  
{
  ////ml_.MLPutFunction("tell", 1);

  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
  //std::cout << std::endl;
}

// ”äŠr‰‰Zq
void EntailmentChecker::visit(boost::shared_ptr<Equal> node)                 
{
  //ml_.MLPutFunction("Equal", 2);
  //      
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<UnEqual> node)               
{
  //ml_.MLPutFunction("UnEqual", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "!=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<Less> node)                  
{
  //ml_.MLPutFunction("Less", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "<";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<LessEqual> node)             
{
  //ml_.MLPutFunction("LessEqual", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "<=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<Greater> node)               
{
  //ml_.MLPutFunction("Greater", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << ">";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
  //ml_.MLPutFunction("GreaterEqual", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << ">=";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  //in_differential_equality_ = false;
}

// ˜_—‰‰Zq
void EntailmentChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ////ml_.MLPutFunction("And, 2);

  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<LogicalOr> node)             
{
  ////ml_.MLPutFunction("Or", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// Zp“ñ€‰‰Zq
void EntailmentChecker::visit(boost::shared_ptr<Plus> node)                  
{
  //ml_.MLPutFunction("Plus", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "+";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Subtract> node)              
{
  //ml_.MLPutFunction("Subtract", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "-";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Times> node)           
{
  //ml_.MLPutFunction("Times", 2);

  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "*";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Divide> node)                
{
  //ml_.MLPutFunction("Divide", 2);
  //  
  //node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  //std::cout << "/";
  //node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// Zp’P€‰‰Zq
void EntailmentChecker::visit(boost::shared_ptr<Negative> node)
{
  //ml_.MLPutFunction("Minus", 1);
  //std::cout << "-";

  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Positive> node)              
{
  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
}

// ”÷•ª
void EntailmentChecker::visit(boost::shared_ptr<Differential> node)          
{
  //ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
  //ml_.MLPutArgCount(1);      // this 1 is for the 'f'
  //ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
  //ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
  //ml_.MLPutSymbol("Derivative");
  //ml_.MLPutInteger(1);


  //in_differential_equality_ = true;
  //in_differential_ = true;
  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
  //std::cout << "'";
  //if(in_differential_){
  //  //std::cout << "[t]"; // ht[t]' ‚Ì‚æ‚¤‚É‚È‚é‚Ì‚ğ–h‚®‚½‚ß
  //}

  //in_differential_ = false;
}

// ¶‹ÉŒÀ
void EntailmentChecker::visit(boost::shared_ptr<Previous> node)              
{
  ////ml_.MLPutFunction("prev", 1);
  //node_sptr chnode(node->get_child_node());
  //chnode->accept(chnode, this);
  //std::cout << "-";
}
  
// •Ï”
void EntailmentChecker::visit(boost::shared_ptr<Variable> node)              
{
  //ml_.MLPutSymbol(node->get_name().c_str());
  //if(in_differential_){
  //  vars_.insert(std::pair<std::string, int>(node->get_name() + "'", 1));
  //}
  //else {
  //  vars_.insert(std::pair<std::string, int>(node->get_name(), 0));
  //}
  //std::cout << node->get_name().c_str();
  //if(in_differential_equality_){
  //  if(in_differential_){
  //    //ml_.MLPutSymbol("t");
  //  }else{
  //    //ml_.MLPutSymbol("t");
  //    //std::cout << "[t]";
  //  }
  //} else {
  //  //ml_.MLPutInteger(0);
  //  //std::cout << "[0]";
  //}
}

// ”š
void EntailmentChecker::visit(boost::shared_ptr<Number> node)                
{
  //ml_.MLPutInteger(atoi(node->get_number().c_str()));
  //std::cout << node->get_number().c_str();
}

/**
 * collected_tells‚©‚çnegative_ask‚ÌƒK[ƒhğŒ‚ªentail‚³‚ê‚é‚Ç‚¤‚©’²‚×‚é
 * TRUE‚È‚çcollected_tells‚Éask§–ñ‚ÌŒãŒ‚ğ’Ç‰Á‚·‚é
 * 
 * Input:
 *  negative_ask ‚Ü‚¾“WŠJ‚³‚ê‚Ä‚¢‚È‚¢ask§–ñ
 *  collected_tells tell§–ñ‚ÌƒŠƒXƒgi“WŠJ‚³‚ê‚½ask§–ñ‚Ìu=>v‚Ì‰E•Ó‚ª‚±‚±‚É’Ç‰Á‚³‚ê‚éj
 * Output:
 *  entail‚³‚ê‚é‚©‚Ç‚¤‚© {TRUE, UNKNOWN, FALSE}
 */
Trivalent EntailmentChecker::check_entailment(
  boost::shared_ptr<Ask> negative_ask,
  TellCollector::tells_t& collected_tells)
{
  // collected_tells‚ğrp_constraint‰» -> S
  // negative_ask‘OŒ‚ğrp_constraint‰» -> g
  // !(negative_ask‘OŒ)‚ğrp_constraint‰» -> ng
  // if []solve(X, S & g, D) == empty -> FALSE
  // elseif []solve(X, S & ng, D) == empty -> TRUE
  // else -> UNKNOWN
  return TRUE;
}

} //namespace bp_simulator
} // namespace hydla
