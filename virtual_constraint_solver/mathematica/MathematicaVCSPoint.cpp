#include "MathematicaVCSPoint.h"

#include <cassert>

#include <boost/algorithm/string/predicate.hpp>

#include "mathlink_helper.h"
#include "PacketErrorHandler.h"
#include "Logger.h"
#include "PacketChecker.h"

using namespace hydla::vcs;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSPoint::MathematicaVCSPoint(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSPoint::~MathematicaVCSPoint()
{}

bool MathematicaVCSPoint::reset()
{
  // TODO: チョイ考える
  assert(0);
//   constraint_store_.first.clear();
//   constraint_store_.second.clear();
  return true;
}

bool MathematicaVCSPoint::reset(const variable_map_t& variable_map)
{
  if(Logger::varflag==6){
  HYDLA_LOGGER_AREA("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_AREA("no Variables");
    return true;
  }
  HYDLA_LOGGER_AREA("------Variable map------\n", variable_map);
  }

  HYDLA_LOGGER_SUMMARY("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_SUMMARY("------Variable map------\n", variable_map);

  std::set<MathValue> and_cons_set;

  variable_map_t::variable_list_t::const_iterator it = 
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = 
    variable_map.end();
  for(; it!=end; ++it)
  {
    const MathVariable& variable = (*it).first;
    const value_t&    value = it->second;

    if(!value.is_undefined()) {
      std::ostringstream val_str;

      value_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
      for(;or_it != or_end; or_it++){
         value_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
         for(; and_it != and_end; and_it++){
            // MathVariable側に関する文字列を作成
            switch(and_it->relation){
              default:
                assert(0);
                break;
              case value_t::EQUAL:
                val_str << "Equal[";
                break;
              case value_t::NOT_EQUAL:
                val_str << "UnEqual[";
                break;
              case value_t::GREATER:
                val_str << "Greater[";
                break;
              case value_t::GREATER_EQUAL:
                val_str << "GreaterEqual[";
                break;
              case value_t::LESS:
                val_str << "Less[";
                break;
              case value_t::LESS_EQUAL:
                val_str << "LessEqual[";
                break;
            }

            if(variable.derivative_count > 0)
            {
              val_str << "Derivative["
                      << variable.derivative_count
                      << "][prev["
                      << PacketSender::var_prefix
                      << variable.name
                      << "]]";
            }
            else
            {
              val_str << "prev["
                      << PacketSender::var_prefix
                      << variable.name
                      << "]";
            }

            val_str << ","
                    << and_it->value
                    << "]"; // Equalの閉じ括弧

            MathValue new_math_value;
            new_math_value.set(val_str.str());
            and_cons_set.insert(new_math_value);
         }
      }

      // 制約ストア内の変数一覧を作成
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
    }
  }

  constraint_store_.first.insert(and_cons_set);

  HYDLA_LOGGER_DEBUG(*this);

  return true;
}

bool MathematicaVCSPoint::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map){
  if(!reset(variable_map)){
    return false;
  }

  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_SUMMARY("no Parameters");
    return true;
  }
  HYDLA_LOGGER_SUMMARY("------Parameter map------\n", parameter_map);


  std::set<MathValue> and_cons_set;
  par_names_.clear();

  parameter_map_t::variable_list_t::const_iterator it = 
    parameter_map.begin();
  parameter_map_t::variable_list_t::const_iterator end = 
    parameter_map.end();
  for(; it!=end; ++it)
  {
    const value_t&    value = it->second;
    if(!value.is_undefined()) {
      value_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
      for(;or_it != or_end; or_it++){
        value_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
        for(; and_it != and_end; and_it++){
          std::ostringstream val_str;
          
          // MathVariable側に関する文字列を作成
          switch(and_it->relation){
            default:
              assert(0);
              break;
            case value_t::EQUAL:
              val_str << "Equal[";
              break;
            case value_t::GREATER:
              val_str << "Greater[";
              break;
            case value_t::GREATER_EQUAL:
              val_str << "GreaterEqual[";
              break;
            case value_t::LESS:
              val_str << "Less[";
              break;
            case value_t::LESS_EQUAL:
              val_str << "LessEqual[";
              break;
          }

          val_str << PacketSender::par_prefix
                  << it->first.get_name();

          val_str << ","
                  << and_it->value
                  << "]"; // Equalの閉じ括弧
          MathValue new_math_value;
          new_math_value.set(val_str.str());
          and_cons_set.insert(new_math_value);
          par_names_.insert(it->first.get_name());
        }
      }
    }
  }
  parameter_store_.first.insert(and_cons_set);
  return true;
}

bool MathematicaVCSPoint::create_variable_map(variable_map_t& variable_map)
{/*
	if(Logger::varflag==7){
	HYDLA_LOGGER_AREA(
    "#*** MathematicaVCSPoint::create_variable_map ***\n",
    "--- variable_map ---\n",
    variable_map,
    "--- constraint_store ---\n",
    *this);
	}
	
  HYDLA_LOGGER_DEBUG(
    "#*** MathematicaVCSPoint::create_variable_map ***\n",
    "--- variable_map ---\n",
    variable_map,
    "--- constraint_store ---\n",
    *this);*/

  // 制約ストアが空（true）の場合は変数表も空で良い
  if(cs_is_true()) return true;


/////////////////// 送信処理

  // convertCSToVM[exprs]を渡したい
  ml_->put_function("convertCSToVM", 1);
  send_cs();

  
/////////////////// 受信処理

  //PacketChecker pc(*ml_);
  //pc.check();

  
  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  ml_->skip_pkt_until(RETURNPKT);
  
  return receive_variable_map(variable_map);
}




bool MathematicaVCSPoint::receive_variable_map(variable_map_t& variable_map)
{

  ml_->MLGetNext();
//  ml_->MLGetNext();

  // List関数の要素数（式の個数）を得る
  int expr_size = ml_->get_arg_count();
  HYDLA_LOGGER_DEBUG("expr_size: ", expr_size);
  ml_->MLGetNext(); // Listという関数名

  variable_t symbolic_variable;
  value_t symbolic_value;

  for(int i=0; i<expr_size; i++)
  {
    ml_->MLGetNext();
    ml_->MLGetNext();
    
    // 変数名（名前、微分回数、prev）
    ml_->MLGetNext();
    ml_->MLGetNext();
    ml_->MLGetNext(); // ?
    std::string variable_name = ml_->get_string();
    int variable_derivative_count = ml_->get_integer();
    int prev = ml_->get_integer();

    // 関係演算子のコード
    int relop_code = ml_->get_integer();
    // 値
    std::string value_str = ml_->get_string();


    // prev変数は処理しない
    if(prev==1) continue;
    
    if(symbolic_variable.name != variable_name || symbolic_variable.derivative_count != variable_derivative_count){
      symbolic_variable.name = variable_name;
      symbolic_variable.derivative_count = variable_derivative_count;
      symbolic_value.clear();
    }
    

    // 関係演算子コードを元に、変数表の対応する部分に代入する
    // TODO: Orの扱い
    switch(relop_code)
    {
      case 0: // Equal
        symbolic_value.add(value_t::Element(value_str,value_t::EQUAL));
        break;
      case 1: // Less
        symbolic_value.add(value_t::Element(value_str,value_t::LESS));
        break;
      case 2: // Greater
        symbolic_value.add(value_t::Element(value_str,value_t::GREATER));
        break;
      case 3: // LessEqual
        symbolic_value.add(value_t::Element(value_str,value_t::LESS_EQUAL));
        break;
      case 4: // GreaterEqual
        symbolic_value.add(value_t::Element(value_str,value_t::GREATER_EQUAL));
        break;
    }
    variable_map.set_variable(symbolic_variable, symbolic_value);
  }

  return true;
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

void MathematicaVCSPoint::create_max_diff_map(
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

void MathematicaVCSPoint::add_left_continuity_constraint(
  PacketSender& ps, max_diff_map_t& max_diff_map)
{
  if(Logger::mathsendflag==1){
	  HYDLA_LOGGER_AREA("---- Begin MathematicaVCSPoint::add_left_continuity_constraint ----");
  }

  HYDLA_LOGGER_DEBUG("---- Begin MathematicaVCSPoint::add_left_continuity_constraint ----");

  // 送信する制約の個数を求める
  int left_cont_vars_count = 0;

  // 制約ストア中の変数のうち、集めたtell制約に出現する最大微分回数より小さい微分回数であるもののみ追加
  constraint_store_vars_t::const_iterator cs_vars_it = constraint_store_.second.begin();
  constraint_store_vars_t::const_iterator cs_vars_end = constraint_store_.second.end();
  for(; cs_vars_it!=cs_vars_end; ++cs_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(cs_vars_it->get<0>());
    if(md_it!=max_diff_map.end() &&
       md_it->second  > cs_vars_it->get<1>()) 
    {
      left_cont_vars_count++;
    }
  }
  if(Logger::mathsendflag==1){
	  HYDLA_LOGGER_AREA("left_cont_vars_count(in cs_var): ", left_cont_vars_count);
  }

  HYDLA_LOGGER_DEBUG("left_cont_vars_count(in cs_var): ", left_cont_vars_count);  


  // 時刻0では制約ストアが空なため、集めたtell制約内の変数について調べる
  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();
  for(; md_it!=md_end; ++md_it) {
    if(constraint_store_.second.find(md_it->first)==constraint_store_.second.end()){
      for(int i=0; i<md_it->second; ++i){
        left_cont_vars_count++;
      }
    }
  }
  if(Logger::mathsendflag==1){
	    HYDLA_LOGGER_AREA("left_cont_vars_count(in cs_var + in vars): ", left_cont_vars_count);  
  }

  HYDLA_LOGGER_DEBUG("left_cont_vars_count(in cs_var + in vars): ", left_cont_vars_count);  


  if(Logger::mathsendflag==1){
	    HYDLA_LOGGER_AREA("--- in cs_var ---");  
  }

  HYDLA_LOGGER_DEBUG("--- in cs_var ---");  

  // Mathematicaへ送信
  ml_->put_function("List", left_cont_vars_count);

  cs_vars_it  = constraint_store_.second.begin();
  cs_vars_end = constraint_store_.second.end();
  for(; cs_vars_it!=cs_vars_end; ++cs_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(cs_vars_it->get<0>());
    if(md_it!=max_diff_map.end() &&
       md_it->second  > cs_vars_it->get<1>()) 
    {
      ml_->put_function("Equal", 2);

      // Prev変数側
      // 変数名
      ps.put_var(
        boost::make_tuple(cs_vars_it->get<0>(), 
                          cs_vars_it->get<1>(), 
                          true),
        PacketSender::VA_None);

      // Now変数側
      // 変数名
      ps.put_var(
        boost::make_tuple(cs_vars_it->get<0>(), 
                          cs_vars_it->get<1>(), 
                          false),
        PacketSender::VA_None);
    }
  }
  if(Logger::mathsendflag==1){
	    HYDLA_LOGGER_AREA("--- in vars ---");  
  }

  HYDLA_LOGGER_DEBUG("--- in vars ---");  

  // max_diff_mapについてつくる
  md_it = max_diff_map.begin();
  md_end = max_diff_map.end();
  for(; md_it!=md_end; ++md_it) {
    if(constraint_store_.second.find(md_it->first)==constraint_store_.second.end()){
      for(int i=0; i<md_it->second; ++i){
        ml_->put_function("Equal", 2);
        
        // Prev変数側
        // 変数名
        ps.put_var(
          boost::make_tuple(md_it->first, 
                            i, 
                            true),
          PacketSender::VA_None);
        
        // Now変数側
        // 変数名
        ps.put_var(
          boost::make_tuple(md_it->first, 
                            i, 
                            false),
          PacketSender::VA_None);

        // 制約ストア内の変数扱いする
        // TODO:要検討
        constraint_store_.second.insert(boost::make_tuple(md_it->first, 
                                                          i, 
                                                          true));

        constraint_store_.second.insert(boost::make_tuple(md_it->first, 
                                                          i, 
                                                          false));

      }
    }
  }

}

VCSResult MathematicaVCSPoint::add_constraint(const tells_t& collected_tells, const appended_asks_t &appended_asks)
{
	if(Logger::constflag==9){
		HYDLA_LOGGER_AREA(
    "#*** Begin MathematicaVCSPoint::add_constraint ***");
	}

  HYDLA_LOGGER_DEBUG(
    "#*** Begin MathematicaVCSPoint::add_constraint ***");

  PacketSender ps(*ml_);


/////////////////// 送信処理

  // isConsistent[ pexpr, expr, vars]を渡したい
  ml_->put_function("isConsistent", 3);

  send_ps();

  // exprは3つの部分から成る
  ml_->put_function("Join", 3);
  int tells_size = collected_tells.size() + appended_asks.size();
  ml_->put_function("List", tells_size);
	if(Logger::constflag==9){
		HYDLA_LOGGER_AREA(
    "tells_size:", tells_size);
	}

  HYDLA_LOGGER_DEBUG(
    "tells_size:", tells_size);

  // tell制約の集合からexprを得てMathematicaに渡す
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    if(Logger::constflag==9){
	  HYDLA_LOGGER_AREA("put node: ", *(*tells_it)->get_child());
	}
	  HYDLA_LOGGER_DEBUG("put node: ", *(*tells_it)->get_child());
    ps.put_node((*tells_it)->get_child(), PacketSender::VA_None);
  }
  
  // appended_asksからガード部分を得てMathematicaに渡す
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    HYDLA_LOGGER_DEBUG("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    ps.put_node(append_it->ask->get_guard(), PacketSender::VA_None, false, append_it->entailed);
  }

  // 制約ストアからもexprを得てMathematicaに渡す
	send_cs();


  // 左連続性に関する制約を渡す
  // 現在採用している制約に出現する変数の最大微分回数よりも小さい微分回数のものについてprev(x)=x追加
  max_diff_map_t max_diff_map;
  create_max_diff_map(ps, max_diff_map);
  add_left_continuity_constraint(ps, max_diff_map);


  // varsを渡す
  ml_->put_function("Join", 2);
  ps.put_vars(PacketSender::VA_None);
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();
  
  //send_pars();


/////////////////// 受信処理
  HYDLA_LOGGER_DEBUG( "--- receive ---");
 
  //PacketChecker pc(*ml_);
  //pc.check();

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
    // 充足
    result = VCSR_TRUE;
    if(Logger::conflag==1||Logger::conflag==0){
     HYDLA_LOGGER_AREA("consistent");
    }
    HYDLA_LOGGER_SUMMARY("consistent");
    //無矛盾性判定
    // 解けた場合は解が「文字列で」返ってくるのでそれを制約ストアに入れる
    // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]や
    // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]や
    // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
    // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  やList[List["True"], List[]]など
	if(Logger::constflag==10){
		HYDLA_LOGGER_AREA( "---build constraint store---");
	}
	HYDLA_LOGGER_DEBUG( "---build constraint store---");
    ml_->MLGetNext();

    // 制約ストアをリセット
//    reset();
    constraint_store_.first.clear();

    // List関数の要素数（Orで結ばれた解の個数）を得る
    int or_size = ml_->get_arg_count();
  if(Logger::constflag==10){
	  HYDLA_LOGGER_AREA( "or_size: ", or_size);
  }
    HYDLA_LOGGER_DEBUG( "or_size: ", or_size);
    ml_->MLGetNext(); // Listという関数名

    for(int i=0; i<or_size; i++)
    {
      ml_->MLGetNext(); // List関数（Andで結ばれた解を表している）

      // List関数の要素数（Andで結ばれた解の個数）を得る
      int and_size = ml_->get_arg_count();
	  if(Logger::constflag==10){
		  HYDLA_LOGGER_AREA( "and_size: ", and_size);
	  }
      HYDLA_LOGGER_DEBUG( "and_size: ", and_size);
      ml_->MLGetNext(); // Listという関数名
      if(and_size > 0) ml_->MLGetNext(); // Listの中の先頭要素

      std::set<MathValue> value_set;    
      for(int j=0; j<and_size; j++)
      {
        std::string str = ml_->get_string();
        MathValue math_value;
        math_value.set(str);
        value_set.insert(math_value);
      }
      constraint_store_.first.insert(value_set);
    }

    constraint_store_.second.insert(ps.vars_begin(), ps.vars_end());
  }
  else {
    assert(ret_code==2);
    result = VCSR_FALSE;
	if(Logger::conflag==1||Logger::conflag==0){
     HYDLA_LOGGER_AREA("inconsistent");
    }
    HYDLA_LOGGER_SUMMARY("inconsistent");//矛盾
  }
  if(Logger::constflag==9){
	  HYDLA_LOGGER_AREA(
    *this,
    "\n#*** End MathematicaVCSPoint::add_constraint ***");
  }

  HYDLA_LOGGER_DEBUG(
    *this,
    "\n#*** End MathematicaVCSPoint::add_constraint ***");

  return result;
}
  
VCSResult MathematicaVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  if(Logger::enflag==1||Logger::enflag==0){
     HYDLA_LOGGER_AREA(	"#*** MathematicaVCSPoint::check_entailment ***\n", 
	"ask: ");
	 (negative_ask)->dump_infix(std::cout);
	 HYDLA_LOGGER_AREA("\n");
	}
  HYDLA_LOGGER_DEBUG(
    "#*** MathematicaVCSPoint::check_entailment ***\n", 
    "ask: ", *negative_ask);

  PacketSender ps(*ml_);

  // checkEntailment[guard, store, vars]を渡したい
  ml_->put_function("checkEntailment", 3);

  // ask制約のガードの式を得てMathematicaに渡す
  ps.put_node(negative_ask->get_guard(), PacketSender::VA_None);

  ml_->put_function("Join", 2);
  // 制約ストアとpsから式を得てMathematicaに渡す
  send_cs();
  send_ps();

  // varsを渡す
  ml_->put_function("Join", 3);
  ps.put_vars(PacketSender::VA_None);
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();
  
  send_pars();

  /*if(hydla::logger::Logger::flag==2||hydla::logger::Logger::flag==0){
     HYDLA_LOGGER_AREA(
		 "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
    }*/

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

////////// 受信処理

  // PacketChecker pc(*ml_);
  // pc.check();

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
	if(Logger::enflag==1||Logger::enflag==0){
     HYDLA_LOGGER_AREA("entailed");
	}
	if(Logger::conflag==1||Logger::conflag==0){
     HYDLA_LOGGER_AREA("Because entailed,isConsistency judgment is done again");
    }
    HYDLA_LOGGER_SUMMARY("entailed");
  }
  else if(ret_code==2) {
    result = VCSR_FALSE;
    if(Logger::enflag==1||Logger::enflag==0){
      HYDLA_LOGGER_AREA("not entailed");
    }
    HYDLA_LOGGER_SUMMARY("not entailed");
  }
  else {
    assert(ret_code==3);
    // TODO: VCSR_UNKNOWNを返し、分岐処理
    result = VCSR_UNKNOWN;
  }
  return result;
}

VCSResult MathematicaVCSPoint::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time,
  const not_adopted_tells_list_t& not_adopted_tells_list)
{
  // Pointではintegrate関数無効
  assert(0);
  return VCSR_FALSE;
}

void MathematicaVCSPoint::send_cs() const
{
	if(Logger::mathsendflag==3){
		HYDLA_LOGGER_AREA("---- Send Constraint Store -----");
	}

  HYDLA_LOGGER_DEBUG("---- Send Constraint Store -----");

  int or_cons_size = constraint_store_.first.size();
  if(or_cons_size <= 0)
  {
	  if(Logger::mathsendflag==3){
		  HYDLA_LOGGER_AREA("no Constraints");
	  }
    HYDLA_LOGGER_DEBUG("no Constraints");
    ml_->put_function("List", 0);
    return;
  }

  ml_->put_function("List", 1);
  ml_->put_function("Or", or_cons_size);
  if(Logger::mathsendflag==3){
	  HYDLA_LOGGER_AREA("or cons size: ", or_cons_size);
  }
  HYDLA_LOGGER_DEBUG("or cons size: ", or_cons_size);

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_end = 
    constraint_store_.first.end();
  for(; or_cons_it!=or_cons_end; ++or_cons_it)
  {
    int and_cons_size = (*or_cons_it).size();
    ml_->put_function("And", and_cons_size);
  if(Logger::mathsendflag==3){
	  HYDLA_LOGGER_AREA("and cons size: ", and_cons_size);
  }
    HYDLA_LOGGER_DEBUG("and cons size: ", and_cons_size);

    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    std::set<MathValue>::const_iterator and_cons_end = 
      (*or_cons_it).end();
    for(; and_cons_it!=and_cons_end; ++and_cons_it)
    {
      ml_->put_function("ToExpression", 1);
      std::string str = (*and_cons_it).get_string();
      ml_->put_string(str);
   if(Logger::mathsendflag==3){
	   HYDLA_LOGGER_AREA("put cons: ", str);
   }
      HYDLA_LOGGER_DEBUG("put cons: ", str);
    }
  }
}


void MathematicaVCSPoint::send_ps() const
{
  HYDLA_LOGGER_DEBUG("---- Send Parameter Store -----");

  int or_cons_size = parameter_store_.first.size();
  if(or_cons_size <= 0)
  {
    HYDLA_LOGGER_DEBUG("no Parameters");
    ml_->put_function("List", 0);
    return;
  }

  ml_->put_function("List", 1);
  ml_->put_function("Or", or_cons_size);
  HYDLA_LOGGER_DEBUG("or cons size: ", or_cons_size);

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    parameter_store_.first.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_end = 
    parameter_store_.first.end();
  for(; or_cons_it!=or_cons_end; ++or_cons_it)
  {
    int and_cons_size = (*or_cons_it).size();
    ml_->put_function("And", and_cons_size);
    HYDLA_LOGGER_DEBUG("and cons size: ", and_cons_size);

    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    std::set<MathValue>::const_iterator and_cons_end = 
      (*or_cons_it).end();
    for(; and_cons_it!=and_cons_end; ++and_cons_it)
    {
      ml_->put_function("ToExpression", 1);
      std::string str = (*and_cons_it).get_string();
      ml_->put_string(str);
      HYDLA_LOGGER_DEBUG("put cons: ", str);
    }
  }
}


void MathematicaVCSPoint::send_pars() const{
  ml_->put_function("List", par_names_.size());
  for(std::set<std::string>::const_iterator it=par_names_.begin();it!=par_names_.end();it++){
    ml_->put_symbol(PacketSender::par_prefix + *it);
  }
}

void MathematicaVCSPoint::send_cs_vars() const
{
  int vars_size = constraint_store_.second.size();

  if(Logger::mathsendflag==1){
	    HYDLA_LOGGER_AREA(
    "---- Send Constraint Store Vars -----\n",
    "vars_size: ", vars_size);
  }

  HYDLA_LOGGER_DEBUG(
    "---- Send Constraint Store Vars -----\n",
    "vars_size: ", vars_size);

  PacketSender ps(*ml_);
  
  ml_->put_function("List", vars_size);

  constraint_store_vars_t::const_iterator it = 
    constraint_store_.second.begin();
  constraint_store_vars_t::const_iterator end = 
    constraint_store_.second.end();
  for(; it!=end; ++it) {
    ps.put_var(*it, PacketSender::VA_None);
  }
}

std::ostream& MathematicaVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSPoint ***\n"
    << "--- constraint store ---\n";

  // 
  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    std::set<MathValue>::const_iterator and_cons_it = 
      (*or_cons_it).begin();
    while((and_cons_it) != (*or_cons_it).end())
    {
      s << (*and_cons_it).get_string() << " ";
      and_cons_it++;
    }
    s << "\n";
    or_cons_it++;
  }

  // 制約ストア内に存在する変数のダンプ
  s << "-- vars --\n";
  constraint_store_vars_t::const_iterator vars_it = 
    constraint_store_.second.begin();
  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << "\n";
    vars_it++;
  }

  return s;
}

std::ostream& operator<<(std::ostream& s, const MathematicaVCSPoint& m)
{
  return m.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

