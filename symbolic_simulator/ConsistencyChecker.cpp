#include "ConsistencyChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {


ConsistencyChecker::ConsistencyChecker(MathLink& ml) :
  ml_(ml),
  in_differential_equality_(false),
  in_differential_(false)
{}

ConsistencyChecker::~ConsistencyChecker()
{}

// TellêßñÒ
void ConsistencyChecker::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.MLPutFunction("tell", 1);

  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << std::endl;
}

// î‰ärââéZéq
void ConsistencyChecker::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.MLPutFunction("Equal", 2);
        
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.MLPutFunction("UnEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "!=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<Less> node)                  
{
  ml_.MLPutFunction("Less", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "<";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.MLPutFunction("LessEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "<=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<Greater> node)               
{
  ml_.MLPutFunction("Greater", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << ">";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void ConsistencyChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.MLPutFunction("GreaterEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << ">=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

// ò_óùââéZéq
void ConsistencyChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
  //ml_.MLPutFunction("And, 2);

  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.MLPutFunction("Or", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// éZèpìÒçÄââéZéq
void ConsistencyChecker::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.MLPutFunction("Plus", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "+";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.MLPutFunction("Subtract", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "-";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Times> node)                 
{
  ml_.MLPutFunction("Times", 2);

  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "*";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Divide> node)                
{
  ml_.MLPutFunction("Divide", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "/";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// éZèpíPçÄââéZéq
void ConsistencyChecker::visit(boost::shared_ptr<Negative> node)              
{       
  ml_.MLPutFunction("Minus", 1);
  std::cout << "-";

  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
}

void ConsistencyChecker::visit(boost::shared_ptr<Positive> node)              
{
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
}

// î˜ï™
void ConsistencyChecker::visit(boost::shared_ptr<Differential> node)          
{

  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
  ml_.MLPutArgCount(1);      // this 1 is for the 'f'
  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
  ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
  ml_.MLPutSymbol("Derivative");
  ml_.MLPutInteger(1);


  in_differential_equality_ = true;
  in_differential_ = true;
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << "'";
  if(in_differential_){
    //std::cout << "[t]"; // ht[t]' ÇÃÇÊÇ§Ç…Ç»ÇÈÇÃÇñhÇÆÇΩÇﬂ
  }

  in_differential_ = false;
}

// ç∂ã…å¿
void ConsistencyChecker::visit(boost::shared_ptr<Previous> node)              
{
  //ml_.MLPutFunction("prev", 1);
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << "-";
}
  
// ïœêî
void ConsistencyChecker::visit(boost::shared_ptr<Variable> node)              
{
  ml_.MLPutSymbol(node->get_name().c_str());
  if(in_differential_){
    vars_.insert(std::pair<std::string, int>(node->get_name() + "'", 1));
  }
  else {
    vars_.insert(std::pair<std::string, int>(node->get_name(), 0));
  }
  std::cout << node->get_name().c_str();
  if(in_differential_equality_){
    if(in_differential_){
      //ml_.MLPutSymbol("t");
    }else{
      //ml_.MLPutSymbol("t");
      //std::cout << "[t]";
    }
  } else {
    //ml_.MLPutInteger(0);
    //std::cout << "[0]";
  }
}

// êîéö
void ConsistencyChecker::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  std::cout << node->get_number().c_str();
}


bool ConsistencyChecker::is_consistent(std::vector<boost::shared_ptr<hydla::parse_tree::Tell> >& tells)
{

/*
  ml_.MLPutFunction("List", 3);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(2);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("y");
  ml_.MLPutInteger(2);

  ml_.MLPutFunction("List", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLEndPacket();
*/

  int tells_size = tells.size();
  // isConsistent[expr, vars]ÇìnÇµÇΩÇ¢
  ml_.MLPutFunction("isConsistent", 2);
  ml_.MLPutFunction("List", tells_size);

  // tellêßñÒÇÃèWçáÇ©ÇÁexprÇìæÇƒMathematicaÇ…ìnÇ∑
  for(int i=0; i<tells_size; i++){
    visit(tells[i]);
  }


  // varsÇìnÇ∑
  int var_num = vars_.size();
  ml_.MLPutFunction("List", var_num);
  std::map<std::string, int>::iterator it = vars_.begin();
  const char* sym;
  std::cout << "vars: ";
  while(it!=vars_.end()){
    sym = ((*it).first).c_str();
    if((*it).second==0){
      ml_.MLPutSymbol(sym);
    }else {
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.MLPutSymbol("Derivative");
      ml_.MLPutInteger(1);
      ml_.MLPutSymbol(sym);
      //ml_.MLPutSymbol("t");
    }
    std::cout << sym << " ";
    it++;
  }

  // ml_.MLEndPacket();

  // óvëfÇÃëSçÌèú
  vars_.clear();

  std::cout << std::endl;

/*
// ï‘Ç¡ÇƒÇ≠ÇÈÉpÉPÉbÉgÇâêÕ
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  ml_.MLGetInteger(&num);
  //std::cout << "#num:" << num << std::endl;
  
  // MathematicaÇ©ÇÁ1ÅiTrueÇï\Ç∑ÅjÇ™ï‘ÇÍÇŒtrueÇÅA0ÅiFalseÇï\Ç∑ÅjÇ™ï‘ÇÍÇŒfalseÇï‘Ç∑
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
