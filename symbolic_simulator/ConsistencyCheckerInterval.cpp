#include "ConsistencyCheckerInterval.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

ConsistencyCheckerInterval::ConsistencyCheckerInterval(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

ConsistencyCheckerInterval::~ConsistencyCheckerInterval()
{}


bool ConsistencyCheckerInterval::is_consistent(tells_t& collected_tells, 
                                               ConstraintStoreInterval& constraint_store)
{
  // isConsistentInterval[tells, store, tellsVars, storeVars]を渡したい
  ml_.put_function("isConsistentInterval", 4);


  // tell制約の集合からtellsを得てMathematicaに渡す
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  tells_t::iterator tells_it = collected_tells.begin();
  PacketSenderInterval psi(ml_, debug_mode_);

  while((tells_it) != collected_tells.end())
  {
    psi.visit((*tells_it));
    tells_it++;
  }

  // 制約ストアstoreをMathematicaに渡す
  psi.put_cs(constraint_store);

  // tellsvarsを渡す
  psi.put_vars();

  // storevarsを渡す
  psi.put_cs_vars(constraint_store);

  // 結果を受け取る前に制約ストアを初期化
  constraint_store.first.clear();  
  constraint_store.second.clear();  

/*
ml_.skip_pkt_until(RETURNPKT);
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check2();
*/

  ml_.skip_pkt_until(RETURNPKT);

  // 解けなかった場合は0が返る（制約間に矛盾がある、またはover-constraintということ）
  if(ml_.MLGetType() == MLTKINT)
  {
    if(debug_mode_) std::cout << "ConsistencyCheckerInterval: false" << std::endl;
    ml_.MLNewPacket();
    return false;
  }

  // List[数値, 制約一覧, 変数一覧]が返る
  // 数値部分は問題なく解けたら1、under-constraintが起きていれば2が返る

  ml_.MLGetNext(); // Listという関数名
  ml_.MLGetNext(); // Listの中の先頭

  int n;
  if(! ml_.MLGetInteger(&n)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
    throw MathLinkError("MLGetInteger", ml_.MLError());
  }

  if(debug_mode_) {
    std::cout << "ConsistencyCheckerInterval: " << n  << std::endl;
    if(n==2) std::cout << "under-constraint" << std::endl;
    std::cout << "---build constraint store---" << std::endl;
  }

  ml_.MLGetNext(); // List関数

  // List関数の要素数（制約の個数）を得る
  int cons_size;
  if(! ml_.MLGetArgCount(&cons_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
    return false;
  }
  ml_.MLGetNext(); // Listという関数名
  ml_.MLGetNext(); // Listの中の先頭要素

  std::set<SymbolicValue> value_set;    
  for(int i=0; i<cons_size; i++)
  {
    std::string str = ml_.get_string();
    SymbolicValue symbolic_value;
    symbolic_value.str = str;
    value_set.insert(symbolic_value);
  }
  constraint_store.first.insert(value_set);


  // 出現する変数の一覧が文字列で返ってくるのでそれを制約ストアに入れる
  ml_.MLGetNext(); // List関数

  // List関数の要素数（変数一覧に含まれる変数の個数）を得る
  int vars_size;
  if(! ml_.MLGetArgCount(&vars_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
    return false;
  }
  ml_.MLGetNext(); // Listという関数名

  for(int k=0; k<vars_size; k++)
  {
    SymbolicVariable symbolic_variable;
    std::string sym;
    ml_.MLGetNext(); // Derivative[number][変数名][]またはx[]などの関数
    switch(ml_.MLGetNext()) // Derivative[number][変数名]またはxという関数名
    {
      case MLTKFUNC:
        ml_.MLGetNext(); // Derivative[number]という関数名
        ml_.MLGetNext(); // Derivativeという関数名
        ml_.MLGetNext(); // number
        int n;
        if(! ml_.MLGetInteger(&n)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          throw MathLinkError("MLGetInteger", ml_.MLError());
          return false;
        }
        symbolic_variable.derivative_count = n;
        ml_.MLGetNext(); // 変数名
        symbolic_variable.name = ml_.get_symbol();
        ml_.MLGetNext(); // t
        break;
      case MLTKSYM:
        sym = ml_.get_symbol();
        symbolic_variable.derivative_count = 0;
        ml_.MLGetNext(); // t
        symbolic_variable.name = sym;
        break;
      default:
        ;
    }
    constraint_store.second.insert(symbolic_variable);
  }
  ml_.MLNewPacket(); // エラー回避用。エラーの原因不明

  if(debug_mode_) {
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
    std::cout << "----------------------------" << std::endl;
  }

  return n >= 1;
}


} //namespace symbolic_simulator
} // namespace hydla
