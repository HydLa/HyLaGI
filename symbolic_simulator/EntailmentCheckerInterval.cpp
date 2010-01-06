#include "EntailmentCheckerInterval.h"
//#include "PacketChecker.h"
#include <iostream>

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


EntailmentCheckerInterval::EntailmentCheckerInterval(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

EntailmentCheckerInterval::~EntailmentCheckerInterval()
{}

/**
 * collected_tellsから、negative_asks内のask制約のガード条件が満たされるかどうか調べる
 * @param negative_ask     まだ展開されていないask制約1つ
 * @param collected_tells  tell制約のリスト（展開されたask制約の「=>」の右辺はここに追加される）
 * @return チェックの結果、そのask制約が展開されたかどうか
 */

bool EntailmentCheckerInterval::check_entailment(
  const boost::shared_ptr<hydla::parse_tree::Ask>& negative_ask, 
//  hydla::simulator::TellCollector::tells_t& collected_tells,
  hydla::symbolic_simulator::ConstraintStore& constraint_store)
{

  // checkEntailment[guard, Join[tells, store], vars]を渡したい
  ml_.put_function("checkEntailment", 3);


  // ask制約のガードの式を得てMathematicaに渡す
  PacketSenderInterval psi(ml_, debug_mode_);
  psi.visit(negative_ask);

/*
  ml_.put_function("Join", 2);
  // tell制約の集合からtellsを得てMathematicaに渡す
  int tells_size = collected_tells.size();
  ml_.put_function("List", tells_size);
  TellCollector::tells_t::iterator tells_it = collected_tells.begin();
  while(tells_it!=collected_tells.end())
  {
    psi.visit((*tells_it));
    tells_it++;
  }
*/

  // 制約ストアからも式storeを得てMathematicaに渡す
  psi.put_cs(constraint_store);

  // varsを渡す
  ml_.put_function("Join", 2);
  psi.put_vars();
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
  if(debug_mode_) std::cout << "EntailmentCheckerInterval#num:" << num << std::endl;

  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1;
}


} //namespace symbolic_simulator
} // namespace hydla
