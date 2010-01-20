#include "MathematicaVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "mathlink_helper.h"
#include "Logger.h"
#include "PacketSender.h"
#include "PacketChecker.h"
#include "Types.h"

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
  HYDLA_LOGGER_DEBUG("------Variable map------\n", 
                     variable_map);

  variable_map_t::variable_list_t::const_iterator it  = variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = variable_map.end();
  for(; it!=end; ++it) {
    constraint_store_.init_vars.insert(
      std::make_pair(it->first, it->second));
  }

  HYDLA_LOGGER_DEBUG(constraint_store_);

  return true;
}

bool MathematicaVCSInterval::create_variable_map(variable_map_t& variable_map)
{
  // Intervalではcreate_variable_map関数無効
  assert(0);
  return false;
}

namespace {

struct MaxDiffMapDumper
{
  template<typename T>
  MaxDiffMapDumper(T it, T end)
  {
    for(; it!=end; ++it) {
      s << "name: " << it->first
        << "diff: " << it->second
        << "\n";      
    }
  }

  std::stringstream s;
};

}

void MathematicaVCSInterval::create_max_diff_map(
  PacketSender& ps, max_diff_map_t& max_diff_map)
{
  PacketSender::vars_const_iterator vars_it  = ps.vars_begin();
  PacketSender::vars_const_iterator vars_end = ps.vars_end();
  for(; vars_it!=vars_end; ++vars_it) {
    std::string name(vars_it->get<0>());
    int derivative_count = vars_it->get<1>();

    max_diff_map_t::iterator it = max_diff_map.find(name);
    if(it==max_diff_map.end()) {
      max_diff_map.insert(
        std::make_pair(name, derivative_count));
    }
    else if(it->second < derivative_count) {
      it->second = derivative_count;
    }    
  }

  HYDLA_LOGGER_DEBUG(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());

}

void MathematicaVCSInterval::send_init_cons(
  PacketSender& ps, const max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_DEBUG("---- Begin MathematicaVCSInterval::send_init_cons ----");
    
  // 送信する制約の個数を求める
  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  int init_vars_count = 0;
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);
    if(md_it!=max_diff_map.end() &&
       md_it->second  > init_vars_it->first.derivative_count) 
    {
      init_vars_count++;
    }
  }

  HYDLA_LOGGER_DEBUG("init_vars_count: ", init_vars_count);

  // Mathematicaへ送信
  ml_->put_function("List", init_vars_count);
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);
    if(md_it!=max_diff_map.end() &&
       md_it->second  > init_vars_it->first.derivative_count) 
    {
      ml_->put_function("Equal", 2);

      // 変数名
      ps.put_var(
        boost::make_tuple(init_vars_it->first.name, 
                          init_vars_it->first.derivative_count, 
                          false),
        PacketSender::VA_Zero);

      // 値
      ml_->put_function("ToExpression", 1);
      ml_->put_string(init_vars_it->second.str);      
    }
  }
}

void MathematicaVCSInterval::send_vars(
  PacketSender& ps, const max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_DEBUG("---- MathematicaVCSInterval::send_vars ----");

  max_diff_map_t::const_iterator it;
  max_diff_map_t::const_iterator end = max_diff_map.end();

  // 送信する個数
  int vars_count = 0;
  for(it=max_diff_map.begin(); it!=end; ++it) {
    for(int i=0; i<=it->second; i++) {
      vars_count++;
    }
  }

  HYDLA_LOGGER_DEBUG("count:", vars_count);

  ml_->put_function("List", vars_count);
  for(it=max_diff_map.begin(); it!=end; ++it) {
    for(int i=0; i<=it->second; i++) {
      ps.put_var(boost::make_tuple(it->first, i, false), 
                 PacketSender::VA_Time);

      HYDLA_LOGGER_DEBUG("put: ", 
                         "  name: ", it->first,
                         "  diff: ", i);
    }
  }  
}

VCSResult MathematicaVCSInterval::add_constraint(const tells_t& collected_tells)
{
  HYDLA_LOGGER_DEBUG("#*** Begin MathematicaVCSInterval::add_constraint ***");

  PacketSender ps(*ml_);

  // isConsistentInterval[expr, vars]を渡したい
  ml_->put_function("isConsistentInterval", 2);
  
  ml_->put_function("Join", 3);
  ml_->put_function("List", collected_tells.size());

  // tell制約の集合からtellsを得てMathematicaに渡す
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; (tells_it) != tells_end; ++tells_it) {
    ps.put_node((*tells_it)->get_child(), PacketSender::VA_Time, true);
  }

  // 制約ストアconstraintsをMathematicaに渡す
  send_cs(ps);

  max_diff_map_t max_diff_map;

  // 変数の最大微分回数をもとめる
  create_max_diff_map(ps, max_diff_map);
  
  // 初期値制約の送信
  send_init_cons(ps, max_diff_map);

  // 変数のリストを渡す
  send_vars(ps, max_diff_map);


  HYDLA_LOGGER_DEBUG("--- receive  ---");

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

  ml_->skip_pkt_until(RETURNPKT);

  VCSResult ret;
  switch(ml_->get_integer()) 
  {
    case 0: {
      HYDLA_LOGGER_DEBUG("consistent");
      ret = VCSR_TRUE;

      // 制約ストアにtell制約の追加
      constraint_store_.constraints.insert(
        constraint_store_.constraints.end(),
        collected_tells.begin(), 
        collected_tells.end());

      // 制約ストア中で使用される変数の一覧の更新
      PacketSender::vars_const_iterator ps_vars_it  = ps.vars_begin();
      PacketSender::vars_const_iterator ps_vars_end = ps.vars_end();
      for(; ps_vars_it!=ps_vars_end; ++ps_vars_it) {
        MathVariable mv;
        mv.name             = ps_vars_it->get<0>();
        mv.derivative_count = ps_vars_it->get<1>();
        constraint_store_.cons_vars.insert(mv);
      }
    }      
      break;

    case 1:
      HYDLA_LOGGER_DEBUG("over-constraint");
      ret = VCSR_FALSE;
      break;

    case 2:
      HYDLA_LOGGER_DEBUG("solver error");
      ret = VCSR_SOLVER_ERROR;
      break;

    default:
      assert(0);
  }

  HYDLA_LOGGER_DEBUG(constraint_store_);

  return ret;
}
  
VCSResult MathematicaVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
  HYDLA_LOGGER_DEBUG(
    "#*** MathematicaVCSInterval::check_entailment ***\n", 
    "ask: ", *negative_ask);

  PacketSender ps(*ml_);

  // checkEntailment[guard, store, vars]を渡したい
  ml_->put_function("checkEntailment", 3);

  // ask制約のガードの式を得てMathematicaに渡す
  ps.put_node(negative_ask->get_guard(), PacketSender::VA_Zero, true);

  // 制約ストアconstraintsをMathematicaに渡す
  ml_->put_function("List", constraint_store_.init_vars.size());
  constraint_store_t::init_vars_t::const_iterator 
    init_vars_it = constraint_store_.init_vars.begin();
  constraint_store_t::init_vars_t::const_iterator 
    init_vars_end = constraint_store_.init_vars.end();
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    ml_->put_function("Equal", 2);

    // 変数名
    ps.put_var(
      boost::make_tuple(init_vars_it->first.name, 
                        init_vars_it->first.derivative_count, 
                        false),
      PacketSender::VA_Zero);

    // 値
    ml_->put_function("ToExpression", 1);
    ml_->put_string(init_vars_it->second.str);      
  }

  

  // varsを渡す
  ps.put_vars(PacketSender::VA_Zero, true);

  ml_->MLEndPacket();

//   PacketChecker pc(*ml_);
//   pc.check();

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetType();
  
  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  int num  = ml_->get_integer();
  HYDLA_LOGGER_DEBUG("EntailmentChecker#num:", num);
  return num==1 ? VCSR_TRUE : VCSR_FALSE;
}

void MathematicaVCSInterval::send_ask_guards(
  PacketSender& ps, 
  const hydla::simulator::ask_set_t& asks) const
{
  // {ガードの式、askのID}のリスト形式で送信する

  ml_->put_function("List", asks.size());
  hydla::simulator::ask_set_t::const_iterator it  = asks.begin();
  hydla::simulator::ask_set_t::const_iterator end = asks.end();
  for(; it!=end; ++it)
  {
    HYDLA_LOGGER_DEBUG("send ask : ", **it);

    ml_->put_function("List", 2);    

    // guard条件を送る
    ps.put_node((*it)->get_guard(), PacketSender::VA_Time, true);

    // IDを送る
    ml_->MLPutInteger((*it)->get_id());
  }
}

bool MathematicaVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  HYDLA_LOGGER_DEBUG("#*** MathematicaVCSInterval::integrate ***");

  HYDLA_LOGGER_DEBUG(constraint_store_);

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
  PacketSender ps(*ml_);
  
  // integrateCalc[store, posAsk, negAsk, vars, maxTime]を渡したい
  ml_->put_function("integrateCalc", 5);

  ml_->put_function("Join", 2);

  // 制約ストアから式storeを得てMathematicaに渡す
  send_cs(ps);

  max_diff_map_t max_diff_map;

  // 変数の最大微分回数をもとめる
  create_max_diff_map(ps, max_diff_map);
  
  // 初期値制約の送信
  send_init_cons(ps, max_diff_map);

  // posAskを渡す（{ガードの式、askのID}をそれぞれ）
  HYDLA_LOGGER_DEBUG("-- send positive ask -- ");
  send_ask_guards(ps, positive_asks);

  // negAskを渡す（{ガードの式、askのID}をそれぞれ）
  HYDLA_LOGGER_DEBUG("-- send negative ask -- ");
  send_ask_guards(ps, negative_asks);

  // 変数のリストを渡す
  send_vars(ps, max_diff_map);

  // maxTimeを渡す
  time_t send_time(max_time);
  send_time -= current_time;
  HYDLA_LOGGER_DEBUG("send time:", send_time);
  send_time.send_time(*ml_);


////////////////// 受信処理

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

  ml_->skip_pkt_until(RETURNPKT);

  HYDLA_LOGGER_DEBUG("---integrate calc result---");
  integrate_result.states.resize(1);
  virtual_constraint_solver_t::IntegrateResult::NextPhaseState& state = 
    integrate_result.states.back();

//   PacketChecker pc(*ml_);
//   pc.check2();
  
//   while(ml_->MLGetType() != MLTKINT) {
//     int pkt = ml_->MLGetNext();
//     HYDLA_LOGGER_DEBUG("skip:", pkt);
//   }
//   int list_size = ml_->get_arg_count();
//   HYDLA_LOGGER_DEBUG("list_size : ", list_size);
    
  // List[制約一覧, 変化したaskとそのIDの組の一覧]が返る
  ml_->MLGetNext(); 
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
    //value.str = value.str.substr(15, value.str.size() - 16);
    HYDLA_LOGGER_DEBUG("value : ", value.str);
    ml_->MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }

  // askとそのIDの組一覧を得る
  int changed_asks_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("changed_asks_size : ", changed_asks_size);
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

  // max timeかどうか
  HYDLA_LOGGER_DEBUG("-- receive max time --");
//   PacketChecker c(*ml_);
//   c.check2();
  if(changed_asks_size==0) {
    ml_->MLGetNext();  
    ml_->MLGetNext();  
  }
  state.is_max_time = ml_->get_integer() == 1;
  HYDLA_LOGGER_DEBUG("is_max_time : ", state.is_max_time);

//   HYDLA_LOGGER_DEBUG(
//     "--- integrate result ---\n", 
//     integrate_result);

  return true;
}

void MathematicaVCSInterval::send_cs(PacketSender& ps) const
{
  HYDLA_LOGGER_DEBUG(
    "---- Send Constraint Store -----\n",
    "cons size: ", constraint_store_.constraints.size());

  ml_->put_function("List", 
                    constraint_store_.constraints.size());

  constraint_store_t::constraints_t::const_iterator 
    cons_it  = constraint_store_.constraints.begin();
  constraint_store_t::constraints_t::const_iterator 
    cons_end = constraint_store_.constraints.end();
  for(; (cons_it) != cons_end; ++cons_it) {
    ps.put_node((*cons_it)->get_child(), PacketSender::VA_Time, true);
  }
}

void MathematicaVCSInterval::send_cs_vars() const
{
  HYDLA_LOGGER_DEBUG("---- Send Constraint Store Vars -----");
}

std::ostream& MathematicaVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSInterval ***\n"
    << "--- constraint store ---\n";

//   std::set<MathVariable>::const_iterator vars_it = 
//     constraint_store_.second.begin();
//   std::set<std::set<MathValue> >::const_iterator or_cons_it = 
//     constraint_store_.first.begin();
//   while((or_cons_it) != constraint_store_.first.end())
//   {
//     std::set<MathValue>::const_iterator and_cons_it = 
//       (*or_cons_it).begin();
//     while((and_cons_it) != (*or_cons_it).end())
//     {
//       s << (*and_cons_it).str << " ";
//       and_cons_it++;
//     }
//     s << "\n";
//     or_cons_it++;
//   }

//   while((vars_it) != constraint_store_.second.end())
//   {
//     s << *(vars_it) << " ";
//     vars_it++;
//   }

  return s;
}

std::ostream& operator<<(std::ostream& s, 
                         const MathematicaVCSInterval::constraint_store_t& c)
{
  s << "---- MathematicaVCSInterval::consraint_store_t ----\n"
    << "-- init vars --\n";

  BOOST_FOREACH(
    const MathematicaVCSInterval::constraint_store_t::init_vars_t::value_type& i, 
    c.init_vars)
  {
    s << "variable: " << i.first
      << "   value: " << i.second
      << "\n";
  }

  s << "-- constraints --\n";
  BOOST_FOREACH(
    const MathematicaVCSInterval::constraint_store_t::constraints_t::value_type& i, 
    c.constraints)
  {
    s << *i;
  }
  
  return s;
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

