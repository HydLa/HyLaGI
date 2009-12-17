#include "ConsistencyChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

ConsistencyChecker::ConsistencyChecker(MathLink& ml) :
  ml_(ml)
{}

ConsistencyChecker::~ConsistencyChecker()
{}


bool ConsistencyChecker::is_consistent(TellCollector::tells_t& collected_tells, ConstraintStore& constraint_store)
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
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  PacketSender ps(ml_);
  while((tells_it) != collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }

  // 制約ストアからもexprを得てMathematicaに渡す
  ps.put_cs(constraint_store);


  // varsを渡す
  ps.put_vars();

/*
ml_.skip_pkt_until(RETURNPKT);
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check2();
*/

  ml_.skip_pkt_until(RETURNPKT);
  // 解けなかった場合は0が返る（制約間に矛盾があるということ）
  if(ml_.MLGetType() == MLTKINT)
  {
    std::cout << "ConsistencyChecker: false" << std::endl;
    ml_.MLNewPacket();
    return false;
  }

  // 解けた場合は各変数名とその値が返ってくるのでそれを制約ストアに入れる
  // List[pair[x,1]]やList[pair[x,1], pair[y,2], pair[z,3]]や
  // List[pair[x,1], pair[Derivative[1][x],1], pair[prev[x],1], pair[prev[Derivative[2][x],1]]]やList[]など
  std::cout << "---build constraint store---" << std::endl;

  // 最初はList関数が届くのでその要素数（pairの個数）を得る
  int funcarg;
  if(! ml_.MLGetArgCount(&funcarg)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    return false;
  }

  ml_.MLGetNext(); // Listという関数名

  SymbolicVariable symbolic_variable;
  SymbolicValue symbolic_value;
  // Listに含まれる1つ1つのpairについて調べる
  for(int i = 0; i < funcarg; i++)
  {
    ml_.MLGetNext(); // pair関数が得られる
    ml_.MLGetNext(); // pairという関数名
//    symbolic_variable.previous = false;    
  A:
    switch(ml_.MLGetNext()) // pair[variable, value]のvariable側が得られる
    {
    case MLTKFUNC: // Derivative[number][]とprev[]
      switch(ml_.MLGetNext()) // Derivative[number]やprevという関数名
      {
      case MLTKFUNC: // Derivative[number]
        ml_.MLGetNext(); // Derivativeという関数名
        ml_.MLGetNext(); // number
        int n;
        if(! ml_.MLGetInteger(&n)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_variable.derivative_count = n;
        ml_.MLGetNext(); // 変数
        symbolic_variable.name = ml_.get_symbol();
        break;
      case MLTKSYM: // prev
//        symbolic_variable.previous = true;
        goto A; // prevの中身を調べる（通常変数の場合とDerivativeつきの場合とがある）
        break;
      default:
        ;
      }
      break;
    case MLTKSYM: // シンボル（記号）xとかyとか
      symbolic_variable.derivative_count = 0;
      symbolic_variable.name = ml_.get_symbol();
      break;
    default:
      ;
    }

    int numerator;
    int denominator;
    switch(ml_.MLGetNext()) // pair[variable, value]のvalue側が得られる
    {
      case MLTKFUNC: // Rational関数
        symbolic_value.rational = true;
        ml_.MLGetNext(); // Rationalという関数名
        ml_.MLGetNext(); // 分子
        if(! ml_.MLGetInteger(&numerator)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_value.numerator = numerator;        
        ml_.MLGetNext(); // 分母
        if(! ml_.MLGetInteger(&denominator)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_value.denominator = denominator;
        break;
      case MLTKINT:
        symbolic_value.rational = false;
        if(! ml_.MLGetInteger(&numerator)){
          std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
          return false;
        }
        symbolic_value.numerator = numerator;        
        symbolic_value.denominator = 1;
        break;
      default:
        ;
    }   
    constraint_store.set_variable(symbolic_variable, symbolic_value); 
  }

  std::cout << constraint_store;
  std::cout << "----------------------------" << std::endl;
  std::cout << "ConsistencyChecker: true" << std::endl;

  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
