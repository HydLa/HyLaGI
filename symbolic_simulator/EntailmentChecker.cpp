#include "EntailmentChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


EntailmentChecker::EntailmentChecker(MathLink& ml) :
  ml_(ml),
  in_differential_equality_(false),
  in_differential_(false)
{}

EntailmentChecker::~EntailmentChecker()
{}

// Tell制約
void EntailmentChecker::visit(boost::shared_ptr<Tell> node)                  
{
  //ml_.MLPutFunction("tell", 1);

  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << std::endl;
}

// 比較演算子
void EntailmentChecker::visit(boost::shared_ptr<Equal> node)                 
{
  ml_.MLPutFunction("Equal", 2);
        
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<UnEqual> node)               
{
  ml_.MLPutFunction("UnEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "!=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<Less> node)                  
{
  ml_.MLPutFunction("Less", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "<";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<LessEqual> node)             
{
  ml_.MLPutFunction("LessEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "<=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<Greater> node)               
{
  ml_.MLPutFunction("Greater", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << ">";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

void EntailmentChecker::visit(boost::shared_ptr<GreaterEqual> node)          
{
  ml_.MLPutFunction("GreaterEqual", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << ">=";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
  in_differential_equality_ = false;
}

// 論理演算子
void EntailmentChecker::visit(boost::shared_ptr<LogicalAnd> node)            
{
  //ml_.MLPutFunction("And, 2);

  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<LogicalOr> node)             
{
  //ml_.MLPutFunction("Or", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// 算術二項演算子
void EntailmentChecker::visit(boost::shared_ptr<Plus> node)                  
{
  ml_.MLPutFunction("Plus", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "+";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Subtract> node)              
{
  ml_.MLPutFunction("Subtract", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "-";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Times> node)                 
{
  ml_.MLPutFunction("Times", 2);

  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "*";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Divide> node)                
{
  ml_.MLPutFunction("Divide", 2);
    
  node_sptr lnode(node->get_lhs()); lnode->accept(lnode, this);
  std::cout << "/";
  node_sptr rnode(node->get_rhs()); rnode->accept(rnode, this);
}
  
// 算術単項演算子
void EntailmentChecker::visit(boost::shared_ptr<Negative> node)              
{       
  ml_.MLPutFunction("Minus", 1);
  std::cout << "-";

  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
}

void EntailmentChecker::visit(boost::shared_ptr<Positive> node)              
{
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
}

// 微分
void EntailmentChecker::visit(boost::shared_ptr<Differential> node)          
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
    //std::cout << "[t]"; // ht[t]' のようになるのを防ぐため
  }

  in_differential_ = false;
}

// 左極限
void EntailmentChecker::visit(boost::shared_ptr<Previous> node)              
{
  //ml_.MLPutFunction("prev", 1);
  node_sptr chnode(node->get_child_node());
  chnode->accept(chnode, this);
  std::cout << "-";
}
  
// 変数
void EntailmentChecker::visit(boost::shared_ptr<Variable> node)              
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

// 数字
void EntailmentChecker::visit(boost::shared_ptr<Number> node)                
{    
  ml_.MLPutInteger(atoi(node->get_number().c_str()));
  std::cout << node->get_number().c_str();
}

/*
 * collected_tellsから、negative_asks内のask制約のガード条件が満たされるかどうか調べる
 * 
 * Input:
 *  positive_asks すでに展開されているask制約
 *  negative_asks まだ展開されていないask制約
 *  collected_tells tell制約のリスト（展開されたask制約の「=>」の右辺がここに追加される）
 * Output:
 *  チェックの結果、展開されたask制約が1つでも存在したかどうか
 */

bool EntailmentChecker::check_entailment(
  boost::shared_ptr<hydla::parse_tree::Ask> negative_ask, collected_tells_t& collected_tells)
{

  return true;

  ml_.MLPutFunction("EntailmentChecker", 3);

  ml_.MLPutFunction("And", 2);
  ml_.MLPutFunction("GreaterEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(0);
  ml_.MLPutFunction("LessEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(5);

  ml_.MLPutFunction("And", 2);
  ml_.MLPutFunction("GreaterEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(2);
  ml_.MLPutFunction("LessEqual", 2);
  ml_.MLPutSymbol("x");
  ml_.MLPutInteger(3);

  ml_.MLPutFunction("List", 1);
  ml_.MLPutSymbol("x");
  ml_.MLEndPacket();

/*
  int positive_asks_size   = positive_asks.size();
  int negative_asks_size   = negative_asks.size();
  int collected_tells_size = collected_tells.size();
  // checkEntailment[asks, tells, vars]を渡したい
  ml_.MLPutFunction("checkEntailment", 3);
  ml_.MLPutFunction("List", tells_size);

  // tell制約の集合からexprを得てMathematicaに渡す
  collected_tells_t::iterator tells_it = tells.begin();
  while(tells_it!=vars_.end())
  {
    visit((*tells_it));
    tells_it++;
  }


  // varsを渡す
  int var_num = vars_.size();
  ml_.MLPutFunction("List", var_num);
  std::map<std::string, int>::iterator vars_it = vars_.begin();
  const char* sym;
  std::cout << "vars: ";
  while(vars_it!=vars_.end())
  {
    sym = ((*vars_it).first).c_str();
    if((*vars_it).second==0)
    {
      ml_.MLPutSymbol(sym);
    }
    else
    {
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
    vars_it++;
  }

  // ml_.MLEndPacket();

  // 要素の全削除
  vars_.clear();

  std::cout << std::endl;
*/

/*
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  ml_.MLGetInteger(&num);
  std::cout << "#num:" << num << std::endl;
  
  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
