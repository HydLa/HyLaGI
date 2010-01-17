#include "MathematicaVCSPoint.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"
#include "PacketSender.h"

using namespace hydla::vcs;

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
  constraint_store_ = constraint_store_t();
  return true;
}

bool MathematicaVCSPoint::reset(const variable_map_t& variable_map)
{
  HYDLA_LOGGER_DEBUG("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_DEBUG("no Variables");
    return true;
  }
  HYDLA_LOGGER_DEBUG("------Variable map------\n", variable_map);

  variable_map_t::variable_list_t::const_iterator it = 
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = 
    variable_map.end();
  for(; it!=end; ++it)
  {
    const MathVariable& symbolic_variable = (*it).first;
    const MathValue& symbolic_value = (*it).second;    

    if(symbolic_value.str != "") { //未定義かどうか  TODO: 未定義判定関数つくる？
      std::ostringstream val_str;

      // MathVariable側に関する文字列を作成
      val_str << "Equal[";
      if(symbolic_variable.derivative_count > 0)
      {
        val_str << "Derivative["
                << symbolic_variable.derivative_count
                << "][prev[usrVar"
                << symbolic_variable.name
                << "]]";
      }
      else
      {
        val_str << "prev[usrVar"
                << symbolic_variable.name
                << "]";
      }

      val_str << ","
              << symbolic_value.str
              << "]"; // Equalの閉じ括弧

      MathValue new_symbolic_value;
      new_symbolic_value.str = val_str.str();
      std::set<MathValue> value_set;
      value_set.insert(new_symbolic_value);
      constraint_store_.first.insert(value_set);

      // 制約ストア内の変数一覧を作成
      MathVariable new_variable(symbolic_variable);
      new_variable.name = "prev[usrVar" + symbolic_variable.name + "]";
      constraint_store_.second.insert(new_variable);
    }
  }

  HYDLA_LOGGER_DEBUG(*this);

  return true;
}

bool MathematicaVCSPoint::create_variable_map(variable_map_t& variable_map)
{
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
  // prev[]を除く処理は要らなさそう？
  return true;
}

VCSResult MathematicaVCSPoint::add_constraint(const tells_t& collected_tells)
{
  HYDLA_LOGGER_DEBUG(
    "#*** Begin MathematicaVCSPoint::add_constraint ***");

  // isConsistent[expr, vars]を渡したい
  ml_->put_function("isConsistent", 2);

  ml_->put_function("Join", 2);
  // tell制約の集合からexprを得てMathematicaに渡す
  int tells_size = collected_tells.size();
  ml_->put_function("List", tells_size);
  tells_t::const_iterator tells_it = collected_tells.begin();
  PacketSender ps(*ml_);
  while((tells_it) != collected_tells.end())
  {
    ps.put_node(*tells_it);
    tells_it++;
  }

  // 制約ストアからもexprを得てMathematicaに渡す
  send_cs();

  // varsを渡す
  ml_->put_function("Join", 2);
  ps.put_vars();
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();

  // 結果を受け取る前に制約ストアを初期化
  reset();


  ml_->skip_pkt_until(RETURNPKT);
  // 解けなかった場合は0が返る（制約間に矛盾があるということ）
  HYDLA_LOGGER_DEBUG("GetType: ", ml_->MLGetType());
  if(ml_->MLGetType() == 35)
  {
    std::cout << ml_->get_symbol() << std::endl;
    HYDLA_LOGGER_DEBUG("Consistency Check : false");
    ml_->MLNewPacket();
    return VCSR_FALSE;
  }

  // 解けた場合は解が「文字列で」返ってくるのでそれを制約ストアに入れる
  // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]や
  // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]や
  // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
  // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  やList[List["True"], List[]]など
  HYDLA_LOGGER_DEBUG( "---build constraint store---");
  std::cout << "a" << std::endl;
  ml_->MLGetNext(); // Listという関数名
  std::cout << "b" << std::endl;
  ml_->MLGetNext(); // List関数（Orで結ばれた解を表している）
  std::cout << "c" << std::endl;

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


  // 出現する変数の一覧が文字列で返ってくるのでそれを制約ストアに入れる
  ml_->MLGetNext(); // List関数

  // List関数の要素数（変数一覧に含まれる変数の個数）を得る
  int vars_size = ml_->get_arg_count();
  ml_->MLGetNext(); // Listという関数名

  for(int k=0; k<vars_size; k++)
  {
    MathVariable symbolic_variable;
    switch(ml_->MLGetNext())
    {
      case MLTKFUNC: // Derivative[number][]
        ml_->MLGetNext(); // Derivative[number]という関数名
        ml_->MLGetNext(); // Derivativeという関数名
        ml_->MLGetNext(); // number
        symbolic_variable.derivative_count =
          ml_->get_integer();
        ml_->MLGetNext(); // 変数
        symbolic_variable.name = ml_->get_symbol();
        break;
      case MLTKSYM: // シンボル（記号）xとかyとか
        symbolic_variable.derivative_count = 0;
        symbolic_variable.name = ml_->get_symbol();
        break;
      default:
        ;
    }

    constraint_store_.second.insert(symbolic_variable);
  }

  HYDLA_LOGGER_DEBUG(
    *this,
    "\n#*** End MathematicaVCSPoint::add_constraint ***");
  
  return VCSR_TRUE;
}
  
VCSResult MathematicaVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  HYDLA_LOGGER_DEBUG(
    "#*** MathematicaVCSPoint::check_entailment ***\n", 
    "ask: ", *negative_ask);

  // checkEntailment[guard, store, vars]を渡したい
  ml_->put_function("checkEntailment", 3);


  // ask制約のガードの式を得てMathematicaに渡す
  PacketSender ps(*ml_, PacketSender::NP_POINT_PHASE);
  ps.put_node(negative_ask);

  // 制約ストアから式を得てMathematicaに渡す
  send_cs();

  // varsを渡す
  ml_->put_function("Join", 2);
  ps.put_vars();
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();

  ml_->skip_pkt_until(RETURNPKT);
  
  int num  = ml_->get_integer();
  HYDLA_LOGGER_DEBUG("EntailmentChecker#num:", num);

  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1 ? VCSR_TRUE : VCSR_FALSE;
}

bool MathematicaVCSPoint::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time)
{
  // Pointではintegrate関数無効
  assert(0);
  return false;
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

void MathematicaVCSPoint::send_cs_vars() const
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

std::ostream& MathematicaVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSPoint ***\n"
    << "--- constraint store ---\n";

  std::set<std::set<MathValue> >::const_iterator or_cons_it = 
    constraint_store_.first.begin();
  std::set<MathVariable>::const_iterator vars_it = 
    constraint_store_.second.begin();
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

std::ostream& operator<<(std::ostream& s, const MathematicaVCSPoint& m)
{
  return m.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

