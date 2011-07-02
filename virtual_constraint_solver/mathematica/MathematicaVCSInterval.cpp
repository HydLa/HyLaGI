#include "MathematicaVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "Logger.h"
#include "PacketChecker.h"
#include "PacketErrorHandler.h"
#include "MathematicaExpressionConverter.h"

using namespace hydla::vcs;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace mathematica {

namespace {

struct MaxDiffMapDumper
{
  template<typename T>
  MaxDiffMapDumper(T it, T end)
  {
    for(; it!=end; ++it) {
      s << "name: " << it->first
        << "  diff: " << it->second
        << "\n";      
    }
  }

  std::stringstream s;
};

}


MathematicaVCSInterval::MathematicaVCSInterval(MathLink* ml, int approx_precision) :
  ml_(ml),
  approx_precision_(approx_precision)
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

  HYDLA_LOGGER_VCS_SUMMARY("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", 
                     variable_map);

  variable_map_t::variable_list_t::const_iterator it  = variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = variable_map.end();
  for(; it!=end; ++it) {
    if(!it->second.is_undefined()) {
      constraint_store_.init_vars.insert(
        std::make_pair(it->first, it->second));
    }

    else {
      value_t value;
      constraint_store_.init_vars.insert(
        std::make_pair(it->first, value));
    }

    // 初期値制約に関するmax_diff_mapに追加
    constraint_store_t::init_vars_max_diff_map_t::iterator ivmd_it = 
      constraint_store_.init_vars_max_diff_map.find(it->first.name);
    if(ivmd_it==constraint_store_.init_vars_max_diff_map.end()) 
    {
      constraint_store_.init_vars_max_diff_map.insert(
        std::make_pair(it->first.name, it->first.derivative_count));
    }
    else if(it->first.derivative_count > ivmd_it->second)
    {
      ivmd_it->second = it->first.derivative_count;
    }

  }

  HYDLA_LOGGER_VCS(constraint_store_);

  return true;
}



bool MathematicaVCSInterval::reset(const variable_map_t& variable_map,  const parameter_map_t& parameter_map)
{
  if(!reset(variable_map)){
    return false;
  }


  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", 
                     parameter_map);

  parameter_map_=parameter_map;

  return true;
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

  HYDLA_LOGGER_VCS(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());

}

void MathematicaVCSInterval::send_init_cons(
  PacketSender& ps, 
  const max_diff_map_t& max_diff_map,
  bool use_approx)
{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSInterval::send_init_cons ----");
    
  // 送信する制約の個数を求める
  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  int init_vars_count = 0;

  // 送る必要のある初期値制約をカウント
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);


    // 初期値制約のうち、集めたtell制約に出現する際の最大微分回数よりも小さい微分回数のものをカウントする
    if(md_it!=max_diff_map.end() && 
       md_it->second  > init_vars_it->first.derivative_count)
    {
      init_vars_count++;
    }


/*
    // 初期値制約のうち、集めたtell制約に出現しないものをカウント
    if(md_it==max_diff_map.end())
    {
      init_vars_count++;
    }
    // 初期値制約のうち、集めたtell制約に出現する際の最大微分回数よりも小さい微分回数のものをカウントする
    else if(md_it->second  > init_vars_it->first.derivative_count)
    {
      init_vars_count++;
    }
*/


  }


  HYDLA_LOGGER_VCS("init_vars_count: ", init_vars_count);


  // 制約をMathematicaへ送信
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
        if(use_approx && approx_precision_ > 0) {
          // 近似して送信
          ml_->put_function("approxExpr", 2);
          ml_->put_integer(approx_precision_);
        }
        ps.put_node(init_vars_it->second.get_node(), PacketSender::VA_None, false);
    }

  }

}



void MathematicaVCSInterval::send_parameter_cons() const{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSInterval::send_parameter_cons ----");


  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();
  int para_size=0;
  for(; par_it!=par_end; ++par_it) {
    value_range_t::or_vector::const_iterator or_it = par_it->second.or_begin(), or_end = par_it->second.or_end();
    for(;or_it != or_end; or_it++){
      para_size += or_it->size();
    }
  }

  HYDLA_LOGGER_VCS("parameter_cons_count: ", para_size);
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", 
                     parameter_map_);
  
  // 制約をMathematicaへ送信
  ml_->put_function("List", para_size);
  
  for(par_it = parameter_map_.begin(); par_it!=par_end; ++par_it) {
    value_range_t::or_vector::const_iterator or_it = par_it->second.or_begin(), or_end = par_it->second.or_end();
    for(;or_it != or_end; or_it++){
      value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
      for(; and_it != and_end; and_it++){
      
        ml_->put_function(
          MathematicaExpressionConverter::get_relation_math_string(and_it->relation), 2);

        // 変数名
        ml_->put_symbol(PacketSender::par_prefix + par_it->first.get_name());

        // 値
        ml_->put_function("ToExpression", 1);
        ml_->put_string(and_it->value.get_string());
      }
    }
  }
}


void MathematicaVCSInterval::send_pars() const{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSInterval::send_pars ----");


  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();
  
  // 制約をMathematicaへ送信
  ml_->put_function("List", parameter_map_.size());
  
  for(par_it = parameter_map_.begin(); par_it!=par_end; ++par_it) {
    ml_->put_symbol(PacketSender::par_prefix + par_it->first.get_name());
  }
}



void MathematicaVCSInterval::send_vars(
  PacketSender& ps, const max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- MathematicaVCSInterval::send_vars ----");

  ml_->put_function("Join", 2);
    
  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();

  // 送信する個数
  int vars_count = 0;
  for(; md_it!=md_end; ++md_it) {
    for(int i=0; i<=md_it->second; i++) {
      vars_count++;
    }
  }

  HYDLA_LOGGER_VCS("count:", vars_count);

  ml_->put_function("List", vars_count);
  for(md_it=max_diff_map.begin(); md_it!=md_end; ++md_it) {
    for(int i=0; i<=md_it->second; i++) {
      ps.put_var(boost::make_tuple(md_it->first, i, false), 
                 PacketSender::VA_Time);

      HYDLA_LOGGER_VCS("put: ", 
                         "  name: ", md_it->first,
                         "  diff: ", i);
    }
  }


  ml_->put_function("List", 0);


/*
  // 制約ストア中の変数のうち、集めたtell制約中に含まれないものも追加で送信
  constraint_store_t::cons_vars_t::const_iterator cs_vars_it = constraint_store_.cons_vars.begin();
  constraint_store_t::cons_vars_t::const_iterator cs_vars_end = constraint_store_.cons_vars.end();

  ml_->put_function("List",  constraint_store_.cons_vars.size());

  HYDLA_LOGGER_VCS("cs_vars_count:",  constraint_store_.cons_vars.size());

  for(; cs_vars_it!=cs_vars_end; ++cs_vars_it){
      ps.put_var(boost::make_tuple(cs_vars_it->name, cs_vars_it->derivative_count, false), 
                 PacketSender::VA_Time);

      HYDLA_LOGGER_VCS("put: ", 
                         "  name: ", cs_vars_it->name,
                         "  diff: ", cs_vars_it->derivative_count);
  }
*/

}

VCSResult MathematicaVCSInterval::add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSInterval::add_constraint ***");

  PacketSender ps(*ml_);

  // isConsistentInterval[expr, vars]を渡したい
  ml_->put_function("isConsistentInterval", 2);

  // expr部分
  ml_->put_function("Join", 4);
  ml_->put_function("List", collected_tells.size() + appended_asks.size());

  HYDLA_LOGGER_VCS(
    "tells_size:", collected_tells.size() + appended_asks.size());

  // tell制約の集合からtellsを得てMathematicaに渡す
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; (tells_it) != tells_end; ++tells_it) {
    ps.put_node((*tells_it)->get_child(), PacketSender::VA_Time, true);
  }
  
  // appended_asksからガード部分を得てMathematicaに渡す
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    ps.put_node(append_it->ask->get_guard(), PacketSender::VA_Time, true, append_it->entailed);
  }

  // 制約ストアconstraintsをMathematicaに渡す
  send_cs(ps);
  send_parameter_cons();

  max_diff_map_t max_diff_map;

  // 変数の最大微分回数をもとめる
  create_max_diff_map(ps, max_diff_map);
  
  // 初期値制約の送信
  send_init_cons(ps, max_diff_map, false);

  // vars部分
  // 変数のリストを渡す
//  send_vars(ps, max_diff_map);


  // varsを渡す
  ps.put_vars(PacketSender::VA_Time, true);

////////// 受信処理
  HYDLA_LOGGER_EXTERN("--- receive  ---");

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext();
  ml_->MLGetNext();
  ml_->MLGetNext();  
  
  VCSResult result;
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code==1) { 
    // 充足
    HYDLA_LOGGER_VCS_SUMMARY("consistent");
    result = VCSR_TRUE;

    // 制約ストアにtell制約の追加
    constraint_store_.constraints.insert(
      constraint_store_.constraints.end(),
      collected_tells.begin(), 
      collected_tells.end());

    // appended_asksも追加
    /*constraint_store_.constraints.insert(
      constraint_store_.constraints.end(),
      collected_tells.begin(), 
      collected_tells.end());*/

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
  else { 
    // 制約エラー
    assert(ret_code==2);
    result = VCSR_FALSE;
    HYDLA_LOGGER_VCS_SUMMARY("inconsistent");
  }

  HYDLA_LOGGER_VCS(constraint_store_);

  return result;
}
  
VCSResult MathematicaVCSInterval::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS_SUMMARY(
    "#*** MathematicaVCSInterval::check_entailment ***\n", 
    "ask: ", *negative_ask);

  PacketSender ps(*ml_);

  // checkEntailmentInterval[guard, store, vars, pars]を渡したい
  ml_->put_function("checkEntailmentInterval", 4);

  // guard部分
  // ask制約のガードの式を得てMathematicaに渡す
  ps.put_node(negative_ask->get_guard(), PacketSender::VA_Time, true);
//  ps.put_node(negative_ask->get_guard(), PacketSender::VA_Zero, true);


  // store部分
  ml_->put_function("Join", 4);

  // 制約ストアconstraintsをMathematicaに渡す

  send_cs(ps);

  max_diff_map_t max_diff_map;

  // 変数の最大微分回数をもとめる
  create_max_diff_map(ps, max_diff_map);
  
  // 初期値制約の送信
  send_init_cons(ps, max_diff_map, false);

  ml_->put_function("List", appended_asks.size());
  // appended_asksからガード部分を得てMathematicaに渡す
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    ps.put_node(append_it->ask->get_guard(), PacketSender::VA_Time, true, append_it->entailed);
  }

  // pstoreを渡す
  send_parameter_cons();

  // 変数のリストを渡す
  send_vars(ps, max_diff_map);

  // 記号定数のリストを渡す
  send_pars();

  // varsを渡す
  //ps.put_vars(PacketSender::VA_Time, true);
  

  ml_->MLEndPacket();

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

////////// 受信処理


  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext();
  ml_->MLGetNext();
  ml_->MLGetNext();  
  
  VCSResult result;
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code==1) {
    result = VCSR_TRUE;
    HYDLA_LOGGER_VCS_SUMMARY("entailed");
  }
  else if(ret_code==2){
    result = VCSR_FALSE;
    HYDLA_LOGGER_VCS_SUMMARY("not entailed");
  }else{
    assert(ret_code==3);
    result = VCSR_UNKNOWN;
  }
  return result;
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
    HYDLA_LOGGER_VCS("send ask : ", **it);

    ml_->put_function("List", 2);    

    // guard条件を送る
    ps.put_node((*it)->get_guard(), PacketSender::VA_Time, true);

    // IDを送る
    ml_->MLPutInteger((*it)->get_id());
  }
}

VCSResult MathematicaVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time,
  const not_adopted_tells_list_t& not_adopted_tells_list,
  const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS("#*** MathematicaVCSInterval::integrate ***");

  HYDLA_LOGGER_VCS(constraint_store_);

////////////////// 送信処理
  PacketSender ps(*ml_);
  
  // integrateCalc[cons, posAsk, negAsk, NACons, vars, maxTime]を渡したい
  ml_->put_function("integrateCalc", 6);

  // 制約consを渡す
  ml_->put_function("Join", 4);


  // 制約ストアから式storeを得てMathematicaに渡す
  send_cs(ps);

  // 変数の最大微分回数をもとめる
  max_diff_map_t max_diff_map;
  create_max_diff_map(ps, max_diff_map);
  // 初期値制約の送信
  send_init_cons(ps, max_diff_map, true);
  

  ml_->put_function("List", appended_asks.size());  
  // appended_asksからガード部分を得てMathematicaに渡す
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    ps.put_node(append_it->ask->get_guard(), PacketSender::VA_Time, true, append_it->entailed);
  }

  //定数条件の送信
  send_parameter_cons();

  // posAskを渡す（{ガードの式、askのID}をそれぞれ）
  HYDLA_LOGGER_VCS("-- send positive ask -- ");
  send_ask_guards(ps, positive_asks);

  // negAskを渡す（{ガードの式、askのID}をそれぞれ）
  HYDLA_LOGGER_VCS("-- send negative ask -- ");
  send_ask_guards(ps, negative_asks);

  // 採用していないモジュール内のtell制約NAConsを渡す（{{式、ID}, {式、ID}, ...}をそれぞれのモジュールに関して）
  HYDLA_LOGGER_VCS("-- send not adopted constraint -- ");
  send_not_adopted_tells(ps, not_adopted_tells_list);

  // 変数のリストを渡す
  send_vars(ps, max_diff_map);

  // maxTimeを渡す
  time_t tmp_time(max_time);
  tmp_time -= current_time;
  

  HYDLA_LOGGER_VCS("current time:", current_time);
  HYDLA_LOGGER_VCS("send time:", tmp_time);
  if(approx_precision_ > 0) {
    // 近似して送信
    ml_->put_function("approxExpr", 2);
    ml_->put_integer(approx_precision_);
  }
  send_time(tmp_time);


////////////////// 受信処理

//   PacketChecker pc(*ml_);
//   pc.check();

  MathematicaExpressionConverter mec;

  HYDLA_LOGGER_EXTERN(
    "-- integrate math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext(); 
  ml_->MLGetNext(); // Listという関数名
  ml_->MLGetNext(); // Listの中の先頭要素

  // 求解に成功したかどうか
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    return VCSR_SOLVER_ERROR;
  }
  HYDLA_LOGGER_VCS("---integrate calc result---");
  virtual_constraint_solver_t::IntegrateResult::NextPhaseState state;

  
  // 変数と値の組を受け取る
  ml_->MLGetNext(); // Listという関数名
  int variable_list_size = ml_->get_arg_count();
  HYDLA_LOGGER_VCS("variable_list_size : ", variable_list_size);  
  ml_->MLGetNext(); ml_->MLGetNext();
  for(int i=0; i<variable_list_size; i++)
  {
    ml_->MLGetNext(); 
    ml_->MLGetNext();
    variable_t variable;
    value_t    value;
    HYDLA_LOGGER_VCS("--- add variable ---");

      // 変数名
    variable.name = ml_->get_symbol().substr(6);
    HYDLA_LOGGER_VCS("name  : ", variable.name);
    ml_->MLGetNext();

      // 微分回数
    variable.derivative_count = ml_->get_integer();
    HYDLA_LOGGER_VCS("derivative : ", variable.derivative_count);
    ml_->MLGetNext();

      // 値
    value = mec.convert_math_string_to_symbolic_value(ml_->get_string());
    HYDLA_LOGGER_VCS("value : ", value.get_string());
    ml_->MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }
  
  
  // 次のPPの時刻と，その場合の条件の組，更に終了時刻かどうかを得る
  HYDLA_LOGGER_VCS("-- receive next PP time --");
  int next_time_size = ml_->get_arg_count();
  HYDLA_LOGGER_VCS("next_time_size: ", next_time_size);
  ml_->MLGetNext();
  for(int time_it = 0; time_it < next_time_size; time_it++){
    ml_->MLGetNext();ml_->MLGetNext(); ml_->MLGetNext();
    state.time = mec.convert_math_string_to_symbolic_value(ml_->get_string()) + current_time;
    HYDLA_LOGGER_VCS_SUMMARY("next_phase_time: ", state.time);
    ml_->MLGetNext();
    int condition_size = ml_->get_arg_count();//条件式の数
    ml_->MLGetNext(); ml_->MLGetNext();
    
    state.parameter_map.clear();
    parameter_t tmp_param, prev_param;
    for(int cond_it = 0; cond_it < condition_size; cond_it++){
      value_range_t tmp_range;
      ml_->MLGetNext(); ml_->MLGetNext();
      tmp_param.name = ml_->get_string();
      HYDLA_LOGGER_VCS("returned parameter_name: ", tmp_param.name);
      ml_->MLGetNext();
      int relop_code = ml_->get_integer();
      HYDLA_LOGGER_VCS("returned relop_code: ", relop_code);
      ml_->MLGetNext();
      std::string parameter_value_string = ml_->get_string();
      HYDLA_LOGGER_VCS("returned value: ", parameter_value_string);
      ml_->MLGetNext();
      tmp_range = state.parameter_map.get_variable(tmp_param);
      tmp_range.add(value_range_t::Element(MathematicaExpressionConverter::convert_math_string_to_symbolic_value(parameter_value_string),
                                           MathematicaExpressionConverter::get_relation_from_code(relop_code)));
      state.parameter_map.set_variable(tmp_param, tmp_range);
      prev_param.name = tmp_param.name;
    }
    state.is_max_time = ml_->get_integer();
    HYDLA_LOGGER_VCS("is_max_time: ",  state.is_max_time);
    HYDLA_LOGGER_VCS("--parameter map--\n",  state.parameter_map);
    
    

    // 未定義の変数を変数表に反映
    // 初期値制約（未定義変数を含む）とvariable_mapとの差分を解消
    add_undefined_vars_to_vm(state.variable_map);
    
    integrate_result.states.push_back(state);
  }

////////////////// 受信終了

  for(unsigned int state_it = 0; state_it < integrate_result.states.size(); state_it++){
  // 時刻の簡約化。本来は関数使ってやるべきだけど、とりあえずそのままここに
    HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", integrate_result.states[state_it].time);
    ml_->put_function("ToString", 1);
    ml_->put_function("FullForm", 1);
    ml_->put_function("Simplify", 1);
    ps.put_node(integrate_result.states[state_it].time.get_node(), PacketSender::VA_None, true);
    ml_->skip_pkt_until(RETURNPKT);
    integrate_result.states[state_it].time = mec.convert_math_string_to_symbolic_value(ml_->get_string());


    // 時刻の近似
    if(approx_precision_ > 0) {
      ml_->put_function("ToString", 1);
      ml_->put_function("FullForm", 1);
      ml_->put_function("approxExpr", 2);
      ml_->put_integer(approx_precision_);
      send_time(integrate_result.states[state_it].time);
      ml_->skip_pkt_until(RETURNPKT);
      ml_->MLGetNext(); 
      integrate_result.states[state_it].time = mec.convert_math_string_to_symbolic_value(ml_->get_string());
    }
  }


  return VCSR_TRUE;
}

void MathematicaVCSInterval::send_time(const time_t& time){
  HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", time);
  PacketSender ps(*ml_);
  ps.put_node(time.get_node(), PacketSender::VA_None, false);
}

void MathematicaVCSInterval::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("--- apply_time_to_vm ---");

  PacketSender ps(*ml_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", it->first);

    // 値
    value_t    value;
    if(!it->second.is_undefined()) {
      ml_->put_function("applyTime2Expr", 2);
      ps.put_node(it->second.get_node(), PacketSender::VA_Time, true);
      send_time(time);

    ////////////////// 受信処理

      HYDLA_LOGGER_OUTPUT(
        "-- math debug print -- \n",
        (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

      ml_->skip_pkt_until(RETURNPKT);
      ml_->MLGetNext();
      ml_->MLGetNext();
      ml_->MLGetNext(); 

      int ret_code = ml_->get_integer();
      if(ret_code==0) {
        // TODO: 適切な処理をする
        assert(0);
      }
      else {
        assert(ret_code==1);
        std::string tmp = ml_->get_string();
        MathematicaExpressionConverter mec;
        value = mec.convert_math_string_to_symbolic_value(tmp);
        HYDLA_LOGGER_OUTPUT("value : ", value.get_string());
      }
    }

    out_vm.set_variable(it->first, value);   
  }
}

void MathematicaVCSInterval::add_undefined_vars_to_vm(variable_map_t& vm)
{
  HYDLA_LOGGER_VCS("--- add undefined vars to vm ---");  

  // 変数表に登録されている変数名一覧
  HYDLA_LOGGER_VCS("-- variable_name_list --");
  std::set<MathVariable> variable_name_list;
  variable_map_t::const_iterator vm_it = vm.begin();
  variable_map_t::const_iterator vm_end = vm.end();
  for(; vm_it!=vm_end; ++vm_it){
    variable_name_list.insert(vm_it->first);
    HYDLA_LOGGER_VCS(vm_it->first);
  }

  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  HYDLA_LOGGER_VCS("-- search undefined variable --");
  // 初期値制約変数のうち、変数表に登録されている変数名一覧内にないものを抽出？
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    variable_t variable = init_vars_it->first;
    std::set<MathVariable>::const_iterator vlist_it = variable_name_list.find(variable);
    if(vlist_it==variable_name_list.end()){      
      value_t value;
      HYDLA_LOGGER_VCS("variable : ", variable);
      HYDLA_LOGGER_VCS("value : ", value);
      vm.set_variable(variable, value);
    }
  }
}

void MathematicaVCSInterval::send_not_adopted_tells(PacketSender& ps, const not_adopted_tells_list_t& na_tells_list) const
{
  // {tellの式、tellのID}のリストのリスト形式で送信する

  // 採用していないモジュールの数
  ml_->put_function("List", na_tells_list.size());

  not_adopted_tells_list_t::const_iterator na_tells_list_it = na_tells_list.begin();
  not_adopted_tells_list_t::const_iterator na_tells_list_end = na_tells_list.end();
  for(; na_tells_list_it!=na_tells_list_end; ++na_tells_list_it){
    // 採用していない、あるモジュール内のtell制約の数
    ml_->put_function("List", (*na_tells_list_it).size());

    tells_t::const_iterator na_tells_it  = (*na_tells_list_it).begin();
    tells_t::const_iterator na_tells_end = (*na_tells_list_it).end();
    for(; na_tells_it!=na_tells_end; ++na_tells_it) {
      HYDLA_LOGGER_VCS("send not adopted tell : ", **na_tells_it);

      ml_->put_function("List", 2);

      // tell制約を送る
      ps.put_node((*na_tells_it)->get_child(), PacketSender::VA_Time, true);
      
      // IDを送る
      ml_->MLPutInteger((*na_tells_it)->get_id());
    }
  }
}

void MathematicaVCSInterval::send_cs(PacketSender& ps) const
{
  HYDLA_LOGGER_VCS(
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
  HYDLA_LOGGER_VCS("---- Send Constraint Store Vars -----");
}

std::ostream& MathematicaVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSInterval ***\n"
    << "--- constraint store ---\n";
/*
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
   }*/

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

  s << "-- init vars max diff map --\n";
  BOOST_FOREACH(
    const MathematicaVCSInterval::constraint_store_t::init_vars_max_diff_map_t::value_type& i, 
    c.init_vars_max_diff_map)
  {
    s << "name: " << i.first
      << "  diff: " << i.second
      << "\n";
  }
  
  return s;
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

