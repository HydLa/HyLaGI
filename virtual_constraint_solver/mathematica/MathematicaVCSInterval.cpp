#include "MathematicaVCSInterval.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"
#include "PacketSender.h"

using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSInterval::MathematicaVCSInterval(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSInterval::~MathematicaVCSInterval()
{}

bool MathematicaVCSInterval::reset()
{
  constraint_store_ = constraint_store_t();
  return true;
}

bool MathematicaVCSInterval::reset(const variable_map_t& variable_map)
{
  HYDLA_LOGGER_DEBUG("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_DEBUG("no Variables");
    return true;
  }
  HYDLA_LOGGER_DEBUG("------Variable map------", variable_map);

  MathValue symbolic_value;
  std::string value;
  MathVariable symbolic_variable;
  std::string variable_name;

  variable_map_t::variable_list_t::const_iterator it = variable_map.begin();

  std::set<MathValue> value_set;
  while(it != variable_map.end())
  {
    symbolic_value = (*it).second;    
    value = symbolic_value.str;
    if(value != "") break;
    it++;
  }

  while(it != variable_map.end())
  {
    symbolic_variable = (*it).first;
    symbolic_value = (*it).second;    
    variable_name = symbolic_variable.name;
    value = symbolic_value.str;

    std::string str = "";

    // MathVariable側に関する文字列を作成
    str += "Equal[";
    if(symbolic_variable.derivative_count > 0)
    {
      std::ostringstream derivative_count;
      derivative_count << symbolic_variable.derivative_count;
      str += "Derivative[";
      str += derivative_count.str();
      str += "][usrVar";
      str += variable_name;
      str += "][0]";
    }
    else
    {
      str += "usrVar";
      str += variable_name;
      str += "[0]";
    }

    str += ",";

    // MathValue側に関する文字列を作成
    str += value;
    str += "]"; // Equalの閉じ括弧

    MathValue new_symbolic_value;
    new_symbolic_value.str = str;
    value_set.insert(new_symbolic_value);


    // 制約ストア内の変数一覧を作成
    symbolic_variable.name = "usrVar" + variable_name;
    constraint_store_.second.insert(symbolic_variable);

    it++;
    while(it != variable_map.end())
    {
      symbolic_value = (*it).second;
      value = symbolic_value.str;
      if(value != "") break;
      it++;
    }
  }
  constraint_store_.first.insert(value_set);

  HYDLA_LOGGER_DEBUG(*this);

  return true;
}

bool MathematicaVCSInterval::create_variable_map(variable_map_t& variable_map)
{
  //TODO: coutに出力するのをやめる

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  // Orでつながった制約のうち、最初の1つだけを採用することにする
  std::set<MathValue>::const_iterator and_cons_it = (*or_cons_it).begin();
  while((and_cons_it) != (*or_cons_it).end())
  {
    std::string cons_str = (*and_cons_it).str;
    // cons_strは"Equal[usrVarx,2]"や"Equal[Derivative[1][usrVary],3]"など

    unsigned int loc = cons_str.find("Equal[", 0);
    loc += 6; // 文字列"Equal["の長さ分
    unsigned int comma_loc = cons_str.find(",", loc);
    if(comma_loc == std::string::npos)
    {
      std::cout << "can't find comma." << std::endl;
      return false;
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
        return false;
      }
      std::string variable_derivative_count_str = variable_str.substr(variable_loc, bracket_loc-variable_loc);
      variable_derivative_count = std::atoi(variable_derivative_count_str.c_str());
      variable_loc = bracket_loc + 2; // "]["の長さ分
      bracket_loc = variable_str.find("]", variable_loc);
      if(bracket_loc == std::string::npos)
      {
        std::cout << "can't find bracket." << std::endl;
        return false;
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
      return false;
    }
    std::string value_str = cons_str.substr(comma_loc + 1, end_loc - (comma_loc + 1));

    MathVariable symbolic_variable;
    MathValue symbolic_value;
    symbolic_variable.name = variable_name;
    symbolic_variable.derivative_count = variable_derivative_count;
    symbolic_value.str = value_str;

    variable_map.set_variable(symbolic_variable, symbolic_value); 
    and_cons_it++;
  }

  // [t]を除く処理は要らなさそう？

  return true;
}

VCSResult MathematicaVCSInterval::add_constraint(const tells_t& collected_tells)
{
  // isConsistentInterval[tells, store, tellsVars, storeVars]を渡したい
  ml_->put_function("isConsistentInterval", 4);

  // tell制約の集合からtellsを得てMathematicaに渡す
  int tells_size = collected_tells.size();
  ml_->put_function("List", tells_size);
  tells_t::const_iterator tells_it = collected_tells.begin();
  PacketSender psi(*ml_, PacketSender::NP_INTERVAL_PHASE);

  while((tells_it) != collected_tells.end())
  {
    psi.visit((*tells_it));
    tells_it++;
  }

  // 制約ストアstoreをMathematicaに渡す
  send_cs();

  // TODO: 初期値制約の送り方が間違えている
  
  // tellsvarsを渡す
  psi.put_vars();

  // storevarsを渡す
  send_cs_vars();

  // 結果を受け取る前に制約ストアを初期化
  reset();

  ml_->skip_pkt_until(RETURNPKT);

  // 解けなかった場合は0が返る（制約間に矛盾がある、またはover-constraintということ）
  if(ml_->MLGetType() == MLTKINT)
  {
    HYDLA_LOGGER_DEBUG("over-constraint");
    ml_->MLNewPacket();
    return VCSR_FALSE;
  }

  // List[数値, 制約一覧, 変数一覧]が返る
  // 数値部分は問題なく解けたら1、under-constraintが起きていれば2が返る

  ml_->MLGetNext(); // Listという関数名
  ml_->MLGetNext(); // Listの中の先頭（数値）

  int n = ml_->get_integer();

  HYDLA_LOGGER_DEBUG(
    n==2 ? "over-constraint" : "", 
    "---build constraint store---");

  ml_->MLGetNext(); // List関数

  // List関数の要素数（制約の個数）を得る
  int cons_size = ml_->get_arg_count();
  ml_->MLGetNext(); // Listという関数名
  ml_->MLGetNext(); // Listの中の先頭要素

  std::set<MathValue> value_set;    
  for(int i=0; i<cons_size; i++)
  {
    std::string str = ml_->get_string();
    MathValue symbolic_value;
    symbolic_value.str = str;
    value_set.insert(symbolic_value);
  }
  constraint_store_.first.insert(value_set);


  // 出現する変数の一覧が文字列で返ってくるのでそれを制約ストアに入れる
  ml_->MLGetNext(); // List関数

  // List関数の要素数（変数一覧に含まれる変数の個数）を得る
  int vars_size = ml_->get_arg_count();
  ml_->MLGetNext(); // Listという関数名

  for(int k=0; k<vars_size; k++)
  {
    MathVariable symbolic_variable;
    std::string sym;
    ml_->MLGetNext(); // Derivative[number][変数名][]またはx[]などの関数
    switch(ml_->MLGetNext()) // Derivative[number][変数名]またはxという関数名
    {
      case MLTKFUNC:
        ml_->MLGetNext(); // Derivative[number]という関数名
        ml_->MLGetNext(); // Derivativeという関数名
        ml_->MLGetNext(); // number
        symbolic_variable.derivative_count = ml_->get_integer();
        ml_->MLGetNext(); // 変数名
        symbolic_variable.name = ml_->get_symbol();
        ml_->MLGetNext(); // t
        break;
      case MLTKSYM:
        sym = ml_->get_symbol();
        symbolic_variable.derivative_count = 0;
        ml_->MLGetNext(); // t
        symbolic_variable.name = sym;
        break;
      default:
        ;
    }
    constraint_store_.second.insert(symbolic_variable);
  }
  ml_->MLNewPacket(); // エラー回避用。エラーの原因不明

  HYDLA_LOGGER_DEBUG(*this);

  return n >= 1 ? VCSR_TRUE : VCSR_FALSE;
}
  
VCSResult MathematicaVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
  // checkEntailment[guard, store, vars]を渡したい
  ml_->put_function("checkEntailment", 3);


  // ask制約のガードの式を得てMathematicaに渡す
  PacketSender psi(*ml_, PacketSender::NP_INTERVAL_PHASE);
  psi.put_node(negative_ask);

  // 制約ストアから式storeを得てMathematicaに渡す
  send_cs();

  // varsを渡す
  ml_->put_function("Join", 2);
  psi.put_vars();
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();

  ml_->skip_pkt_until(RETURNPKT);
  
  int num = ml_->get_integer();
  HYDLA_LOGGER_DEBUG("EntailmentCheckerInterval#num:", num);

  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1 ? VCSR_TRUE : VCSR_FALSE;
}

bool MathematicaVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  //TODO: 初期値制約の送り方に問題あり

  HYDLA_LOGGER_DEBUG("#*** MathematicaVCSInterval::integrate ***");

//   HYDLA_LOGGER_DEBUG(
//     "#*** Integrator ***\n",
//     "--- positive asks ---\n",
//     positive_asks,
//     "--- negative asks ---\n",
//     negative_asks,
//     "--- current time ---\n",
//     current_time,
//     "--- max time ---\n",
//     max_time);

////////////////// 送信処理
  PacketSender psi(*ml_, PacketSender::NP_INTERVAL_PHASE);
  
  // integrateCalc[store, posAsk, negAsk, vars, maxTime]を渡したい
  ml_->put_function("integrateCalc", 5);

  // 制約ストアから式storeを得てMathematicaに渡す
  send_cs();

  // posAskを渡す（{ガードの式、askのID}をそれぞれ）
  int pos_asks_size = positive_asks.size();
  ml_->put_function("List", pos_asks_size);
  positive_asks_t::const_iterator pos_asks_it = positive_asks.begin();
  while((pos_asks_it) != positive_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send pos ask : ", **pos_asks_it);

    ml_->put_function("List", 2);    
    psi.put_node((*pos_asks_it)->get_guard());
    // IDを送る
    int pos_id = (*pos_asks_it)->get_id();
    ml_->MLPutInteger(pos_id);

    pos_asks_it++;
  }

  // negAskを渡す（{ガードの式、askのID}をそれぞれ）
  int neg_asks_size = negative_asks.size();
  ml_->put_function("List", neg_asks_size);
  negative_asks_t::const_iterator neg_asks_it = negative_asks.begin();
  while((neg_asks_it) != negative_asks.end())
  {
    HYDLA_LOGGER_DEBUG("send neg ask : ", **neg_asks_it);

    ml_->put_function("List", 2);
    psi.put_node((*neg_asks_it)->get_guard());
    // IDを送る
    int neg_id = (*neg_asks_it)->get_id();
    ml_->MLPutInteger(neg_id);

    neg_asks_it++;
  }

  // varsを渡す
  ml_->put_function("DeleteDuplicates", 1); // 重複を回避。要修正。
  ml_->put_function("Join", 2);
  psi.put_vars();
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();

  // maxTimeを渡す
  ml_->put_function("ToExpression", 1);
  time_t send_time(max_time);
  send_time -= current_time;
  send_time.send_time(*ml_);

  ml_->skip_pkt_until(RETURNPKT);

////////////////// 受信処理

  HYDLA_LOGGER_DEBUG("---integrate calc result---");
  integrate_result.states.resize(1);
  virtual_constraint_solver_t::IntegrateResult::NextPhaseState& state = 
    integrate_result.states.back();

  ml_->MLGetNext(); // List関数
  int list_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("list_size : ", list_size);
  
  // List[制約一覧, 変化したaskとそのIDの組の一覧]が返る
  ml_->MLGetNext(); // Listという関数名
  ml_->MLGetNext(); // Listの中の先頭要素

  // next_point_phase_timeを得る
  MathTime next_phase_time;
  state.time.receive_time(*ml_);
  state.time += current_time;
  HYDLA_LOGGER_DEBUG("next_phase_time : ", state.time);  
  ml_->MLGetNext(); // Listという関数名
  
  // 変数表の作成
  int variable_list_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("variable_list_size : ", variable_list_size);  
  ml_->MLGetNext(); ml_->MLGetNext();
  for(int i=0; i<variable_list_size; i++)
  {
    ml_->MLGetNext(); 
    ml_->MLGetNext();

    MathVariable variable;
    MathValue    value;

    HYDLA_LOGGER_DEBUG("--- add variable ---");

    // 変数名
    variable.name = ml_->get_symbol().substr(6);
    HYDLA_LOGGER_DEBUG("name  : ", variable.name);
    ml_->MLGetNext();

    // 微分回数
    variable.derivative_count = ml_->get_integer();
    HYDLA_LOGGER_DEBUG("derivative : ", variable.derivative_count);
    ml_->MLGetNext();

    // 値
    value.str = ml_->get_string();
    // Function[List, .....] をのぞく
    // TODO: こんな処理本当はいならないはず
    value.str = value.str.substr(15, value.str.size() - 16);
    HYDLA_LOGGER_DEBUG("value : ", value.str);
    ml_->MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }

  // askとそのIDの組一覧を得る
  int changed_asks_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("changed_asks_size : ", changed_asks_size);
  state.is_max_time = changed_asks_size == 0; // 変化したaskがない場合はmax_timeに達した場合である
  HYDLA_LOGGER_DEBUG("is_max_time : ", state.is_max_time);

  if(changed_asks_size>0) {
  ml_->MLGetNext(); // List関数
  ml_->MLGetNext(); // Listという関数名
  }
  for(int j=0; j<changed_asks_size; j++)
  {
    HYDLA_LOGGER_DEBUG("--- add changed ask ---");

    ml_->MLGetNext(); // List関数
    ml_->MLGetNext(); // Listという関数名
    std::string changed_ask_type_str = ml_->get_symbol(); // pos2negまたはneg2pos
    HYDLA_LOGGER_DEBUG("changed_ask_type_str : ", changed_ask_type_str);

    hydla::simulator::AskState changed_ask_type;
    if(changed_ask_type_str == "pos2neg"){
      changed_ask_type = hydla::simulator::Positive2Negative;
    }
    else if(changed_ask_type_str == "neg2pos") {
      changed_ask_type = hydla::simulator::Negative2Positive;
    }
    else {
      assert(0);
    }

    int changed_ask_id = ml_->get_integer();
    HYDLA_LOGGER_DEBUG("changed_ask_id : ", changed_ask_id);

    integrate_result.changed_asks.push_back(
      std::make_pair(changed_ask_type, changed_ask_id));
  }

//   HYDLA_LOGGER_DEBUG(
//     "--- integrate result ---\n", 
//     integrate_result);

  return true;
}

void MathematicaVCSInterval::send_cs() const
{
  HYDLA_LOGGER_DEBUG("---- Send Constraint Store -----");

  int or_cons_size = constraint_store_.first.size();
  if(or_cons_size <= 0)
  {
    HYDLA_LOGGER_DEBUG("no Constraints");
    ml_->put_function("List", 0);
    return;
  }

  std::set<std::set<MathValue> >::const_iterator or_cons_it;
  std::set<MathValue>::const_iterator and_cons_it;
//     or_cons_it = constraint_store_.first.begin();
//     while((or_cons_it) != constraint_store_.first.end())
//     {
//       and_cons_it = (*or_cons_it).begin();
//       while((and_cons_it) != (*or_cons_it).end())
//       {
//         std::cout << (*and_cons_it).str << " ";
//         and_cons_it++;
//       }
//       std::cout << std::endl;
//       or_cons_it++;
//     }

//     if(debug_mode_) {
//       std::cout << "----------------------------" << std::endl;
//     }

  ml_->put_function("List", 1);
  ml_->put_function("Or", or_cons_size);
  or_cons_it = constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    int and_cons_size = (*or_cons_it).size();
    ml_->put_function("And", and_cons_size);
    and_cons_it = (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      ml_->put_function("ToExpression", 1);
      std::string str = (*and_cons_it).str;
      ml_->put_string(str);
      and_cons_it++;
    }
    or_cons_it++;
  }
}

void MathematicaVCSInterval::send_cs_vars() const
{
  HYDLA_LOGGER_DEBUG("---- Send Constraint Store Vars -----");

  int vars_size = constraint_store_.second.size();
  std::set<MathVariable>::const_iterator vars_it = 
    constraint_store_.second.begin();

  ml_->put_function("List", vars_size);
  while((vars_it) != constraint_store_.second.end())
  {
    if(int value = (*vars_it).derivative_count > 0)
    {
      ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative[*number*], arg f
      ml_->MLPutArgCount(1);      // this 1 is for the 'f'
      ml_->MLPutNext(MLTKFUNC);   // The func we are putting has head Derivative, arg 2
      ml_->MLPutArgCount(1);      // this 1 is for the '*number*'
      ml_->put_symbol("Derivative");
      ml_->MLPutInteger(value);
      ml_->put_symbol((*vars_it).name);

      HYDLA_LOGGER_DEBUG("Derivative[", value, "][", (*vars_it).name, "]");
    }
    else
    {
      ml_->put_symbol((*vars_it).name);
        
      HYDLA_LOGGER_DEBUG((*vars_it).name);
    }
    vars_it++;
  }
}

std::ostream& MathematicaVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSInterval ***\n"
    << "--- constraint store ---\n";

  std::set<MathVariable>::const_iterator vars_it = 
    constraint_store_.second.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      s << (*and_cons_it).str << " ";
      and_cons_it++;
    }
    s << "\n";
    or_cons_it++;
  }

  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << " ";
    vars_it++;
  }

  return s;
}

std::ostream& operator<<(std::ostream& s, const MathematicaVCSInterval& m)
{
  return m.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

