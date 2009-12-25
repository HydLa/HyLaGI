#include "EntailmentChecker.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


EntailmentChecker::EntailmentChecker(MathLink& ml) :
  ml_(ml)
{}

EntailmentChecker::~EntailmentChecker()
{}

/**
 * collected_tellsから、negative_asks内のask制約のガード条件が満たされるかどうか調べる
 * @param negative_ask     まだ展開されていないask制約1つ
 * @param collected_tells  tell制約のリスト（展開されたask制約の「=>」の右辺はここに追加される）
 * @return チェックの結果、そのask制約が展開されたかどうか
 */

bool EntailmentChecker::check_entailment(
  const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask, 
//hydla::simulator::TellCollector::tells_t& collected_tells)
  hydla::symbolic_simulator::ConstraintStore& constraint_store)
{

/*
  ml_.put_function("checkEntailment", 3);

  ml_.put_function("And", 2);
  ml_.put_function("GreaterEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(0);
  ml_.put_function("LessEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(5);

  ml_.put_function("List", 1);
  ml_.put_function("And", 2);
  ml_.put_function("GreaterEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(2);
  ml_.put_function("LessEqual", 2);
  ml_.put_symbol("x");
  ml_.MLPutInteger(3);

  ml_.put_function("List", 1);
  ml_.put_symbol("x");

  ml_.MLEndPacket();
*/

  // checkEntailment[guard, tells, vars]を渡したい
  ml_.put_function("checkEntailment", 3);


  // ask制約のガードの式を得てMathematicaに渡す
  PacketSender ps(ml_);
  ps.visit(negative_ask);


  // 制約ストアからもexprを得てMathematicaに渡す
  ps.put_cs(constraint_store);

/*
  // tell制約の集合からtellsを得てMathematicaに渡す
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while(tells_it!=collected_tells.end())
  {
    ps.visit((*tells_it));
    tells_it++;
  }
*/

  // varsを渡す
  ml_.put_function("Join", 2);
  ps.put_vars();
  ml_.put_function("ToExpression", 1);
  ml_.put_string(constraint_store.second.str);

/*
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check();
*/

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  ml_.MLGetInteger(&num);
  std::cout << "EntailmentChecker#num:" << num << std::endl;
  
  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
