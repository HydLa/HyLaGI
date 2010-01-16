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
  return cons_store_.reset();
}

bool MathematicaVCSPoint::reset(const variable_map_t& vm)
{
  return cons_store_.reset(vm);
}

bool MathematicaVCSPoint::create_variable_map(variable_map_t& vm)
{
  return cons_store_.create_variable_map(vm);
}

Trivalent MathematicaVCSPoint::add_constraint(const tells_t& collected_tells)
{
  HYDLA_LOGGER_DEBUG("#*** add constraint ***");

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
    ps.visit((*tells_it));
    tells_it++;
  }

  // 制約ストアからもexprを得てMathematicaに渡す
  cons_store_.send_cs(ml_);

  // varsを渡す
  ml_->put_function("Join", 2);
  ps.put_vars();
  // 制約ストア内に出現する変数も渡す
  cons_store_.send_cs_vars(ml_);

  // 結果を受け取る前に制約ストアを初期化
  cons_store_.reset();

/*
//ml_->skip_pkt_until(RETURNPKT);
// 返ってくるパケットを解析
PacketChecker pc(ml_);
pc.check();
*/

  ml_->skip_pkt_until(RETURNPKT);
  // 解けなかった場合は0が返る（制約間に矛盾があるということ）
  if(ml_->MLGetType() == MLTKINT)
  {
    HYDLA_LOGGER_DEBUG("Consistency Check : false");
    ml_->MLNewPacket();
    return Tri_FALSE;
  }

  // 解けた場合は解が「文字列で」返ってくるのでそれを制約ストアに入れる
  // List[List[List["Equal[x, 1]"], List["Equal[x, -1]"]], List[x]]や
  // List[List[List["Equal[x, 1]", "Equal[y, 1]"], List["Equal[x, -1]", "Equal[y, -1]"]], List[x, y, z]]や
  // List[List[List["Equal[x,1]", "Equal[Derivative[1][x],1]", "Equal[prev[x],1]", "Equal[prev[Derivative[2][x]],1]"]],
  // List[x, Derivative[1][x], prev[x], prev[Derivative[2][x]]]]  やList[List["True"], List[]]など
  HYDLA_LOGGER_DEBUG( "---build constraint store---");

  ml_->MLGetNext(); // Listという関数名
  ml_->MLGetNext(); // List関数（Orで結ばれた解を表している）

  // List関数の要素数（Orで結ばれた解の個数）を得る
  int or_size = ml_->get_arg_count();
  ml_->MLGetNext(); // Listという関数名

  for(int i=0; i<or_size; i++)
  {
    ml_->MLGetNext(); // List関数（Andで結ばれた解を表している）

    // List関数の要素数（Andで結ばれた解の個数）を得る
    int and_size = ml_->get_arg_count();
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
    cons_store_.store.first.insert(value_set);
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

    cons_store_.store.second.insert(symbolic_variable);
  }


  //if(debug_mode_) {
  //  std::set<std::set<MathValue> >::iterator or_cons_it;
  //  std::set<MathValue>::iterator and_cons_it;
  //  or_cons_it = constraint_store.first.begin();
  //  while((or_cons_it) != constraint_store.first.end())
  //  {
  //    and_cons_it = (*or_cons_it).begin();
  //    while((and_cons_it) != (*or_cons_it).end())
  //    {
  //      std::cout << (*and_cons_it).str << " ";
  //      and_cons_it++;
  //    }
  //    std::cout << std::endl;
  //    or_cons_it++;
  //  }
  //  std::cout << "----------------------------" << std::endl;
  //  std::cout << "ConsistencyChecker: true" << std::endl;
  //}


  return Tri_TRUE;
}
  
Trivalent MathematicaVCSPoint::check_entailment(const ask_node_sptr& negative_ask)
{
  // checkEntailment[guard, store, vars]を渡したい
  ml_->put_function("checkEntailment", 3);


  // ask制約のガードの式を得てMathematicaに渡す
  PacketSender ps(*ml_, PacketSender::NP_POINT_PHASE);
  ps.put_node(negative_ask);

  // 制約ストアから式を得てMathematicaに渡す
  cons_store_.send_cs(ml_);

  // varsを渡す
  ml_->put_function("Join", 2);
  ps.put_vars();
  // 制約ストア内に出現する変数も渡す
  cons_store_.send_cs_vars(ml_);

  ml_->skip_pkt_until(RETURNPKT);
  
  int num  = ml_->get_integer();
  HYDLA_LOGGER_DEBUG("EntailmentChecker#num:", num);

  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  return num==1 ? Tri_TRUE : Tri_FALSE;
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


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

