#include "ConsistencyChecker.h"
#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


ConsistencyChecker::ConsistencyChecker(MathLink& ml) :
  ml_(ml),
  in_differential_equality_(false),
  differential_count_(0),
  in_prev_(false)
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
  ml_.MLPutFunction("And", 2);

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

  differential_count_++;
  in_differential_equality_ = true;
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);

/*
  if(in_differential_){
    //std::cout << "[t]"; // ht[t]' ÇÃÇÊÇ§Ç…Ç»ÇÈÇÃÇñhÇÆÇΩÇﬂ
  }
*/

  differential_count_--;
}

// ç∂ã…å¿
void ConsistencyChecker::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);

  in_prev_ = false;
}
  
// ïœêî
void ConsistencyChecker::visit(boost::shared_ptr<Variable> node)              
{

  if(in_prev_)
  {
    ml_.MLPutFunction("prev", 1);
    std::cout << "prev[";
    if(differential_count_ > 0)
    {
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.MLPutSymbol("Derivative");
      ml_.MLPutInteger(differential_count_);

      std::cout << "Derivative[1][";
      ml_.MLPutSymbol(("usrVar" + node->get_name()).c_str());
      vars_.insert(std::pair<std::string, int>("usrVar" + node->get_name(), -1*(differential_count_ +1)));
      std::cout << node->get_name().c_str() << "]";
    }
    else
    {
      ml_.MLPutSymbol(("usrVar" + node->get_name()).c_str());
      vars_.insert(std::pair<std::string, int>("usrVar" + node->get_name(), -1));
      std::cout << node->get_name().c_str();
    }
    std::cout << "]";
  }
  else if(differential_count_ > 0){
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
    ml_.MLPutArgCount(1);      // this 1 is for the 'f'
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
    ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
    ml_.MLPutSymbol("Derivative");
    ml_.MLPutInteger(differential_count_);

    std::cout << "Derivative[1][";
    ml_.MLPutSymbol(("usrVar" + node->get_name()).c_str());
    vars_.insert(std::pair<std::string, int>("usrVar" + node->get_name(), differential_count_));
    std::cout << node->get_name().c_str() << "]";
  }
  else
  {
    ml_.MLPutSymbol(("usrVar" + node->get_name()).c_str());
    vars_.insert(std::pair<std::string, int>("usrVar" + node->get_name(), 0));
    std::cout << node->get_name().c_str();
  }

/*
  if(in_differential_equality_){
    if(in_differential_)
    {
      ml_.MLPutSymbol("t");
    }
    else
    {
      ml_.MLPutSymbol("t");
      std::cout << "[t]";
    }
  }
  else
  {
    ml_.MLPutInteger(0);
    std::cout << "[0]";
  }
*/

}

// êîéö
void ConsistencyChecker::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  std::cout << node->get_number().c_str();
}


bool ConsistencyChecker::is_consistent(collected_tells_t& collected_tells)
{

/*
  ml_.MLPutFunction("isConsistent", 2);
  ml_.MLPutFunction("List", 3);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(2);
  ml_.MLPutFunction("Equal", 2);
  ml_.MLPutSymbol("y");
  ml_.MLPutInteger(1);

  ml_.MLPutFunction("List", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutSymbol("y");
  ml_.MLEndPacket();
*/


  // isConsistent[expr, vars]ÇìnÇµÇΩÇ¢
  ml_.MLPutFunction("isConsistent", 2);

  // tellêßñÒÇÃèWçáÇ©ÇÁexprÇìæÇƒMathematicaÇ…ìnÇ∑
  int tells_size = collected_tells.size();
  ml_.MLPutFunction("List", tells_size);
  collected_tells_t::iterator tells_it = collected_tells.begin();
  while((tells_it) != collected_tells.end())
  {
    visit((*tells_it));
    tells_it++;
  }

  // varsÇìnÇ∑
  int var_num = vars_.size();
  ml_.MLPutFunction("List", var_num);
  std::multimap<std::string, int>::iterator vars_it = vars_.begin();
  const char* sym;
  int value;
  std::cout << "vars: ";
  while(vars_it!=vars_.end())
  {
    sym = ((*vars_it).first).c_str();
    value = (*vars_it).second;
    if(value < 0)
    {
      ml_.MLPutFunction("prev", 1);
      std::cout << "prev[";
      value += 1;
      value *= -1;
      if(value != 0)
      {
        ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
        ml_.MLPutArgCount(1);      // this 1 is for the 'f'
        ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
        ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
        ml_.MLPutSymbol("Derivative");
        ml_.MLPutInteger(value);
        ml_.MLPutSymbol(sym);
        //ml_.MLPutSymbol("t");
        std::cout << "Derivative[" << value << "][" << sym << "]";
      }
      else
      {
        ml_.MLPutSymbol(sym);
        std::cout << sym;
      }
      std::cout << "]";
    }
    else if(value != 0)
    {
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.MLPutSymbol("Derivative");
      ml_.MLPutInteger(value);
      ml_.MLPutSymbol(sym);
      //ml_.MLPutSymbol("t");
      std::cout << "Derivative[" << value << "][" << sym << "]";
    }
    else
    {
      ml_.MLPutSymbol(sym);
      std::cout << sym;
    }
    std::cout << " ";
    vars_it++;
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
  std::cout << "ConsistencyChecker#num:" << num << std::endl;
  
  // MathematicaÇ©ÇÁ1ÅiTrueÇï\Ç∑ÅjÇ™ï‘ÇÍÇŒtrueÇÅA0ÅiFalseÇï\Ç∑ÅjÇ™ï‘ÇÍÇŒfalseÇï‘Ç∑
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
