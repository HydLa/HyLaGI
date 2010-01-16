#include "PacketSender.h"
#include <iostream>
#include "Logger.h"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

const std::string PacketSender::var_prefix("usrVar");

PacketSender::PacketSender(MathLink& ml, now_phase_t phase) :
  ml_(ml),
  phase_(phase),
  differential_count_(0),
  in_prev_(false)
{}

PacketSender::~PacketSender(){}


// Ask制約
void PacketSender::visit(boost::shared_ptr<Ask> node)                   
{
  // ガード条件のみputする
  debug_string_ += "guard:";
  accept(node->get_guard());    
  HYDLA_LOGGER_DEBUG(debug_string_);
  debug_string_.erase();
}

// Tell制約
void PacketSender::visit(boost::shared_ptr<Tell> node)                  
{
  accept(node->get_child());
  HYDLA_LOGGER_DEBUG(debug_string_);
  debug_string_.erase();
}

// 比較演算子
void PacketSender::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.put_function("Equal", 2);

  accept(node->get_lhs());
  debug_string_ += "=";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.put_function("UnEqual", 2);

  accept(node->get_lhs());
  debug_string_ += "!=";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Less> node)                  
{
  ml_.put_function("Less", 2);

  accept(node->get_lhs());
  debug_string_ += "<";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.put_function("LessEqual", 2);

  accept(node->get_lhs());    
  debug_string_ += "<=";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Greater> node)               
{
  ml_.put_function("Greater", 2);

  accept(node->get_lhs());
  debug_string_ += ">";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.put_function("GreaterEqual", 2);

  accept(node->get_lhs());
  debug_string_ += ">=";
  accept(node->get_rhs());
}

// 論理演算子
void PacketSender::visit(boost::shared_ptr<LogicalAnd> node)            
{
  ml_.put_function("And", 2);

  accept(node->get_lhs());
  debug_string_ += " & ";
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
  debug_string_ += "+";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.put_function("Subtract", 2);

  accept(node->get_lhs());
  debug_string_ += "-";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Times> node)                 
{
  ml_.put_function("Times", 2);

  accept(node->get_lhs());
  debug_string_ += "*";
  accept(node->get_rhs());
}

void PacketSender::visit(boost::shared_ptr<Divide> node)                
{
  ml_.put_function("Divide", 2);

  accept(node->get_lhs());
  debug_string_ += "/";
  accept(node->get_rhs());
}
  
// 算術単項演算子
void PacketSender::visit(boost::shared_ptr<Negative> node)              
{
  ml_.put_function("Minus", 1);

  debug_string_ += "-";
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
  accept(node->get_child());
  differential_count_--;
}

// 左極限
void PacketSender::visit(boost::shared_ptr<Previous> node)              
{
  in_prev_ = true;
  if(this->phase_==NP_POINT_PHASE) {
    ml_.put_function("prev", 1);
    this->debug_string_ += "prev[";
    accept(node->get_child());
    this->debug_string_ += "]";
  } else {
    accept(node->get_child());
  }
  in_prev_ = false;
}
  
// 変数
void PacketSender::visit(boost::shared_ptr<Variable> node)              
{
  if(this->phase_==NP_INTERVAL_PHASE) {
    // 変数名の最後に必ず[t]がつく分
    ml_.MLPutNext(MLTKFUNC);
    ml_.MLPutArgCount(1);
  }

  // 変数のput
  if(differential_count_ > 0){
    // 微分変数なら (Derivative[回数])[変数名]をput
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
    ml_.MLPutArgCount(1);      // this 1 is for the 'f'
    ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
    ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
    ml_.put_symbol("Derivative");
    ml_.MLPutInteger(differential_count_);
    ml_.put_symbol((PacketSender::var_prefix + node->get_name()).c_str());
  }else{
    ml_.put_symbol((PacketSender::var_prefix + node->get_name()).c_str());
  }
  if(this->phase_==NP_INTERVAL_PHASE) ml_.put_symbol("t");

  // putした変数の情報を保持
  if(in_prev_){
    vars_.insert(std::make_pair(PacketSender::var_prefix + node->get_name(),
      -1*(differential_count_ +1)));
  }else{
    vars_.insert(std::make_pair(PacketSender::var_prefix + node->get_name(),
      differential_count_ + 1));
  }

  // デバッグ文字列の生成
  if(differential_count_ > 0){
    this->debug_string_ += "Derivative[";
    this->debug_string_ += differential_count_ ;
    this->debug_string_ += "][";
    this->debug_string_ += (node->get_name() + "]");
  }else{
    this->debug_string_ += node->get_name();
  }
  if(this->phase_==NP_INTERVAL_PHASE) this->debug_string_ += "[t]";
}

// 数字
void PacketSender::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  this->debug_string_ += node->get_number();
}

/**
 * ある式(ノード)をputする
 * @param node putしたい式(ノード)
 */
void PacketSender::put_node(const node_sptr& node)
{
  differential_count_ = 0;
  in_prev_ = false;
  accept(node);
}

/**
 * 変数の一覧を送信
 */
void PacketSender::put_vars()
{
  ml_.put_function("List", vars_.size());

  std::string debug_str("vars: ");

  PacketSender::const_iterator it;
  for(it=this->begin(); it!=this->end(); it++) {
    const std::string name(PacketSender::get_var_name(*it));
    const int d_count = PacketSender::get_var_differential_count(*it);
    const bool is_prev = PacketSender::is_var_prev(*it);

    // 変数がprev && PointPhase -> prev関数をput
    if(is_prev && this->phase_==NP_POINT_PHASE) {
      ml_.put_function("prev", 1);
      debug_str += "prev[";
    }

    // Interval Phaseなら関数変数としての[t]をputする準備が必要
    if(this->phase_==NP_INTERVAL_PHASE) {
      ml_.MLPutNext(MLTKFUNC);
      ml_.MLPutArgCount(1);
    }

    // 微分変数なら (Derivative[回数])[変数名]をput
    if(d_count > 0) {
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_.put_symbol("Derivative");
      ml_.MLPutInteger(d_count);
      ml_.put_symbol(name);
      debug_str += "Derivative[";
      debug_str += d_count;
      debug_str += "][" + name + "]";
    } else {
      // たんに変数名をput
      ml_.put_symbol(name);
      debug_str += name;
    }

    // Interval Phaseなら[t]をput
    if(this->phase_==NP_INTERVAL_PHASE) {
      ml_.put_symbol("t");
      debug_str += "[t]";
    }

    // 変数がprev && PointPhase -> prev関数をputした
    if(is_prev && this->phase_==NP_POINT_PHASE) debug_str += "]";
    debug_str += ", ";

  } // for

  HYDLA_LOGGER_DEBUG(debug_str);
}

// 制約ストアの中身を分析して送信
//void PacketSender::put_cs(ConstraintStoreInterval constraint_store)
//{
//  if(debug_mode_) {
//    std::cout << "------Constraint store------" << std::endl;
//  }
//  int or_cons_size = constraint_store.first.size();
//  if(or_cons_size <= 0)
//  {
//    std::cout << "no Constraints" << std::endl;
//    std::cout << "----------------------------" << std::endl;
//    ml_.put_function("List", 0);
//    return;
//  }
//
//  std::set<std::set<SymbolicValue> >::iterator or_cons_it;
//  std::set<SymbolicValue>::iterator and_cons_it;
//  or_cons_it = constraint_store.first.begin();
//  while((or_cons_it) != constraint_store.first.end())
//  {
//    and_cons_it = (*or_cons_it).begin();
//    while((and_cons_it) != (*or_cons_it).end())
//    {
//      std::cout << (*and_cons_it).str << " ";
//      and_cons_it++;
//    }
//    std::cout << std::endl;
//    or_cons_it++;
//  }
//
//  if(debug_mode_) {
//    std::cout << "----------------------------" << std::endl;
//  }
//
//  ml_.put_function("List", 1);
//  ml_.put_function("Or", or_cons_size);
//  or_cons_it = constraint_store.first.begin();
//  while((or_cons_it) != constraint_store.first.end())
//  {
//    int and_cons_size = (*or_cons_it).size();
//    ml_.put_function("And", and_cons_size);
//    and_cons_it = (*or_cons_it).begin();
//    while((and_cons_it) != (*or_cons_it).end())
//    {
//      ml_.put_function("ToExpression", 1);
//      std::string str = (*and_cons_it).str;
//      ml_.put_string(str);
//      and_cons_it++;
//    }
//    or_cons_it++;
//  }
//}

// 制約ストアに出現する変数の一覧を送信
//void PacketSender::put_cs_vars(ConstraintStoreInterval constraint_store)
//{
//  int vars_size = constraint_store.second.size();
//  std::set<SymbolicVariable>::iterator vars_it = constraint_store.second.begin();
//
//  ml_.put_function("List", vars_size);
//  while((vars_it) != constraint_store.second.end())
//  {
//    if(int value = (*vars_it).derivative_count > 0)
//    {
//      ml_.MLPutNext(MLTKFUNC);
//      ml_.MLPutArgCount(1);
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//      ml_.put_symbol("Derivative");
//      ml_.MLPutInteger(value);
//      ml_.put_symbol((*vars_it).name);
//      if(debug_mode_) std::cout << "Derivative[" << value << "][" << (*vars_it).name << "]";
//    }
//    else
//    {
//      ml_.put_function((*vars_it).name, 1);
//      if(debug_mode_) std::cout << (*vars_it).name;
//    }
//    ml_.put_symbol("t");
//    if(debug_mode_) std::cout << "[t] ";
//    vars_it++;
//  }
//  if(debug_mode_) std::cout << std::endl;
//}

} //namespace symbolic_simulator
} // namespace hydla

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

//using namespace hydla::parse_tree;
//using namespace hydla::simulator;
//
//namespace hydla {
//namespace symbolic_simulator {
//
//
//PacketSender::PacketSender(MathLink& ml, bool debug_mode) :
//  ml_(ml),
//  differential_count_(0),
//  in_prev_(false),
//  debug_mode_(debug_mode)
//{}
//
//PacketSender::~PacketSender()
//{}
//
//
//// Ask制約
//void PacketSender::visit(boost::shared_ptr<Ask> node)                   
//{
//  if(debug_mode_) std::cout << "guard:";
//  //ml_.put_function("ask", 2);
//  accept(node->get_guard());
//    
//  //std::cout << " => ";
//  //accept(node->get_child());
//  if(debug_mode_) std::cout << std::endl;
//}
//
//// Tell制約
//void PacketSender::visit(boost::shared_ptr<Tell> node)                  
//{
//  //ml_.put_function("tell", 1);
//
//  accept(node->get_child());
//  if(debug_mode_) std::cout << std::endl;
//}
//
//// 比較演算子
//void PacketSender::visit(boost::shared_ptr<Equal> node)                 
//{
//  ml_.put_function("Equal", 2);
//        
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << "=";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<UnEqual> node)               
//{
//  ml_.put_function("UnEqual", 2);
//
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << "!=";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<Less> node)                  
//{
//  ml_.put_function("Less", 2);
//
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << "<";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<LessEqual> node)             
//{
//  ml_.put_function("LessEqual", 2);
//
//  accept(node->get_lhs());    
//  if(debug_mode_) std::cout << "<=";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<Greater> node)               
//{
//  ml_.put_function("Greater", 2);
//    
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << ">";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<GreaterEqual> node)          
//{
//  ml_.put_function("GreaterEqual", 2);
//    
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << ">=";
//  accept(node->get_rhs());
//}
//
//// 論理演算子
//void PacketSender::visit(boost::shared_ptr<LogicalAnd> node)            
//{
//  ml_.put_function("And", 2);
//
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << " & ";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<LogicalOr> node)             
//{
//  //ml_.put_function("Or", 2);
//    
//  accept(node->get_lhs());
//  accept(node->get_rhs());
//}
//  
//// 算術二項演算子
//void PacketSender::visit(boost::shared_ptr<Plus> node)                  
//{
//  ml_.put_function("Plus", 2);
//    
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << "+";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<Subtract> node)              
//{
//  ml_.put_function("Subtract", 2);
//    
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << "-";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<Times> node)                 
//{
//  ml_.put_function("Times", 2);
//
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << "*";
//  accept(node->get_rhs());
//}
//
//void PacketSender::visit(boost::shared_ptr<Divide> node)                
//{
//  ml_.put_function("Divide", 2);
//    
//  accept(node->get_lhs());
//  if(debug_mode_) std::cout << "/";
//  accept(node->get_rhs());
//}
//  
//// 算術単項演算子
//void PacketSender::visit(boost::shared_ptr<Negative> node)              
//{       
//  ml_.put_function("Minus", 1);
//  if(debug_mode_) std::cout << "-";
//
//  accept(node->get_child());
//}
//
//void PacketSender::visit(boost::shared_ptr<Positive> node)              
//{
//  accept(node->get_child());
//}
//
//// 微分
//void PacketSender::visit(boost::shared_ptr<Differential> node)          
//{
//
//  differential_count_++;
//
//  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//  ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//  ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//  ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//  ml_.put_symbol("Derivative");
//  ml_.MLPutInteger(1);
//
//  accept(node->get_child());
//
//  differential_count_--;
//}
//
//// 左極限
//void PacketSender::visit(boost::shared_ptr<Previous> node)              
//{
//  in_prev_ = true;
//  ml_.put_function("prev", 1);
//  if(debug_mode_) std::cout << "prev[";
//  accept(node->get_child());
//  if(debug_mode_) std::cout << "]";
//  in_prev_ = false;
//}
//  
//// 変数
//void PacketSender::visit(boost::shared_ptr<Variable> node)              
//{
//  ml_.put_symbol(("usrVar" + node->get_name()).c_str());
//  if(in_prev_){
//    vars_.insert(std::make_pair("usrVar" + node->get_name(), -1*(differential_count_ +1)));
//  }else{
//    vars_.insert(std::make_pair("usrVar" + node->get_name(), differential_count_ + 1));
//  }
//
//  if(debug_mode_) {
//    if(differential_count_ > 0){
//      std::cout << "Derivative[" << differential_count_ << "][" << node->get_name().c_str() << "]";
//    }else{
//      std::cout << node->get_name().c_str();
//    }
//  }
//}
//
//// 数字
//void PacketSender::visit(boost::shared_ptr<Number> node)                
//{    
//  ml_.MLPutInteger(atoi(node->get_number().c_str()));
//  if(debug_mode_) std::cout << node->get_number().c_str();
//}
//
//
//// 変数の一覧を送信
//void PacketSender::put_vars()
//{
//  int var_num = vars_.size();
//  ml_.put_function("List", var_num);
//  std::set<std::pair<std::string, int> >::iterator vars_it = vars_.begin();
//  const char* sym;
//  int value;
//  if(debug_mode_) std::cout << "vars: ";
//  while(vars_it!=vars_.end())
//  {
//    sym = ((*vars_it).first).c_str();
//    value = (*vars_it).second;
//    int prevflag = 0;
//    if(value < 0)
//    {
//      prevflag = 1;
//      ml_.put_function("prev", 1);
//      if(debug_mode_) std::cout << "prev[";
//      value *= -1;
//    }
//    value -= 1;
//    if(value != 0)
//    {
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//      ml_.put_symbol("Derivative");
//      ml_.MLPutInteger(value);
//      ml_.put_symbol(sym);
//      if(debug_mode_) std::cout << "Derivative[" << value << "][" << sym << "]";
//    }
//    else
//    {
//      ml_.put_symbol(sym);
//      if(debug_mode_) std::cout << sym;
//    }
//
//    if(debug_mode_){
//      if(prevflag) std::cout << "]";
//
//      std::cout << " ";
//    }    
//
//    vars_it++;
//  }
//
//  // 要素の全削除
//  vars_.clear();
//
//}
//
//// 制約ストアの中身を分析して送信
//void PacketSender::put_cs(ConstraintStore constraint_store)
//{
//  if(debug_mode_) {
//    std::cout << "------Constraint store------" << std::endl;
//  }
//  int or_cons_size = constraint_store.first.size();
//  if(or_cons_size <= 0)
//  {
//    std::cout << "no Constraints" << std::endl;
//    std::cout << "----------------------------" << std::endl;
//    ml_.put_function("List", 0);
//    return;
//  }
//
//  std::set<std::set<SymbolicValue> >::iterator or_cons_it;
//  std::set<SymbolicValue>::iterator and_cons_it;
//  or_cons_it = constraint_store.first.begin();
//  while((or_cons_it) != constraint_store.first.end())
//  {
//    and_cons_it = (*or_cons_it).begin();
//    while((and_cons_it) != (*or_cons_it).end())
//    {
//      std::cout << (*and_cons_it).str << " ";
//      and_cons_it++;
//    }
//    std::cout << std::endl;
//    or_cons_it++;
//  }
//
//  if(debug_mode_) {
//    std::cout << "----------------------------" << std::endl;
//  }
//
//  ml_.put_function("List", 1);
//  ml_.put_function("Or", or_cons_size);
//  or_cons_it = constraint_store.first.begin();
//  while((or_cons_it) != constraint_store.first.end())
//  {
//    int and_cons_size = (*or_cons_it).size();
//    ml_.put_function("And", and_cons_size);
//    and_cons_it = (*or_cons_it).begin();
//    while((and_cons_it) != (*or_cons_it).end())
//    {
//      ml_.put_function("ToExpression", 1);
//      std::string str = (*and_cons_it).str;
//      ml_.put_string(str);
//      and_cons_it++;
//    }
//    or_cons_it++;
//  }
//}
//
//// 制約ストアに出現する変数の一覧を送信
//void PacketSender::put_cs_vars(ConstraintStore constraint_store)
//{
//  int vars_size = constraint_store.second.size();
//  std::set<SymbolicVariable>::iterator vars_it = constraint_store.second.begin();
//
//  ml_.put_function("List", vars_size);
//  while((vars_it) != constraint_store.second.end())
//  {
//    if(int value = (*vars_it).derivative_count > 0)
//    {
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
//      ml_.MLPutArgCount(1);      // this 1 is for the 'f'
//      ml_.MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
//      ml_.MLPutArgCount(1);      // this 1 is for the '*number*'
//      ml_.put_symbol("Derivative");
//      ml_.MLPutInteger(value);
//      ml_.put_symbol((*vars_it).name);
//      if(debug_mode_) std::cout << "Derivative[" << value << "][" << (*vars_it).name << "]";
//    }
//    else
//    {
//      ml_.put_symbol((*vars_it).name);
//      if(debug_mode_) std::cout << (*vars_it).name;
//    }
//    if(debug_mode_) std::cout << " ";
//    vars_it++;
//  }
//  if(debug_mode_) std::cout << std::endl;
//}
//
//
//} //namespace symbolic_simulator
//} // namespace hydla
