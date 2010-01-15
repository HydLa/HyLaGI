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

  ml_.MLGetNext(); // List関数
  int list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("list_size : ", list_size);
  
  // List[制約一覧, 変化したaskとそのIDの組の一覧]が返る
  ml_.MLGetNext(); // Listという関数名

  // next_point_phase_timeを得る
  ml_.MLGetNext(); // Listの中の先頭要素
  SymbolicTime next_point_phase_time;
  next_point_phase_time.str = ml_.get_string();
  HYDLA_LOGGER_DEBUG("next_point_phase_time : ", next_point_phase_time.str);  

  // 制約一覧を得る
  ml_.MLGetNext(); // Listという関数名
  // List関数の要素数（制約の個数）を得る
  int variable_list_size = ml_.get_arg_count();
  HYDLA_LOGGER_DEBUG("variable_list_size : ", variable_list_size);  
  ml_.MLGetNext(); // List中の先頭要素

  variable_map_t variable_map;
  for(int i=0; i<variable_list_size; i++)
  {
    ml_.MLGetNext(); ml_.MLGetNext();
    
    std::string name = ml_.get_symbol();
    int derivative = ml_.get_integer();
    std::string value = ml_.get_string();

    HYDLA_LOGGER_DEBUG("name  : ", name);

//    std::string variable_value = ml_.get_string();

//     HYDLA_LOGGER_DEBUG(
//       "name  : ", variable_name,
//       "value : ", variable_value);

//    tmp_cons.insert(str);
  }
  ml_.MLGetNext(); // List関数
/*

  // askとそのIDの組一覧を得る
  int changed_asks_size;
  if(! ml_.MLGetArgCount(&changed_asks_size)){
    std::cout << "#funcCase:MLGetArgCount:unable to get the number of arguments from ml" << std::endl;
    throw MathLinkError("MLGetArgCount", ml_.MLError());
  }
  bool is_max_time = changed_asks_size > 0; // 変化したaskがない場合はmax_timeに達した場合である
  ml_.MLGetNext(); // Listという関数名
  ask_list_t changed_asks;
  for(int j=0; j<changed_asks_size; j++)
  {
    ml_.MLGetNext(); // List関数
    ml_.MLGetNext(); // Listという関数名
    std::string changed_ask_type_str = ml_.get_symbol(); // pos2negまたはneg2pos
    AskState changed_ask_type;
    if(changed_ask_type_str == "pos2neg"){
      changed_ask_type = Positive2Negative;
    }
    else{
      changed_ask_type = Negative2Positive;
    }
    int changed_ask_id;
    if(! ml_.MLGetInteger(&changed_ask_id)){
      HYDLA_LOGGER_ERROR("MLGetInteger:unable to read the int from ml");
      throw MathLinkError("MLGetInteger", ml_.MLError());
    }
    changed_asks.push_back(std::make_pair(changed_ask_type, changed_ask_id));
  }


  // tmp_consから変数表をつくる
  std::set<std::string>::iterator tmp_cons_it = tmp_cons.begin();
  variable_map_t variable_map;
  while((tmp_cons_it) != tmp_cons.end())
  {
    std::string cons_str = (*tmp_cons_it);
    // cons_strは"Equal[usrVarx,2]"や"Equal[Derivative[1][usrVary],3]"など

    unsigned int loc = cons_str.find("Equal[", 0);
    loc += 6; // 文字列"Equal["の長さ分
    unsigned int comma_loc = cons_str.find(",", loc);
    if(comma_loc == std::string::npos)
    {
      std::cout << "can't find comma." << std::endl;
//      return;
    }
    std::string variable_str = cons_str.substr(loc, comma_loc-loc);
    // variable_strは"usrVarx"や"Derivative[1][usrVarx]"など

    // nameとderivative_countへの分離
    std::string variable_name;
    int variable_derivative_count;
    unsigned int variable_loc = variable_str.find("Derivative[", 0);
    if(variable_loc != std::string::npos)
    {
      variable_loc += 11; // "Derivative["の長さ分
      unsigned int bracket_loc = variable_str.find("][", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
//        return;
      }
      std::string variable_derivative_count_str = variable_str.substr(variable_loc, bracket_loc-variable_loc);
      variable_derivative_count = std::atoi(variable_derivative_count_str.c_str());
      variable_loc = bracket_loc + 2; // "]["の長さ分
      bracket_loc = variable_str.find("]", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
//        return;
      }
      variable_loc += 6; // "usrVar"の長さ分
      variable_name =  variable_str.substr(variable_loc, bracket_loc-variable_loc);
    }
    else
    {
      variable_name =  variable_str.substr(6); // "usrVar"の長さ分
      variable_derivative_count = 0;
    }

    // 値の取得
    int str_size = cons_str.size();
    unsigned int end_loc = cons_str.rfind("]", str_size-1);

    if(end_loc == std::string::npos)
    {
      std::cout << "can't find bracket." << std::endl;
//      return;
    }
    std::string value_str = cons_str.substr(comma_loc + 1, end_loc - (comma_loc + 1));

    SymbolicVariable symbolic_variable;
    SymbolicValue symbolic_value;
    symbolic_variable.name = variable_name;
    symbolic_variable.derivative_count = variable_derivative_count;
    symbolic_value.str = value_str;

    variable_map.set_variable(symbolic_variable, symbolic_value); 
    tmp_cons_it++;
  }

  NextPointPhaseState next_point_phase_state;
  next_point_phase_state.next_point_phase_time = next_point_phase_time;
  next_point_phase_state.variable_map = variable_map;
  next_point_phase_state.is_max_time = is_max_time;
  std::vector<NextPointPhaseState> next_point_phase_states_vector;
  next_point_phase_states_vector.push_back(next_point_phase_state);
*/
  IntegrateResult integrate_result;
  //integrate_result.states = next_point_phase_states_vector;
  //integrate_result.ask_list = changed_asks;

  return integrate_result;
}


} //namespace symbolic_simulator
} // namespace hydla
