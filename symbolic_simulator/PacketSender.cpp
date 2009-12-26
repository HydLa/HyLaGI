#include "PacketSender.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


PacketSender::PacketSender(MathLink& ml, bool debug_mode) :
  ml_(ml),
  in_differential_equality_(false),
  differential_count_(0),
  in_prev_(false),
  debug_mode_(debug_mode)
{}

PacketSender::~PacketSender()
{}


// Ask制約
void PacketSender::visit(boost::shared_ptr<Ask> node)                   
{
  if(debug_mode_) std::cout << "guard:";
  //ml_.put_function("ask", 2);
  accept(node->get_guard());
    
  //std::cout << " => ";
  //accept(node->get_child());
  if(debug_mode_) std::cout << std::endl;
}

// Tell制約
void PacketSender::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.put_function("tell", 1);

  accept(node->get_child());
  if(debug_mode_) std::cout << std::endl;
}

// 比較演算子
void PacketSender::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.put_function("Equal", 2);
        
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.put_function("UnEqual", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << "!=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<Less> node)                  
{
  ml_.put_function("Less", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << "<";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.put_function("LessEqual", 2);

  accept(node->get_lhs());    
  if(debug_mode_) std::cout << "<=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<Greater> node)               
{
  ml_.put_function("Greater", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << ">";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

void PacketSender::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.put_function("GreaterEqual", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << ">=";
  accept(node->get_rhs());
  in_differential_equality_ = false;
}

// 論理演算子
void PacketSender::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ml_.put_function("And", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << " & ";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.put_function("Or", 2);
    
  accept(node->get_lhs());
  accept(node->get_rhs());
}
  
// 算術二項演算子
void PacketSender::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.put_function("Plus", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "+";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.put_function("Subtract", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "-";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Times> node)                 
{
  ml_.put_function("Times", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << "*";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Divide> node)                
{
  ml_.put_function("Divide", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "/";
  accept(node->get_rhs());
}
  
// 算術単項演算子
void PacketSender::visit(boost::shared_ptr<Negative> node)              
{       
  ml_.put_function("Minus", 1);
  if(debug_mode_) std::cout << "-";

  accept(node->get_child());
}

void PacketSender::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

// 微分
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
    //std::cout << "[t]"; // ht[t]' のようになるのを防ぐため
  }
*/

  differential_count_--;
}

// 左極限
void PacketSender::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}
  
// 変数
void PacketSender::visit(boost::shared_ptr<Variable> node)              
{
  if(in_prev_)
  {
    ml_.put_function("prev", 1);
    if(debug_mode_) std::cout << "prev[";
  }

  ml_.put_symbol(("usrVar" + node->get_name()).c_str());
  if(in_prev_){
    vars_.insert(std::make_pair("usrVar" + node->get_name(), -1*(differential_count_ +1)));
  }else{
    vars_.insert(std::make_pair("usrVar" + node->get_name(), differential_count_ + 1));
  }

  if(debug_mode_) {
    if(differential_count_ > 0){
      std::cout << "Derivative[" << differential_count_ << "][" << node->get_name().c_str() << "]";
    }else{
      std::cout << node->get_name().c_str();
    }
    if(in_prev_) std::cout << "]";
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

// 数字
void PacketSender::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  if(debug_mode_) std::cout << node->get_number().c_str();
}


// 変数の一覧を送信
void PacketSender::put_vars()
{
  int var_num = vars_.size();
  ml_.put_function("List", var_num);
  std::set<std::pair<std::string, int> >::iterator vars_it = vars_.begin();
  const char* sym;
  int value;
  if(debug_mode_) std::cout << "vars: ";
  while(vars_it!=vars_.end())
  {
    sym = ((*vars_it).first).c_str();
    value = (*vars_it).second;
    int prevflag = 0;
    if(value < 0)
    {
      prevflag = 1;
      ml_.put_function("prev", 1);
      if(debug_mode_) std::cout << "prev[";
      value *= -1;
    }
    value -= 1;
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
      if(debug_mode_) std::cout << "Derivative[" << value << "][" << sym << "]";
    }
    else
    {
      ml_.put_symbol(sym);
      if(debug_mode_) std::cout << sym;
    }

    if(debug_mode_){
      if(prevflag) std::cout << "]";

      std::cout << " ";
    }    

    vars_it++;
  }

  // 要素の全削除
  vars_.clear();

  if(debug_mode_) std::cout << std::endl;
}

// 制約ストアの中身を分析して送信
void PacketSender::put_cs(ConstraintStore constraint_store)
{
  if(debug_mode_) {
    std::cout << "------Constraint store------" << std::endl;
    if(constraint_store.first.str == "True")
    {
      std::cout << "no Constraints" << std::endl;
    }
    else
    {
      std::cout << constraint_store.first << std::endl;
    }
    std::cout << "----------------------------" << std::endl;
  }

  ml_.put_function("List", 1);
  ml_.put_function("ToExpression", 1);
  std::string str = constraint_store.first.str;
  ml_.put_string(str);

/*
  int cs_size = constraint_store.size();
  // 制約ストアが空の場合は、空のリストをJoinする必要あり
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

    // 変数名

    if(variable.previous == true)
    {
      ml_.put_function("prev", 1);
    }

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

    vars_.insert(std::make_pair(variable.name, variable.derivative_count + 1));

    // 変数の値
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
*/

}


} //namespace symbolic_simulator
} // namespace hydla
