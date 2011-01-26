#include "MathematicaVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "mathlink_helper.h"
#include "Logger.h"
#include "PacketSender.h"
#include "PacketChecker.h"
#include "PacketErrorHandler.h"
#include "Types.h"

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
  if(Logger::varflag==1||Logger::varflag==0){
	  HYDLA_LOGGER_AREA("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_AREA("no Variables");
    return true;
  }
  HYDLA_LOGGER_AREA("------Variable map------\n", 
                     variable_map);
  }

  HYDLA_LOGGER_SUMMARY("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_SUMMARY("------Variable map------\n", 
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
  
  if(Logger::constflag==6){
  HYDLA_LOGGER_AREA(constraint_store_);
  }

  HYDLA_LOGGER_DEBUG(constraint_store_);

  return true;
}



bool MathematicaVCSInterval::reset(const variable_map_t& variable_map,  const parameter_map_t& parameter_map)
{
  if(!reset(variable_map)){
    return false;
  }


  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_SUMMARY("------Parameter map------\n", 
                     parameter_map);

  parameter_map_=parameter_map;

  return true;
}


bool MathematicaVCSInterval::create_variable_map(variable_map_t& variable_map)
{
  // Intervalではcreate_variable_map関数無効
  assert(0);
  return false;
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

  if(Logger::constflag==7){
  HYDLA_LOGGER_AREA(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());
  }
  HYDLA_LOGGER_DEBUG(
    "-- max diff map --\n",
    MaxDiffMapDumper(max_diff_map.begin(), 
                     max_diff_map.end()).s.str());

}

void MathematicaVCSInterval::send_init_cons(
  PacketSender& ps, 
  const max_diff_map_t& max_diff_map,
  bool use_approx)
{
  if(Logger::mathsendflag==1){
    HYDLA_LOGGER_AREA("---- Begin MathematicaVCSInterval::send_init_cons ----");
  }
  HYDLA_LOGGER_DEBUG("---- Begin MathematicaVCSInterval::send_init_cons ----");
    
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

  if(Logger::mathsendflag==1){
	  HYDLA_LOGGER_AREA("init_vars_count: ", init_vars_count);
  }

  HYDLA_LOGGER_DEBUG("init_vars_count: ", init_vars_count);


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

        ml_->put_function("ToExpression", 1);
        ml_->put_string(init_vars_it->second.get_first_value());
    }


/*
    // 集めたtell制約内に出現しない
    if(md_it==max_diff_map.end())
    {
      ml_->put_function("Equal", 2);

      // 変数名
      // 初期値制約内で最大微分回数のものは時刻引数[t]として送る
      constraint_store_t::init_vars_max_diff_map_t::const_iterator ivmd_it = 
        constraint_store_.init_vars_max_diff_map.find(init_vars_it->first.name);
      if(init_vars_it->first.derivative_count==ivmd_it->second){
        ps.put_var(
          boost::make_tuple(init_vars_it->first.name, 
                            init_vars_it->first.derivative_count, 
                            false),
          PacketSender::VA_Time);
      }
      else
      {
        ps.put_var(
          boost::make_tuple(init_vars_it->first.name, 
                            init_vars_it->first.derivative_count, 
                            false),
          PacketSender::VA_Zero);
      }

      // 値
      if(use_approx && approx_precision_ > 0) {
        // 近似して送信
        ml_->put_function("approxExpr", 2);
        ml_->put_integer(approx_precision_);
      }

      ml_->put_function("ToExpression", 1);
      ml_->put_string(init_vars_it->second.str);      

      // 制約ストア中で使用される変数の一覧にも追加
      MathVariable mv;
      mv.name             = init_vars_it->first.name;
      mv.derivative_count = init_vars_it->first.derivative_count;
      constraint_store_.cons_vars.insert(mv);

    }
    else if(md_it->second  > init_vars_it->first.derivative_count)
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

      ml_->put_function("ToExpression", 1);
      ml_->put_string(init_vars_it->second.str);      

    }
*/


  }

}



void MathematicaVCSInterval::send_parameter_cons() const{
  HYDLA_LOGGER_DEBUG("---- Begin MathematicaVCSInterval::send_parameter_cons ----");


  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();
  int para_size=0;
  for(; par_it!=par_end; ++par_it) {  
    value_t::or_vector::const_iterator or_it = par_it->second.or_begin(), or_end = par_it->second.or_end();
    for(;or_it != or_end; or_it++){
      para_size += or_it->size();
    }
  }

  HYDLA_LOGGER_DEBUG("parameter_cons_count: ", para_size);
  HYDLA_LOGGER_SUMMARY("------Parameter map------\n", 
                     parameter_map_);
  
  // 制約をMathematicaへ送信
  ml_->put_function("List", para_size);
  
  for(par_it = parameter_map_.begin(); par_it!=par_end; ++par_it) {
  
    value_t::or_vector::const_iterator or_it = par_it->second.or_begin(), or_end = par_it->second.or_end();
    for(;or_it != or_end; or_it++){
      value_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
      for(; and_it != and_end; and_it++){
        switch(and_it->relation){
          default:
            assert(0);
            break;
          case value_t::EQUAL:
            ml_->put_function("Equal", 2);
            break;
          case value_t::NOT_EQUAL:
            ml_->put_function("UnEqual", 2);
            break;
          case value_t::GREATER:
            ml_->put_function("Greater", 2);
            break;
          case value_t::GREATER_EQUAL:
            ml_->put_function("GreaterEqual", 2);
            break;
          case value_t::LESS:
            ml_->put_function("Less", 2);
            break;
          case value_t::LESS_EQUAL:
            ml_->put_function("LessEqual", 2);
            break;
        }
        // 変数名
        ml_->put_symbol(PacketSender::par_prefix + par_it->first.get_name());

        // 値
        ml_->put_function("ToExpression", 1);
        ml_->put_string(and_it->value);
      }
    }
  }
}




void MathematicaVCSInterval::send_vars(
  PacketSender& ps, const max_diff_map_t& max_diff_map)
{
	if(Logger::mathsendflag==2){
		HYDLA_LOGGER_AREA("---- MathematicaVCSInterval::send_vars ----");
	}
  HYDLA_LOGGER_DEBUG("---- MathematicaVCSInterval::send_vars ----");

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

  if(Logger::mathsendflag==2){
	  HYDLA_LOGGER_AREA("count:", vars_count);
  }
  HYDLA_LOGGER_DEBUG("count:", vars_count);

  ml_->put_function("List", vars_count);
  for(md_it=max_diff_map.begin(); md_it!=md_end; ++md_it) {
    for(int i=0; i<=md_it->second; i++) {
      ps.put_var(boost::make_tuple(md_it->first, i, false), 
                 PacketSender::VA_Time);

	if(Logger::mathsendflag==2){
		HYDLA_LOGGER_AREA("put: ", 
                         "  name: ", md_it->first,
                         "  diff: ", i);
	}
      HYDLA_LOGGER_DEBUG("put: ", 
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

  HYDLA_LOGGER_DEBUG("cs_vars_count:",  constraint_store_.cons_vars.size());

  for(; cs_vars_it!=cs_vars_end; ++cs_vars_it){
      ps.put_var(boost::make_tuple(cs_vars_it->name, cs_vars_it->derivative_count, false), 
                 PacketSender::VA_Time);

      HYDLA_LOGGER_DEBUG("put: ", 
                         "  name: ", cs_vars_it->name,
                         "  diff: ", cs_vars_it->derivative_count);
  }
*/

}

VCSResult MathematicaVCSInterval::add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks)
{
	if(Logger::constflag==7){
      HYDLA_LOGGER_AREA("#*** Begin MathematicaVCSInterval::add_constraint ***");
	}
  HYDLA_LOGGER_DEBUG("#*** Begin MathematicaVCSInterval::add_constraint ***");

  PacketSender ps(*ml_);

  // isConsistentInterval[expr, vars]を渡したい
  ml_->put_function("isConsistentInterval", 2);

  // expr部分
  ml_->put_function("Join", 3);
  ml_->put_function("List", collected_tells.size() + appended_asks.size());
  if(Logger::constflag==7){
  HYDLA_LOGGER_AREA(
    "tells_size:", collected_tells.size() + appended_asks.size());
  }
  HYDLA_LOGGER_DEBUG(
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
	  if(Logger::constflag==7){
		HYDLA_LOGGER_AREA("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
	  }
    HYDLA_LOGGER_DEBUG("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    ps.put_node(append_it->ask->get_guard(), PacketSender::VA_Time, true, append_it->entailed);
  }

  // 制約ストアconstraintsをMathematicaに渡す
  send_cs(ps);

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
  HYDLA_LOGGER_DEBUG("--- receive  ---");

  HYDLA_LOGGER_DEBUG(
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
	if(Logger::conflag==2||Logger::conflag==0){
     HYDLA_LOGGER_AREA("consistent");
    }
    // 充足
    HYDLA_LOGGER_DEBUG("consistent");
    result = VCSR_TRUE;

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
  else { 
    // 制約エラー
    assert(ret_code==2);
    result = VCSR_FALSE;
	if(Logger::conflag==2||Logger::conflag==0){
     HYDLA_LOGGER_AREA("inconsistent");
    }
    HYDLA_LOGGER_DEBUG("inconsistent");
  }

  HYDLA_LOGGER_DEBUG(constraint_store_);

  return result;
}
  
VCSResult MathematicaVCSInterval::check_entailment(const ask_node_sptr& negative_ask)
{
  if(Logger::enflag==3||Logger::enflag==0){
     HYDLA_LOGGER_AREA(	"#*** MathematicaVCSInterval::check_entailment ***\n", 
	"ask: ");
	 (negative_ask)->dump_infix(std::cout);
	 HYDLA_LOGGER_AREA("\n");
  }
	
  HYDLA_LOGGER_SUMMARY(
    "#*** MathematicaVCSInterval::check_entailment ***\n", 
    "ask: ", *negative_ask);

  PacketSender ps(*ml_);

  // checkEntailment[guard, store, vars, pstore]を渡したい
  ml_->put_function("checkEntailmentInterval", 3);

  // guard部分
  // ask制約のガードの式を得てMathematicaに渡す
  ps.put_node(negative_ask->get_guard(), PacketSender::VA_Time, true);
//  ps.put_node(negative_ask->get_guard(), PacketSender::VA_Zero, true);


  // store部分
  ml_->put_function("Join", 3);

  // 制約ストアconstraintsをMathematicaに渡す

  send_cs(ps);

/*
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
*/

  max_diff_map_t max_diff_map;

  // 変数の最大微分回数をもとめる
  create_max_diff_map(ps, max_diff_map);
  
  // 初期値制約の送信
  send_init_cons(ps, max_diff_map, false);
  
  // pstoreを渡す
  send_parameter_cons();


  // 変数のリストを渡す
  send_vars(ps, max_diff_map);

  // varsを渡す
  //ps.put_vars(PacketSender::VA_Time, true);
  

  ml_->MLEndPacket();

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  
//  HYDLA_LOGGER_DEBUG(
//    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

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
  	if(Logger::enflag==3||Logger::enflag==0){
      HYDLA_LOGGER_AREA("entailed");
  	}
  	if(Logger::conflag==2||Logger::conflag==0){
      HYDLA_LOGGER_AREA("Because entailed,isConsistency judgment is done again");
    }
    HYDLA_LOGGER_SUMMARY("entailed");
  }
  else {
    assert(ret_code==2);
    result = VCSR_FALSE;
  	if(Logger::enflag==3||Logger::enflag==0){
      HYDLA_LOGGER_AREA("not entailed");
  	}
    HYDLA_LOGGER_SUMMARY("not entailed");
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
    HYDLA_LOGGER_DEBUG("send ask : ", **it);

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
  const not_adopted_tells_list_t& not_adopted_tells_list)
{
  if(Logger::mathcalcflag==1){
    HYDLA_LOGGER_AREA("#*** MathematicaVCSInterval::integrate ***");

    HYDLA_LOGGER_AREA(constraint_store_);
  }
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
//     max_time,
//     "--- not_adopted_tells_list ---\n",
//     not_adopted_tells_list);

////////////////// 送信処理
  PacketSender ps(*ml_);
  
  // integrateCalc[cons, posAsk, negAsk, NACons, vars, maxTime]を渡したい
  ml_->put_function("integrateCalc", 6);

  // 制約consを渡す
  ml_->put_function("Join", 3);

  // 制約ストアから式storeを得てMathematicaに渡す
  send_cs(ps);

  // 変数の最大微分回数をもとめる
  max_diff_map_t max_diff_map;
  create_max_diff_map(ps, max_diff_map);
  // 初期値制約の送信
  send_init_cons(ps, max_diff_map, true);
  
  send_parameter_cons();

  if(Logger::constflag==11){
    // posAskを渡す（{ガードの式、askのID}をそれぞれ）
    HYDLA_LOGGER_AREA("-- send positive ask -- ");
    send_ask_guards(ps, positive_asks);

    // negAskを渡す（{ガードの式、askのID}をそれぞれ）
    HYDLA_LOGGER_AREA("-- send negative ask -- ");
    send_ask_guards(ps, negative_asks);

    // 採用していないモジュール内のtell制約NAConsを渡す（{{式、ID}, {式、ID}, ...}をそれぞれのモジュールに関して）
    HYDLA_LOGGER_AREA("-- send not adopted constraint -- ");
    send_not_adopted_tells(ps, not_adopted_tells_list);
  }

  // posAskを渡す（{ガードの式、askのID}をそれぞれ）
  HYDLA_LOGGER_DEBUG("-- send positive ask -- ");
  send_ask_guards(ps, positive_asks);

  // negAskを渡す（{ガードの式、askのID}をそれぞれ）
  HYDLA_LOGGER_DEBUG("-- send negative ask -- ");
  send_ask_guards(ps, negative_asks);

  // 採用していないモジュール内のtell制約NAConsを渡す（{{式、ID}, {式、ID}, ...}をそれぞれのモジュールに関して）
  HYDLA_LOGGER_DEBUG("-- send not adopted constraint -- ");
  send_not_adopted_tells(ps, not_adopted_tells_list);

  // 変数のリストを渡す
  send_vars(ps, max_diff_map);

  // maxTimeを渡す
  time_t tmp_time(max_time);
  tmp_time -= current_time;
  
  if(Logger::timeflag==1){
  HYDLA_LOGGER_AREA("current time:", current_time);
  HYDLA_LOGGER_AREA("send time:", tmp_time);
  }

  HYDLA_LOGGER_DEBUG("current time:", current_time);
  HYDLA_LOGGER_DEBUG("send time:", tmp_time);
  if(approx_precision_ > 0) {
    // 近似して送信
    ml_->put_function("approxExpr", 2);
    ml_->put_integer(approx_precision_);
  }
  send_time(tmp_time);


////////////////// 受信処理

//   PacketChecker pc(*ml_);
//   pc.check();

  HYDLA_LOGGER_DEBUG(
    "-- integrate math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_DEBUG((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
//  HYDLA_LOGGER_DEBUG((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext(); 
  ml_->MLGetNext(); // Listという関数名
  ml_->MLGetNext(); // Listの中の先頭要素

  // 求解に成功したかどうか
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    return VCSR_SOLVER_ERROR;
  }

  HYDLA_LOGGER_DEBUG("---integrate calc result---");
  integrate_result.states.resize(1);
  virtual_constraint_solver_t::IntegrateResult::NextPhaseState& state = 
    integrate_result.states.back();

  // next_point_phase_timeを得る
  MathTime elapsed_time;
  elapsed_time.set(ml_->get_string());
  if(Logger::timeflag==2){
  HYDLA_LOGGER_AREA("elapsed_time: ", elapsed_time);  
  state.time  = elapsed_time;
  state.time += current_time;
  HYDLA_LOGGER_AREA("next_phase_time: ", state.time);  
  }
  HYDLA_LOGGER_DEBUG("elapsed_time: ", elapsed_time);  
  state.time  = elapsed_time;
  state.time += current_time;
  HYDLA_LOGGER_SUMMARY("next_phase_time: ", state.time);  
  ml_->MLGetNext(); // Listという関数名
  
  // 変数と値の組を受け取る
  int variable_list_size = ml_->get_arg_count();
  if(Logger::varflag==8){
  HYDLA_LOGGER_AREA("variable_list_size : ", variable_list_size);
  }
  HYDLA_LOGGER_DEBUG("variable_list_size : ", variable_list_size);  
  ml_->MLGetNext(); ml_->MLGetNext();
  for(int i=0; i<variable_list_size; i++)
  {
    ml_->MLGetNext(); 
    ml_->MLGetNext();

    variable_t variable;
    value_t    value;
  if(Logger::varflag==8){
	HYDLA_LOGGER_AREA("--- add variable ---");
  }
    HYDLA_LOGGER_DEBUG("--- add variable ---");

    // 変数名
    variable.name = ml_->get_symbol().substr(6);
	if(Logger::varflag==8){
		HYDLA_LOGGER_AREA("name  : ", variable.name);
	}
    HYDLA_LOGGER_DEBUG("name  : ", variable.name);
    ml_->MLGetNext();

    // 微分回数
    variable.derivative_count = ml_->get_integer();
    if(Logger::varflag==8){
		HYDLA_LOGGER_AREA("derivative : ", variable.derivative_count);
	}
	HYDLA_LOGGER_DEBUG("derivative : ", variable.derivative_count);
    ml_->MLGetNext();

    // 値
    value.set( value_t::Element(ml_->get_string(), value_t::EQUAL));
  if(Logger::varflag==8){
	HYDLA_LOGGER_AREA("value : ", value.get_first_value());
  }
    HYDLA_LOGGER_DEBUG("value : ", value.get_first_value());
    ml_->MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }

  // askとそのIDの組一覧を得る
  int changed_asks_size = ml_->get_arg_count();
  if(Logger::constflag==12){
	  HYDLA_LOGGER_AREA("changed_asks_size : ", changed_asks_size);
  }
  HYDLA_LOGGER_DEBUG("changed_asks_size : ", changed_asks_size);
  ml_->MLGetNext(); // Listという関数名
  for(int j=0; j<changed_asks_size; j++)
  {
	if(Logger::constflag==12){
	  HYDLA_LOGGER_AREA("--- add changed ask ---");
	}
    HYDLA_LOGGER_DEBUG("--- add changed ask ---");

    ml_->MLGetNext(); // List関数
    ml_->MLGetNext(); // Listという関数名
    ml_->MLGetNext();

    std::string changed_ask_type_str = ml_->get_symbol(); // pos2negまたはneg2pos
    if(Logger::constflag==12){
		HYDLA_LOGGER_AREA("changed_ask_type_str : ", changed_ask_type_str);
	}
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
  if(Logger::timeflag==3){
  // max timeかどうか
  HYDLA_LOGGER_AREA("-- receive max time --");
//   PacketChecker c(*ml_);
//   c.check2();
  if(changed_asks_size==0) {
    ml_->MLGetNext();  
  }
  state.is_max_time = ml_->get_integer() == 1;
  HYDLA_LOGGER_AREA("is_max_time : ", state.is_max_time);
  }

  // max timeかどうか
  HYDLA_LOGGER_DEBUG("-- receive max time --");
//   PacketChecker c(*ml_);
//   c.check2();
  if(changed_asks_size==0) {
    ml_->MLGetNext();  
  }
  state.is_max_time = ml_->get_integer() == 1;
  HYDLA_LOGGER_DEBUG("is_max_time : ", state.is_max_time);

////////////////// 受信終了

  // 時刻の簡約化。本来は関数使ってやるべきだけど、とりあえずそのままここに
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", state.time);
  ml_->put_function("ToString", 1);
  ml_->put_function("FullForm", 1);
  ml_->put_function("Simplify", 1);
  ml_->put_function("ToExpression", 1);
  ml_->put_string(state.time.get_string());
  ml_->skip_pkt_until(RETURNPKT);
  state.time.set(ml_->get_string());


  // 時刻の近似
  if(approx_precision_ > 0) {
    ml_->put_function("ToString", 1);
    ml_->put_function("FullForm", 1);
    ml_->put_function("approxExpr", 2);
    ml_->put_integer(approx_precision_);
    send_time(state.time);
    ml_->skip_pkt_until(RETURNPKT);
    ml_->MLGetNext(); 
    state.time.set(ml_->get_string());
  }

  // 未定義の変数を変数表に反映
  // 初期値制約（未定義変数を含む）とvariable_mapとの差分を解消
  add_undefined_vars_to_vm(state.variable_map);


/*
  // 出力する時刻のリストを作成する
  HYDLA_LOGGER_DEBUG("--- calc output time list ---");  

  ml_->put_function("createOutputTimeList", 3);
  ml_->put_integer(0);
  elapsed_time.send_time(*ml_);
  max_interval_.send_time(*ml_);

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext(); 
  int output_time_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("output_time_size : ", output_time_size);  
  ml_->MLGetNext(); 
  ml_->MLGetNext(); 
  std::vector<MathTime> output_time_list(output_time_size);
  for(int i=0; i<output_time_size; i++) {

    output_time_list[i].receive_time(*ml_);
    HYDLA_LOGGER_DEBUG("output time : ", output_time_list[i]);  
  }

//   PacketChecker pc(*ml_);
//   pc.check2();

  // 出力関数に対して時刻と変数表の組を与える
  HYDLA_LOGGER_DEBUG("--- send vm to output func ---");  
  std::vector<MathTime>::const_iterator outtime_it  = output_time_list.begin();
  std::vector<MathTime>::const_iterator outtime_end = output_time_list.end();
  for(; outtime_it!=outtime_end; ++outtime_it) {
    variable_map_t outtime_vm;
    apply_time_to_vm(state.variable_map, outtime_vm, *outtime_it);
    
    output(*outtime_it + current_time, outtime_vm);
  }

  // 次のフェーズにおける変数の値を導出する
  //HYDLA_LOGGER_DEBUG("--- calc next phase variable map ---");  
  //apply_time_to_vm(integrate_result.map_with_t, state.variable_map, elapsed_time);    


  // 離散変化時のプロットを補正
  std::cout << std::endl;
  */
  
//   HYDLA_LOGGER_DEBUG(
//     "--- integrate result ---\n", 
//     integrate_result);


  return VCSR_TRUE;
}

void MathematicaVCSInterval::send_time(const time_t& time){
  if(Logger::timeflag==4){
  HYDLA_LOGGER_AREA("SymbolicTime::send_time : ", time);
  }
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", time);
  ml_->put_function("ToExpression", 1);
  ml_->put_string(time.get_string());
}

void MathematicaVCSInterval::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
	if(Logger::timeflag==4){
		HYDLA_LOGGER_AREA("--- apply_time_to_vm ---");
	}

  HYDLA_LOGGER_DEBUG("--- apply_time_to_vm ---");

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    if(Logger::timeflag==4){
    HYDLA_LOGGER_AREA("variable : ", it->first);
	}
    HYDLA_LOGGER_DEBUG("variable : ", it->first);

    // 値
    value_t    value;
    if(!it->second.is_undefined()) {
      ml_->put_function("applyTime2Expr", 2);
      ml_->put_function("ToExpression", 1);
      ml_->put_string(it->second.get_first_value());
      send_time(time);

    ////////////////// 受信処理

      HYDLA_LOGGER_DEBUG(
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
        value.set( value_t::Element(ml_->get_string(), value_t::EQUAL));
        HYDLA_LOGGER_DEBUG("value : ", value.get_first_value());
      }
    }

    out_vm.set_variable(it->first, value);   
  }
}

void MathematicaVCSInterval::add_undefined_vars_to_vm(variable_map_t& vm)
{
	if(Logger::varflag==9){
		HYDLA_LOGGER_AREA("--- add undefined vars to vm ---");  
	}
  HYDLA_LOGGER_DEBUG("--- add undefined vars to vm ---");  

  // 変数表に登録されている変数名一覧
  if(Logger::varflag==9){
	  HYDLA_LOGGER_AREA("-- variable_name_list --");
  }
  HYDLA_LOGGER_DEBUG("-- variable_name_list --");
  std::set<MathVariable> variable_name_list;
  variable_map_t::const_iterator vm_it = vm.begin();
  variable_map_t::const_iterator vm_end = vm.end();
  for(; vm_it!=vm_end; ++vm_it){
    variable_name_list.insert(vm_it->first);
	if(Logger::varflag==9){
		HYDLA_LOGGER_AREA(vm_it->first);
	}
    HYDLA_LOGGER_DEBUG(vm_it->first);
  }

  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  if(Logger::varflag==9){
  HYDLA_LOGGER_AREA("-- search undefined variable --");
  }
  HYDLA_LOGGER_DEBUG("-- search undefined variable --");
  // 初期値制約変数のうち、変数表に登録されている変数名一覧内にないものを抽出？
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    variable_t variable = init_vars_it->first;
    std::set<MathVariable>::const_iterator vlist_it = variable_name_list.find(variable);
    if(vlist_it==variable_name_list.end()){      
      value_t value;
	  if(Logger::varflag==9){
		  HYDLA_LOGGER_AREA("variable : ", variable);
          HYDLA_LOGGER_AREA("value : ", value);
	  }
      HYDLA_LOGGER_DEBUG("variable : ", variable);
      HYDLA_LOGGER_DEBUG("value : ", value);
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
      HYDLA_LOGGER_DEBUG("send not adopted tell : ", **na_tells_it);

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

