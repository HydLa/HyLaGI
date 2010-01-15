#include "Integrator.h"

#include "PacketChecker.h"
#include <iostream>

#include "Logger.h"

using namespace hydla::parse_tree;
using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {


Integrator::Integrator(MathLink& ml, bool debug_mode) :
  ml_(ml),
  debug_mode_(debug_mode)
{}

Integrator::~Integrator()
{}

/**
 * collected_tellsから、negative_asks内のask制約のガード条件が満たされるかどうか調べる
 * @param negative_ask     まだ展開されていないask制約1つ
 * @param collected_tells  tell制約のリスト（展開されたask制約の「=>」の右辺はここに追加される）
 * @return チェックの結果、そのask制約が展開されたかどうか
 */

IntegrateResult Integrator::integrate(
  ConstraintStoreInterval& constraint_store,
  positive_asks_t& positive_asks,
  negative_asks_t& negative_asks,
  const SymbolicTime& current_time,
  std::string max_time)
{
  HYDLA_LOGGER_DEBUG(
    "#*** Integrator ***\n",
    "--- positive asks ---\n",
    positive_asks,
    "--- negative asks ---\n",
    negative_asks,
    "--- current time ---\n",
    current_time,
    "--- max time ---\n",
    max_time);

  
// integrateCalc[store, posAsk, negAsk, vars, maxTime]を渡したい
  ml_.put_function("integrateCalc", 5);

  PacketSenderInterval psi(ml_, true);

  // 制約ストアから式storeを得てMathematicaに渡す
  psi.put_cs(constraint_store);

  // posAskを渡す（{ガードの式、askのID}をそれぞれ）
  int pos_asks_size = positive_asks.size();
  ml_.put_function("List", pos_asks_size);
  positive_asks_t::iterator pos_asks_it = positive_asks.begin();
  while((pos_asks_it) != positive_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send pos ask : ", **pos_asks_it);

    ml_.put_function("List", 2);    
    psi.put_node((*pos_asks_it)->get_guard());
    // IDを送る
    int pos_id = (*pos_asks_it)->get_id();
    ml_.MLPutInteger(pos_id);

    pos_asks_it++;
  }

  // negAskを渡す（{ガードの式、askのID}をそれぞれ）
  int neg_asks_size = negative_asks.size();
  ml_.put_function("List", neg_asks_size);
  negative_asks_t::iterator neg_asks_it = negative_asks.begin();
  while((neg_asks_it) != negative_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send neg ask : ", **neg_asks_it);

    ml_.put_function("List", 2);
    psi.put_node((*neg_asks_it)->get_guard());
    // IDを送る
    int neg_id = (*neg_asks_it)->get_id();
    ml_.MLPutInteger(neg_id);

    neg_asks_it++;
  }

  // varsを渡す
  ml_.put_function("DeleteDuplicates", 1); // 重複を回避。要修正。
  ml_.put_function("Join", 2);
  psi.put_vars();
  // 制約ストア内に出現する変数も渡す
  psi.put_cs_vars(constraint_store);

  // maxTimeを渡す
  ml_.put_function("ToExpression", 1);
  ml_.put_string(max_time);


  // 返ってくるパケットを解析
//  PacketChecker pc(ml_);
//  pc.check();

  ml_.skip_pkt_until(RETURNPKT);

  HYDLA_LOGGER_DEBUG("---integrate calc result---");
  IntegrateResult integrate_result;
  integrate_result.states.resize(1);
  NextPointPhaseState& state = integrate_result.states.back();

  ml_.MLGetNext(); // List関数
  int list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("list_size : ", list_size);
  
  // List[制約一覧, 変化したaskとそのIDの組の一覧]が返る
  ml_.MLGetNext(); // Listという関数名
  ml_.MLGetNext(); // Listの中の先頭要素

  // next_point_phase_timeを得る
  SymbolicTime next_point_phase_time;
  state.next_point_phase_time.str = ml_.get_string();
  HYDLA_LOGGER_DEBUG("next_point_phase_time : ", state.next_point_phase_time.str);  
  ml_.MLGetNext(); // Listという関数名
  
  // 変数表の作成
  int variable_list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("variable_list_size : ", variable_list_size);  
  ml_.MLGetNext(); ml_.MLGetNext();
  for(int i=0; i<variable_list_size; i++)
  {
    ml_.MLGetNext(); ml_.MLGetNext();

    SymbolicVariable variable;
    SymbolicValue    value;

    HYDLA_LOGGER_DEBUG("--- add variable ---");

    // 変数名
    variable.name = ml_.get_symbol();
    HYDLA_LOGGER_DEBUG("name  : ", variable.name);
    ml_.MLGetNext();

    // 微分回数
    variable.derivative_count = ml_.get_integer();
    HYDLA_LOGGER_DEBUG("derivative : ", variable.derivative_count);
    ml_.MLGetNext();

    // 値
    value.str = ml_.get_string();
    HYDLA_LOGGER_DEBUG("value : ", value.str);
    ml_.MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }

  // askとそのIDの組一覧を得る
  int changed_asks_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("changed_asks_size : ", changed_asks_size);
  state.is_max_time = changed_asks_size > 0; // 変化したaskがない場合はmax_timeに達した場合である
  HYDLA_LOGGER_DEBUG("is_max_time : ", state.is_max_time);
  ml_.MLGetNext(); // List関数
  ml_.MLGetNext(); // Listという関数名
  for(int j=0; j<changed_asks_size; j++)
  {
    HYDLA_LOGGER_DEBUG("--- add changed ask ---");

    ml_.MLGetNext(); // List関数
    ml_.MLGetNext(); // Listという関数名
    std::string changed_ask_type_str = ml_.get_symbol(); // pos2negまたはneg2pos
    HYDLA_LOGGER_DEBUG("changed_ask_type_str : ", changed_ask_type_str);

    AskState changed_ask_type;
    if(changed_ask_type_str == "pos2neg"){
      changed_ask_type = Positive2Negative;
    }
    else if(changed_ask_type_str == "neg2pos") {
      changed_ask_type = Negative2Positive;
    }
    else {
      assert(0);
    }

    int changed_ask_id = ml_.get_integer();
    HYDLA_LOGGER_DEBUG("changed_ask_id : ", changed_ask_id);

    integrate_result.ask_list.push_back(
      std::make_pair(changed_ask_type, changed_ask_id));
  }

  HYDLA_LOGGER_DEBUG(
    "--- integrate result ---\n", 
    integrate_result);


  return integrate_result;
}


} //namespace symbolic_simulator
} // namespace hydla
