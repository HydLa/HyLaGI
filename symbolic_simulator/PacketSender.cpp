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
  in_prev_(false)
{}

PacketSender::~PacketSender()
{}


// Ask§–ñ
void PacketSender::visit(boost::shared_ptr<Ask> node)                   
{
  std::cout << "guard:";
  //ml_.put_function("ask", 2);
  accept(node->get_guard());
  
  //std::cout << " => ";
  //accept(node->get_child());
  std::cout << std::endl;
}

// Tell§–ñ
void PacketSender::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.put_function("tell", 1);

  accept(node->get_child());
  std::cout << std::endl;
}

// ”äŠr‰‰Zq
void PacketSender::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.put_function("Equal", 2);
        
  accept(node->get_lhs());
  std::cout << "=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.put_function("UnEqual", 2);

  accept(node->get_lhs());
  std::cout << "!=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<Less> node)                  
{
  ml_.put_function("Less", 2);

  accept(node->get_lhs());
  std::cout << "<";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.put_function("LessEqual", 2);

  accept(node->get_lhs());    
  std::cout << "<=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<Greater> node)               
{
  ml_.put_function("Greater", 2);
    
  accept(node->get_lhs());
  std::cout << ">";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.put_function("GreaterEqual", 2);
    
  accept(node->get_lhs());
  std::cout << ">=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

// ˜_—‰‰Zq
void PacketSender::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ml_.put_function("And", 2);

  accept(node->get_lhs());
  std::cout << " & ";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.put_function("Or", 2);
    
  accept(node->get_lhs());
  accept(node->get_rhs());
}
  
// Zp“ñ€‰‰Zq
void PacketSender::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.put_function("Plus", 2);
    
  accept(node->get_lhs());
  std::cout << "+";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.put_function("Subtract", 2);
    
  accept(node->get_lhs());
  std::cout << "-";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Times> node)                 
{
  ml_.put_function("Times", 2);

  accept(node->get_lhs());
  std::cout << "*";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Divide> node)                
{
  ml_.put_function("Divide", 2);
    
  accept(node->get_lhs());
  std::cout << "/";
  accept(node->get_rhs());
}
  
// Zp’P€‰‰Zq
void PacketSender::visit(boost::shared_ptr<Negative> node)              
{       
  ml_.put_function("Minus", 1);
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

  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
  ml_.MLPutArgCount(1);      // this 1 is for the 'f'
  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
  ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
  ml_.put_symbol("Derivative");
  ml_.MLPutInteger(1);

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
    ml_.put_function("prev", 1);
    std::cout << "prev[";
    if(differential_count_ > 0)
    {
      ml_.put_symbol(("usrVar" + node->get_name()).c_str());
      vars_.insert(std::make_pair("usrVar" + node->get_name(), -1*(differential_count_ +1)));
      std::cout << "Derivative[" << differential_count_ << "][" << node->get_name().c_str() << "]";
    }
    else
    {
      ml_.put_symbol(("usrVar" + node->get_name()).c_str());
      vars_.insert(std::make_pair("usrVar" + node->get_name(), -1));
      std::cout << node->get_name().c_str();
    }
    std::cout << "]";
  }
  else if(differential_count_ > 0){
    ml_.put_symbol(("usrVar" + node->get_name()).c_str());
    vars_.insert(std::make_pair("usrVar" + node->get_name(), differential_count_));
    std::cout << "Derivative[" << differential_count_ << "][" << node->get_name().c_str() << "]";
  }
  else
  {
    ml_.put_symbol(("usrVar" + node->get_name()).c_str());
    vars_.insert(std::make_pair("usrVar" + node->get_name(), 0));
    std::cout << node->get_name().c_str();
  }

/*
  if(in_differential_equality_){
    if(in_differential_)
    {
      ml_.put_symbol("t");
    }
    else
    {
      ml_.put_symbol("t");
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
  ml_.put_function("List", var_num);
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
      ml_.put_function("prev", 1);
      std::cout << "prev[";
      value += 1;
      value *= -1;
      if(value != 0)
      {
        ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
        ml_.MLPutArgCount(1);      // this 1 is for the 'f'
        ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
        ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
        ml_.put_symbol("Derivative");
        ml_.MLPutInteger(value);
        ml_.put_symbol(sym);
        //ml_.put_symbol("t");
        std::cout << "Derivative[" << value << "][" << sym << "]";
      }
      else
      {
        ml_.put_symbol(sym);
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
      ml_.put_symbol("Derivative");
      ml_.MLPutInteger(value);
      ml_.put_symbol(sym);
      //ml_.put_symbol("t");
      std::cout << "Derivative[" << value << "][" << sym << "]";
    }
    else
    {
      ml_.put_symbol(sym);
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
  std::cout << "Constraint store:";
  int cs_size = constraint_store.size();
  if(cs_size < 1)
  {
    ml_.put_function("List", 0);
    std::cout << "no Constraints" << std::endl;
    return;
  }
  std::cout << std::endl;
  std::cout << "----------------------------" << std::endl;
  std::cout << constraint_store;
  std::cout << "----------------------------" << std::endl;

  ml_.put_function("List", cs_size);

  variable_map_t::const_iterator cs_it = constraint_store.begin();
  variable_map_t::const_iterator cs_end = constraint_store.end();
  for(; cs_it!=cs_end; ++cs_it)
  {
    SymbolicVariable variable = (*cs_it).first;
    SymbolicValue value = (*cs_it).second;
    ml_.put_function("Equal", 2);

    // •Ï”–¼
/*
    if(variable.previous == true)
    {
      ml_.put_function("prev", 1);
    }
*/
    if(variable.derivative_count > 0)
    {
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.put_symbol("Derivative");
      ml_.MLPutInteger(variable.derivative_count);
    }
    ml_.put_symbol(variable.name.c_str());

    // •Ï”‚Ì’l
    if(value.rational == true)
    {
      ml_.put_function("Rational", 2);
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
