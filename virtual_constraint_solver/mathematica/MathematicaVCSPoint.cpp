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
    const MathValue&    value = (*it).second;    

    if(!value.is_undefined()) {
      std::ostringstream val_str;

      // MathVariable側に関する文字列を作成
      val_str << "Equal[";
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
              << value.str
              << "]"; // Equalの閉じ括弧

      MathValue new_math_value;
      new_math_value.str = val_str.str();
      and_cons_set.insert(new_math_value);

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

bool MathematicaVCSPoint::create_variable_map(variable_map_t& variable_map)
{
  HYDLA_LOGGER_DEBUG(
    "#*** MathematicaVCSPoint::create_variable_map ***\n",
    "--- variable_map ---\n",
    variable_map,
    "--- constraint_store ---\n",
    *this);    

  // 制約ストアが空（true）の場合は変数表も空で良い
  if(cs_is_true()) return true;


/////////////////// 送信処理

  // convertCSToVM[exprs]を渡したい
  ml_->put_function("convertCSToVM", 1);
  send_cs();


/////////////////// 受信処理

//   PacketChecker pc(*ml_);
//   pc.check();

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));

  ml_->skip_pkt_until(RETURNPKT);

  ml_->MLGetNext();
//  ml_->MLGetNext();

  // List関数の要素数（式の個数）を得る
  int expr_size = ml_->get_arg_count();
  ml_->MLGetNext(); // Listという関数名


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

    MathVariable symbolic_variable;
    MathValue symbolic_value;
    symbolic_variable.name = variable_name;
    symbolic_variable.derivative_count = variable_derivative_count;    
    symbolic_value.str = value_str;

    // 関係演算子コードを元に、変数表の対応する部分に代入する
    // TODO: Orの扱い
    switch(relop_code)
    {
      case 0: // Equal
        variable_map.set_variable(symbolic_variable, symbolic_value);
        break;
      case 1: // Less
        break;
      case 2: // Greater
        break;
      case 3: // LessEqual
        break;
      case 4: // GreaterEqual
        break;        
    }
  }
    

/*
  // 一つだけ採用
  // TODO: 複数解ある場合の処理もきちんと考える
//  assert(constraint_store_.first.size() == 1);
//  for(; or_cons_it!=or_cons_end; ++or_cons_it) {
    std::set<MathValue>::const_iterator and_cons_it = (*or_cons_it).begin();
    std::set<MathValue>::const_iterator and_cons_end = (*or_cons_it).end();
    for(; (and_cons_it) != and_cons_end; and_cons_it++)
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
        variable_name =  variable_str.substr(variable_loc, bracket_loc-variable_loc);
      }
      else
      {
        variable_name =  variable_str; // "usrVar"の長さ分      
        variable_derivative_count = 0;
      }

      // prev変数でなかったら処理
      if(!boost::starts_with(variable_name, "prev")) {
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
        symbolic_variable.name = 
          variable_name.substr(PacketSender::var_prefix.size());
        symbolic_variable.derivative_count = 
          variable_derivative_count;
        symbolic_value.str = value_str;

        variable_map.set_variable(symbolic_variable, symbolic_value);
      } 
    }
//  }
*/

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

  HYDLA_LOGGER_DEBUG("left_cont_vars_count(in cs_var + in vars): ", left_cont_vars_count);  



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

VCSResult MathematicaVCSPoint::add_constraint(const tells_t& collected_tells)
{
  HYDLA_LOGGER_DEBUG(
    "#*** Begin MathematicaVCSPoint::add_constraint ***");

  PacketSender ps(*ml_);


/////////////////// 送信処理

  // isConsistent[expr, vars]を渡したい
  ml_->put_function("isConsistent", 2);

  // exprは3つの部分から成る
  ml_->put_function("Join", 3);
  int tells_size = collected_tells.size();
  ml_->put_function("List", tells_size);

  // tell制約の集合からexprを得てMathematicaに渡す
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    HYDLA_LOGGER_DEBUG("put node: ", *(*tells_it)->get_child());
    ps.put_node((*tells_it)->get_child(), PacketSender::VA_None);
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
  
//   ml_->skip_pkt_until(TEXTPKT);
//   std::cout << ml_->get_string() << std::endl;


/////////////////// 受信処理
  HYDLA_LOGGER_DEBUG( "--- receive ---");

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
    HYDLA_LOGGER_SUMMARY("consistent");//無矛盾性判定
    // 解けた場合は解が「文字列で」返ってくるのでそれを制約ストアに入れる
    // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]や
    // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]や
    // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
    // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  やList[List["True"], List[]]など
    HYDLA_LOGGER_DEBUG( "---build constraint store---");
    ml_->MLGetNext();

    // 制約ストアをリセット
//    reset();
    constraint_store_.first.clear();

    // List関数の要素数（Orで結ばれた解の個数）を得る
    int or_size = ml_->get_arg_count();
    HYDLA_LOGGER_DEBUG( "or_size: ", or_size);
    ml_->MLGetNext(); // Listという関数名

    for(int i=0; i<or_size; i++)
    {
      ml_->MLGetNext(); // List関数（Andで結ばれた解を表している）

      // List関数の要素数（Andで結ばれた解の個数）を得る
      int and_size = ml_->get_arg_count();
      HYDLA_LOGGER_DEBUG( "and_size: ", and_size);
      ml_->MLGetNext(); // Listという関数名
      ml_->MLGetNext(); // Listの中の先頭要素

      std::set<MathValue> value_set;    
      for(int j=0; j<and_size; j++)
      {
        std::string str = ml_->get_string();
        MathValue math_value;
        math_value.str = str;
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

  // 制約ストアから式を得てMathematicaに渡す
  send_cs();

  // varsを渡す
  ml_->put_function("Join", 2);
  ps.put_vars(PacketSender::VA_None);
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();

  /*if(hydla::logger::Logger::flag==2||hydla::logger::Logger::flag==0){
     HYDLA_LOGGER_AREA(
		 "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
    }*/

  HYDLA_LOGGER_DEBUG(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

////////// 受信処理

//   PacketChecker pc(*ml_);
//   pc.check();

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
  else {
    assert(ret_code==2);
    result = VCSR_FALSE;
	if(Logger::enflag==1||Logger::enflag==0){
     HYDLA_LOGGER_AREA("not entailed");
    }
    HYDLA_LOGGER_SUMMARY("not entailed");
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
  HYDLA_LOGGER_DEBUG("---- Send Constraint Store -----");

  int or_cons_size = constraint_store_.first.size();
  if(or_cons_size <= 0)
  {
    HYDLA_LOGGER_DEBUG("no Constraints");
    ml_->put_function("List", 0);
    return;
  }

  ml_->put_function("List", 1);
  ml_->put_function("Or", or_cons_size);
  HYDLA_LOGGER_DEBUG("or cons size: ", or_cons_size);

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  std::set<std::set<MathValue> >::const_iterator or_cons_end = 
    constraint_store_.first.end();
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
      std::string str = (*and_cons_it).str;
      ml_->put_string(str);
      HYDLA_LOGGER_DEBUG("put cons: ", str);
    }
  }
}

void MathematicaVCSPoint::send_cs_vars() const
{
  int vars_size = constraint_store_.second.size();

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
      s << (*and_cons_it).str << " ";
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

