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
  hydla::symbolic_simulator::ConstraintStoreInterval& constraint_store)
{
/*
  // checkEntailment[guard, store, vars]を渡したい
  ml_.put_function("checkEntailment", 3);


  // ask制約のガードの式を得てMathematicaに渡す
  PacketSender psi(ml_, NP_INTERVAL_PHASE);
  psi.visit(negative_ask);

  // 制約ストアから式storeを得てMathematicaに渡す
  psi.put_cs(constraint_store);

  // varsを渡す
  ml_.put_function("Join", 2);
  psi.put_vars();
  // 制約ストア内に出現する変数も渡す
  psi.put_cs_vars(constraint_store);

  ml_.skip_pkt_until(RETURNPKT);
  
  int num;
  if(! ml_.MLGetInteger(&num)){
    std::cout << "MLGetInteger:unable to read the int from ml" << std::endl;
    throw MathLinkError("MLGetInteger", ml_.MLError());
  }
  if(debug_mode_) std::cout << "EntailmentCheckerInterval#num:" << num << std::endl;

  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1;
  */
  return true;
}


} //namespace symbolic_simulator
} // namespace hydla
