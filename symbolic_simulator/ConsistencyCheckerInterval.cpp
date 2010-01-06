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
                                               ConstraintStore& constraint_store)
{
  // isConsistentInterval[tells, store, tellsvars, storevars]を渡したい
  ml_.put_function("isConsistentInterval", 4);


  // tell制約の集合からtellsを得てMathematicaに渡す
  int tells_size = collected_tells.size();
//  ml_.put_function("List", 2);
  ml_.put_function("List", tells_size);
  tells_t::iterator tells_it = collected_tells.begin();
  PacketSenderInterval psi(ml_, debug_mode_);

/*
  psi.visit((*tells_it));
  tells_it++;
  psi.visit((*tells_it));
  tells_it++;
*/

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
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store.second.str);

/*
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check();
*/

/*
ml_.skip_pkt_until(RETURNPKT);
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check2();
*/

  ml_.skip_pkt_until(RETURNPKT);

  int n;
  if(! ml_.MLGetInteger(&n)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
    throw MathLinkError("MLGetInteger", ml_.MLError());
  }

  if(debug_mode_) {
    std::cout << "ConsistencyCheckerInterval: " << n  << std::endl;
  }

  return n >= 1;
/*
  // 解けなかった場合は0が返る（制約間に矛盾があるということ）
  if(ml_.MLGetType() == MLTKINT)
  {
    if(debug_mode_) std::cout << "ConsistencyChecker: false" << std::endl;
    ml_.MLNewPacket();
    return false;
  }

  // 解けた場合は各変数名とその値が「文字列で」返ってくるのでそれを制約ストアに入れる
  if(debug_mode_) {
    std::cout << "---build constraint store---" << std::endl;
  }

  ml_.MLGetNext(); // List関数
  ml_.MLGetNext(); // Listという関数名
  std::string str = ml_.get_string();
  SymbolicValue symbolic_value;
  symbolic_value.str = str;
  constraint_store.first = symbolic_value;

  // 出現する変数の一覧が文字列で返ってくるのでそれを制約ストアに入れる
  str = ml_.get_string();
  SymbolicValue vars_list;
  vars_list.str = str;
  constraint_store.second = vars_list;  

  if(debug_mode_) {
    std::cout << constraint_store.first << std::endl;
    std::cout << "----------------------------" << std::endl;
    std::cout << "ConsistencyChecker: true" << std::endl;
  }

  return true;
*/

}


} //namespace symbolic_simulator
} // namespace hydla
