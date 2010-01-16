#include "ConsistencyChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

ConsistencyChecker::ConsistencyChecker(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

ConsistencyChecker::~ConsistencyChecker()
{}


bool ConsistencyChecker::is_consistent(tells_t& collected_tells, 
                                       ConstraintStore& constraint_store)
{

/*
  ml_.put_function("isConsistent", 2);
  ml_.put_function("List", 3);
  ml_.put_function("Equal", 2);
  ml_.put_symbol("x");
  ml_.put_symbol("y");
  ml_.put_function("Equal", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(2);
  ml_.put_function("Equal", 2);
  ml_.put_symbol("y");
  ml_.MLPutInteger(1);

  ml_.put_function("List", 2);
  ml_.put_symbol("x");
  ml_.put_symbol("y");
  ml_.MLEndPacket();
*/

  // isConsistent[expr, vars]を渡したい
  ml_.put_function("isConsistent", 2);

  ml_.put_function("Join", 2);
  // tell制約の集合からexprを得てMathematicaに渡す
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  tells_t::iterator tells_it = collected_tells.begin();
  PacketSender ps(ml_, NP_POINT_PHASE);
  while((tells_it) != collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }

  // 制約ストアからもexprを得てMathematicaに渡す
  ps.put_cs(constraint_store);


  // varsを渡す
  ml_.put_function("Join", 2);
  ps.put_vars();
  // 制約ストア内に出現する変数も渡す
  ps.put_cs_vars(constraint_store);


  // 結果を受け取る前に制約ストアを初期化
  constraint_store.first.clear();
  constraint_store.second.clear();

/*
//ml_.skip_pkt_until(RETURNPKT);
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  // 解けなかった場合は0が返る（制約間に矛盾があるということ）
  if(ml_.MLGetType() == MLTKINT)
  {
    if(debug_mode_) std::cout << "ConsistencyChecker: false" << std::endl;
    ml_.MLNewPacket();
    return false;
  }

  // 解けた場合は解が「文字列で」返ってくるのでそれを制約ストアに入れる
  // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]や
  // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]や
  // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
  // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  やList[List["True"], List[]]など
  if(debug_mode_) {
    std::cout << "---build constraint store---" << std::endl;
  }

  ml_.MLGetNext(); // Listという関数名
  ml_.MLGetNext(); // List関数（Orで結ばれた解を表している）

  // List関数の要素数（Orで結ばれた解の個数）を得る
  int or_size;
  if(! ml_.MLGetArgCount(&or_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
    return false;
  }
  ml_.MLGetNext(); // Listという関数名

  for(int i=0; i<or_size; i++)
  {
    ml_.MLGetNext(); // List関数（Andで結ばれた解を表している）

    // List関数の要素数（Andで結ばれた解の個数）を得る
    int and_size;
    if(! ml_.MLGetArgCount(&and_size)){
      std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
      throw MathLinkError("MLGetArgCount", ml_.MLError());
      return false;
    }
    ml_.MLGetNext(); // Listという関数名
    ml_.MLGetNext(); // Listの中の先頭要素

    std::set<SymbolicValue> value_set;    
    for(int j=0; j<and_size; j++)
    {
      std::string str = ml_.get_string();
      SymbolicValue symbolic_value;
      symbolic_value.str = str;
      value_set.insert(symbolic_value);
    }
    constraint_store.first.insert(value_set);
  }


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
    switch(ml_.MLGetNext())
    {
      case MLTKFUNC: // Derivative[number][]
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
        ml_.MLGetNext(); // 変数
        symbolic_variable.name = ml_.get_symbol();
        break;
      case MLTKSYM: // シンボル（記号）xとかyとか
        symbolic_variable.derivative_count = 0;
        symbolic_variable.name = ml_.get_symbol();
        break;
      default:
        ;
    }

    constraint_store.second.insert(symbolic_variable);
  }


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
    std::cout << "ConsistencyChecker: true" << std::endl;
  }

  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
