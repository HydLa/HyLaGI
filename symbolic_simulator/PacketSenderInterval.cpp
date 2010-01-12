#include "PacketSenderInterval.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


PacketSenderInterval::PacketSenderInterval(MathLink& ml, bool debug_mode) :
  ml_(ml),
  differential_count_(0),
  in_prev_(false),
  debug_mode_(debug_mode)
{}

PacketSenderInterval::~PacketSenderInterval()
{}


// Ask制約
void PacketSenderInterval::visit(boost::shared_ptr<Ask> node)                   
{
  if(debug_mode_) std::cout << "guard:";
  //ml_.put_function("ask", 2);
  accept(node->get_guard());
    
  //std::cout << " => ";
  //accept(node->get_child());
  if(debug_mode_) std::cout << std::endl;
}

// Tell制約
void PacketSenderInterval::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.put_function("tell", 1);

  accept(node->get_child());
  if(debug_mode_) std::cout << std::endl;
}

// 比較演算子
void PacketSenderInterval::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.put_function("Equal", 2);
        
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "=";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.put_function("UnEqual", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << "!=";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<Less> node)                  
{
  ml_.put_function("Less", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << "<";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.put_function("LessEqual", 2);

  accept(node->get_lhs());    
  if(debug_mode_) std::cout << "<=";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<Greater> node)               
{
  ml_.put_function("Greater", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << ">";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.put_function("GreaterEqual", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << ">=";
  accept(node->get_rhs());
}

// 論理演算子
void PacketSenderInterval::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ml_.put_function("And", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << " & ";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.put_function("Or", 2);
    
  accept(node->get_lhs());
  accept(node->get_rhs());
}
  
// 算術二項演算子
void PacketSenderInterval::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.put_function("Plus", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "+";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.put_function("Subtract", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "-";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<Times> node)                 
{
  ml_.put_function("Times", 2);

  accept(node->get_lhs());
  if(debug_mode_) std::cout << "*";
  accept(node->get_rhs());
}

void PacketSenderInterval::visit(boost::shared_ptr<Divide> node)                
{
  ml_.put_function("Divide", 2);
    
  accept(node->get_lhs());
  if(debug_mode_) std::cout << "/";
  accept(node->get_rhs());
}
  
// 算術単項演算子
void PacketSenderInterval::visit(boost::shared_ptr<Negative> node)              
{       
  ml_.put_function("Minus", 1);
  if(debug_mode_) std::cout << "-";

  accept(node->get_child());
}

void PacketSenderInterval::visit(boost::shared_ptr<Positive> node)              
{
  accept(node->get_child());
}

// 微分
void PacketSenderInterval::visit(boost::shared_ptr<Differential> node)          
{
  differential_count_++;
  accept(node->get_child());
  differential_count_--;
}

// 左極限
void PacketSenderInterval::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  accept(node->get_child());
  in_prev_ = false;
}
  
// 変数
void PacketSenderInterval::visit(boost::shared_ptr<Variable> node)              
{
  // 変数名の最後に必ず[t]がつく分
  ml_.MLPutNext(MLTKFUNC);
  ml_.MLPutArgCount(1);

//  if(in_prev_) ml_.put_function("prev", 1);
  if(differential_count_ > 0){
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
    ml_.MLPutArgCount(1);      // this 1 is for the 'f'
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
    ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
    ml_.put_symbol("Derivative");
    ml_.MLPutInteger(differential_count_);
    ml_.put_symbol(("usrVar" + node->get_name()).c_str());
  }else{
    ml_.put_symbol(("usrVar" + node->get_name()).c_str());
  }
  ml_.put_symbol("t");

  if(in_prev_){
    vars_.insert(std::make_pair("usrVar" + node->get_name(), -1*(differential_count_ +1)));
  }else{
    vars_.insert(std::make_pair("usrVar" + node->get_name(), differential_count_ + 1));
  }

  if(debug_mode_) {
//    if(in_prev_) std::cout << "prev[";
    if(differential_count_ > 0){
      std::cout << "Derivative[" << differential_count_ << "][" << node->get_name().c_str() << "]";
    }else{
      std::cout << node->get_name().c_str();
    }
//    if(in_prev_) std::cout << "]";
    std::cout << "[t]";
  }
}

// 数字
void PacketSenderInterval::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  if(debug_mode_) std::cout << node->get_number().c_str();
}


// 変数の一覧を送信
void PacketSenderInterval::put_vars()
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
//      ml_.put_function("prev", 1);
//      if(debug_mode_) std::cout << "prev[";
      value *= -1;
    }
    value -= 1;
    if(value != 0)
    {
      ml_.MLPutNext(MLTKFUNC);
      ml_.MLPutArgCount(1);
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.put_symbol("Derivative");
      ml_.MLPutInteger(value);
      ml_.put_symbol(sym);
      if(debug_mode_) std::cout << "Derivative[" << value << "][" << sym << "]";
    }
    else
    {
      ml_.put_function(sym, 1);
      if(debug_mode_) std::cout << sym;
    }

    ml_.put_symbol("t");
    if(debug_mode_){
//      if(prevflag) std::cout << "]";
      std::cout << "[t] ";
    }    

    vars_it++;
  }

  // 要素の全削除
  vars_.clear();

}

// 制約ストアの中身を分析して送信
void PacketSenderInterval::put_cs(ConstraintStoreInterval constraint_store)
{
  if(debug_mode_) {
    std::cout << "------Constraint store------" << std::endl;
  }
  int or_cons_size = constraint_store.first.size();
  if(or_cons_size <= 0)
  {
    std::cout << "no Constraints" << std::endl;
    std::cout << "----------------------------" << std::endl;
    ml_.put_function("List", 0);
    return;
  }

  std::set<std::set<SymbolicValue> >::iterator or_cons_it;
  std::set<SymbolicValue>::iterator and_cons_it;
  or_cons_it = constraint_store.first.begin();
  while((or_cons_it) != constraint_store.first.end())
  {
    and_cons_it = (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      std::cout << (*and_cons_it).str << " ";
      and_cons_it++;
    }
    std::cout << std::endl;
    or_cons_it++;
  }

  if(debug_mode_) {
    std::cout << "----------------------------" << std::endl;
  }

  ml_.put_function("List", 1);
  ml_.put_function("Or", or_cons_size);
  or_cons_it = constraint_store.first.begin();
  while((or_cons_it) != constraint_store.first.end())
  {
    int and_cons_size = (*or_cons_it).size();
    ml_.put_function("And", and_cons_size);
    and_cons_it = (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      ml_.put_function("ToExpression", 1);
      std::string str = (*and_cons_it).str;
      ml_.put_string(str);
      and_cons_it++;
    }
    or_cons_it++;
  }
}

// 制約ストアに出現する変数の一覧を送信
void PacketSenderInterval::put_cs_vars(ConstraintStoreInterval constraint_store)
{
  int vars_size = constraint_store.second.size();
  std::set<SymbolicVariable>::iterator vars_it = constraint_store.second.begin();

  ml_.put_function("List", vars_size);
  while((vars_it) != constraint_store.second.end())
  {
    if(int value = (*vars_it).derivative_count > 0)
    {
      ml_.MLPutNext(MLTKFUNC);
      ml_.MLPutArgCount(1);
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.put_symbol("Derivative");
      ml_.MLPutInteger(value);
      ml_.put_symbol((*vars_it).name);
      if(debug_mode_) std::cout << "Derivative[" << value << "][" << (*vars_it).name << "]";
    }
    else
    {
      ml_.put_function((*vars_it).name, 1);
      if(debug_mode_) std::cout << (*vars_it).name;
    }
    ml_.put_symbol("t");
    if(debug_mode_) std::cout << "[t] ";
    vars_it++;
  }
  if(debug_mode_) std::cout << std::endl;
}

} //namespace symbolic_simulator
} // namespace hydla
