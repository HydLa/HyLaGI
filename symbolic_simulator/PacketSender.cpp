#include "PacketSender.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


PacketSender::PacketSender(MathLink& ml) :
  ml_(ml),
  in_differential_equality_(false),
  differential_count_(0),
  in_prev_(false),
  in_guard_(false)
{}

PacketSender::~PacketSender()
{}


// Ask§–ñ
void PacketSender::visit(boost::shared_ptr<Ask> node)                   
{
  std::cout << "guard:";
  //ml_.MLPutFunction("ask", 2);
  in_guard_ = true;
  accept(node->get_guard());
  
  in_guard_ = false;
  //std::cout << " => ";
  //accept(node->get_child());
  std::cout << std::endl;
}

// Tell§–ñ
void PacketSender::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.MLPutFunction("tell", 1);

  accept(node->get_child());
  std::cout << std::endl;
}

// ”äŠr‰‰Zq
void PacketSender::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.MLPutFunction("Equal", 2);
        
  accept(node->get_lhs());
  std::cout << "=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.MLPutFunction("UnEqual", 2);

  accept(node->get_lhs());
  std::cout << "!=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<Less> node)                  
{
  ml_.MLPutFunction("Less", 2);

  accept(node->get_lhs());
  std::cout << "<";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.MLPutFunction("LessEqual", 2);

  accept(node->get_lhs());    
  std::cout << "<=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<Greater> node)               
{
  ml_.MLPutFunction("Greater", 2);
    
  accept(node->get_lhs());
  std::cout << ">";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.MLPutFunction("GreaterEqual", 2);
    
  accept(node->get_lhs());
  std::cout << ">=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

// ˜_—‰‰Zq
void PacketSender::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ml_.MLPutFunction("And", 2);

  accept(node->get_lhs());
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.MLPutFunction("Or", 2);
    
  accept(node->get_lhs());
  accept(node->get_rhs());
}
  
// Zp“ñ€‰‰Zq
void PacketSender::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.MLPutFunction("Plus", 2);
    
  accept(node->get_lhs());
  std::cout << "+";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.MLPutFunction("Subtract", 2);
    
  accept(node->get_lhs());
  std::cout << "-";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Times> node)                 
{
  ml_.MLPutFunction("Times", 2);

  accept(node->get_lhs());
  std::cout << "*";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Divide> node)                
{
  ml_.MLPutFunction("Divide", 2);
    
  accept(node->get_lhs());
  std::cout << "/";
  accept(node->get_rhs());
}
  
// Zp’P€‰‰Zq
void PacketSender::visit(boost::shared_ptr<Negative> node)              
{       
  ml_.MLPutFunction("Minus", 1);
  std::cout << "-";

  accept(node->get_child());
}

void PacketSender::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

// ”÷•ª
void PacketSender::visit(boost::shared_ptr<Differential> node)          
{

  differential_count_++;
  in_differential_equality_ = true;
  accept(node->get_child());

/*
  if(in_differential_){
    //std::cout << "[t]"; // ht[t]' ‚Ì‚æ‚¤‚É‚È‚é‚Ì‚ğ–h‚®‚½‚ß
  }
*/

  differential_count_--;
}

// ¶‹ÉŒÀ
void PacketSender::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());

  in_prev_ = false;
}
  
// •Ï”
void PacketSender::visit(boost::shared_ptr<Variable> node)              
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
      vars_.insert(std::make_pair("usrVar" + node->get_name(), -1*(differential_count_ +1)));
      std::cout << node->get_name().c_str() << "]";
    }
    else
    {
      ml_.MLPutSymbol(("usrVar" + node->get_name()).c_str());
      vars_.insert(std::make_pair("usrVar" + node->get_name(), -1));
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
    vars_.insert(std::make_pair("usrVar" + node->get_name(), differential_count_));
    std::cout << node->get_name().c_str() << "]";
  }
  else
  {
    ml_.MLPutSymbol(("usrVar" + node->get_name()).c_str());
    vars_.insert(std::make_pair("usrVar" + node->get_name(), 0));
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

// ”š
void PacketSender::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  std::cout << node->get_number().c_str();
}


// •Ï”‚Ìˆê——‚ğ‘—M
void PacketSender::put_vars()
{
  int var_num = vars_.size();
  ml_.MLPutFunction("List", var_num);
  std::set<std::pair<std::string, int> >::iterator vars_it = vars_.begin();
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

  // —v‘f‚Ì‘Síœ
  vars_.clear();

  std::cout << std::endl;
}

// §–ñƒXƒgƒA‚Ì’†g‚ğ•ªÍ‚µ‚Ä‘—M
void PacketSender::put_cs(ConstraintStore constraint_store)
{
  std::cout << "Constraint store: ";
  // vlt  cs_map = constraint_store.variables;
  int cs_size = constraint_store.size();
  if(cs_size < 1)
  {
    ml_.MLPutFunction("List", 0);
    std::cout << "no Constraint store" << std::endl;
    return;
  }
  ml_.MLPutFunction("List", cs_size);

  variable_map_t::const_iterator cs_it = constraint_store.begin();
  variable_map_t::const_iterator cs_end = constraint_store.end();
  for(; cs_it!=cs_end; ++cs_it)
  {
    SymbolicVariable variable = (*cs_it).first;
    SymbolicValue value = (*cs_it).second;
    ml_.MLPutFunction("Equal", 2);

    // •Ï”–¼
    //if(variable.previous == true)
    {
      ml_.MLPutFunction("prev", 1);
      std::cout << "prev[";
    }
    if(variable.derivative_count > 0)
    {
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.MLPutSymbol("Derivative");
      ml_.MLPutInteger(variable.derivative_count);
      std::cout << "Derivative[" << variable.derivative_count << "][";
    }
    ml_.MLPutSymbol(variable.name.c_str());
    std::cout << variable.name;
    if(variable.derivative_count > 0)
    {
      std::cout << "]";
    }
    //if(variable.previous == true)
    {
      std::cout << "]";
    }
    std::cout << "=";

    // •Ï”‚Ì’l
    if(value.rational == true)
    {
      ml_.MLPutFunction("Rational", 2);
      ml_.MLPutInteger(value.numerator);
      ml_.MLPutInteger(value.denominator);
    }
    else 
    {
      ml_.MLPutInteger(value.numerator);
    }
  }
}


} //namespace symbolic_simulator
} // namespace hydla
