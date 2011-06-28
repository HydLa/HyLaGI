
#include <cassert>
#include <boost/algorithm/string/predicate.hpp>

#include "REDUCEVCSPoint.h"

// TODO RTreeVisitorの引っ越し
//#include "../../parser/RTreeVisitor.h"
#include "REDUCEStringSender.h"


/*
#include "mathlink_helper.h"
#include "PacketErrorHandler.h"
#include "PacketChecker.h"
 */

#include "../mathematica/MathematicaExpressionConverter.h"

#include "Logger.h"

using namespace hydla::vcs;
using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace reduce {

//REDUCEVCSPoint::REDUCEVCSPoint(MathLink* ml) :ml_(ml)
REDUCEVCSPoint::REDUCEVCSPoint(REDUCELink* cl) :
  cl_(cl)
{
  std::cout << "Begin REDUCEVCSPoint::REDUCEVCSPoint(REDUCEClient* cl)" << std::endl;

}

REDUCEVCSPoint::~REDUCEVCSPoint()
{
  std::cout << "Begin REDUCEVCSPoint::~REDUCEVCSPoint()" << std::endl;

}

// MathematicaVCSPointより
bool REDUCEVCSPoint::reset()
{
  std::cout << "Begin REDUCEVCSPoint::reset()" << std::endl;
  // TODO: チョイ考える
  assert(0);
  //   constraint_store_.first.clear();
  //   constraint_store_.second.clear();
  return true;
}

// MathematicaVCSPointより
bool REDUCEVCSPoint::reset(const variable_map_t& variable_map)
{

  HYDLA_LOGGER_VCS_SUMMARY("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", variable_map);

  std::set<MathValue> and_cons_set;

  MathematicaExpressionConverter mec;

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
              << mec.convert_symbolic_value_to_math_string(value)
              << "]"; // Equalの閉じ括弧

      MathValue new_math_value;
      new_math_value.set(val_str.str());
      and_cons_set.insert(new_math_value);

      // 制約ストア内の変数一覧を作成
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
    }
  }

//  constraint_store_.first.insert(and_cons_set);

  HYDLA_LOGGER_VCS(*this);

  return true;
}

// MathematicaVCSPointより
bool REDUCEVCSPoint::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map){
//  assert(0);
//  return false;
  if(!reset(variable_map)){
    return false;
  }

  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", parameter_map);


  std::set<MathValue> and_cons_set;
  par_names_.clear();

  parameter_map_t::variable_list_t::const_iterator it =
    parameter_map.begin();
  parameter_map_t::variable_list_t::const_iterator end =
    parameter_map.end();
  for(; it!=end; ++it)
  {
    const value_range_t&    value = it->second;
    if(!value.is_undefined()) {
      value_range_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
      for(;or_it != or_end; or_it++){
        value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
        for(; and_it != and_end; and_it++){
          std::ostringstream val_str;

          // MathVariable側に関する文字列を作成
          val_str << MathematicaExpressionConverter::get_relation_math_string(and_it->relation) << "[" ;

          val_str << PacketSender::par_prefix
                  << it->first.get_name();

          val_str << ","
                  << and_it->value
                  << "]"; // 閉じ括弧
          MathValue new_math_value;
          new_math_value.set(val_str.str());
          and_cons_set.insert(new_math_value);
          par_names_.insert(it->first.get_name());
        }
      }
    }
  }
//  parameter_store_.first.insert(and_cons_set);
  return true;
}
//TODO 定数返しの修正
bool REDUCEVCSPoint::create_variable_map(variable_map_t& variable_map, parameter_map_t& parameter_map)
{
  assert(0);
  return false;
}

//TODO 定数返しの修正
void REDUCEVCSPoint::create_max_diff_map(
    PacketSender& ps, max_diff_map_t& max_diff_map)
{
  assert(0);
}

//TODO 定数返しの修正
void REDUCEVCSPoint::add_left_continuity_constraint(
    PacketSender& ps, max_diff_map_t& max_diff_map)
{
  assert(0);
}

VCSResult REDUCEVCSPoint::add_constraint(const tells_t& collected_tells, const appended_asks_t &appended_asks)
{

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::add_constraint ***");
  REDUCEStringSender rss = REDUCEStringSender(*cl_);


//////////////////// 送信処理

  // send_stringのstringはどのように区切って送信してもOK

  // depend文
  // TODO:処理の一般化
  cl_->send_string("depend ht,t;");
  cl_->send_string("depend v,t;");

  // tell制約の集合からexprを得てREDUCEに渡す
  std::cout << "collected_tells" << std::endl;
  cl_->send_string("expr_:={");
//  cl_->send_string("expr_:={df(y,t,2) = -10,");
//  cl_->send_string("y = 10, df(y,t,1) = 0, prev(y) = y, df(prev(y),t,1) = df(y,t,1)};");

  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    if(tells_it != collected_tells.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", *(*tells_it)->get_child());
    rss.put_node((*tells_it)->get_child());
  }
  cl_->send_string("};");


  // appended_asksからガード部分を得てREDUCEに渡す
  std::cout << "appended_asks" << std::endl;
  cl_->send_string("pexpr_:={");
//  cl_->send_string("{}");

  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    if(append_it != appended_asks.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_child());
  }
  cl_->send_string("};");


  // varsを渡す
  //   ex) {y, prev(y), df(y,t,1), df(prev(y),t,1), df(y,t,2), y, prev(y), df(y,t,1), df(prev(y),t,1)}
  // vars_に関して一番外側の"{}"部分は、put_vars内で送っている
  cl_->send_string("vars_:=");
  rss.put_vars();
  cl_->send_string(";");


  cl_->send_string("symbolic redeval '(isconsistent vars_ pexpr_ expr_);");


/////////////////// 受信処理
  HYDLA_LOGGER_VCS("--- receive ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ",
                   ans);

  VCSResult result;

  // S式パーサで読み取る
  sp_.parse_main(ans.c_str());

  // {コード, {{{変数, 関係演算子コード, 値},...}, ...}}の構造
  const_tree_iter_t ct_it = sp_.get_tree_iterator();

  // コードを取得
  const_tree_iter_t ret_code_it = ct_it->children.begin();
  std::string ret_code_str = std::string(ret_code_it->value.begin(), ret_code_it->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ",
                   ret_code_str);

  if(ret_code_str=="RETERROR___"){
    // ソルバエラー
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code_str==" \"RETTRUE___\"") {
    // 充足
    // TODO: スペースや""が残らないようにパーサを修正
    HYDLA_LOGGER_VCS( "---build constraint store---");

    // 制約ストアをリセット
    //    reset();
//    constraint_store_.first.clear();
    //     constraint_store_.first = NULL;

    // 制約ストア構築
    constraint_store_.first = ret_code_it+1;
    constraint_store_.second.insert(rss.vars_begin(), rss.vars_end());

  }
  else {
    assert(ret_code_str == " \"RETFALSE___\"");
    result = VCSR_FALSE;
  }

  HYDLA_LOGGER_VCS(
    *this,
    "\n#*** End REDUCEVCSPoint::add_constraint ***");




/*
  size_t size = ct_it->children.size();
  std::cout << "children size: " << size << "\n";


  for(size_t j=0; j<size; j++) {
    SExpParser::const_tree_iter_t child_it = ct_it->children.begin()+j;
    std::cout << j << "th child\n";
    std::cout << "ID: " << child_it->value.id().to_long() << "\n";
    std::cout << "Node:" << std::string(child_it->value.begin(), child_it->value.end()) << "\n";
  }
*/ 

  return result;

}

//TODO 定数返しの修正
VCSResult REDUCEVCSPoint::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t &appended_asks)
{

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_entailment ***",
                   "ask: ", *negative_ask);
  REDUCEStringSender rss = REDUCEStringSender(*cl_);


/////////////////// 送信処理

  // ask制約のガードの式を得てMathematicaに渡す
  cl_->send_string("guard_:=");
  rss.put_node(negative_ask->get_guard());
  cl_->send_string(";");  


  // 制約ストアとpsから式を得てMathematicaに渡す
  cl_->send_string("store_:=append(");
  send_cs();
  cl_->send_string(",");
  send_ps();
  cl_->send_string(");");


  // varsを渡す
  cl_->send_string("vars_:=append(");
  rss.put_vars();
  cl_->send_string(",");
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();
  cl_->send_string(");");


  cl_->send_string("symbolic redeval '(checkentailment guard_ store_ vars_);");


/////////////////// 受信処理
  HYDLA_LOGGER_VCS( "--- receive ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("check_entailment_ans: ",
                   ans);

  // S式パーサで読み取る
  sp_.parse_main(ans.c_str());

  // {コード}の構造
  const_tree_iter_t ct_it = sp_.get_tree_iterator();
  size_t size = ct_it->children.size();
  std::cout << "children size: " << size << "\n";






  assert(0);

  return VCSR_FALSE;
}


VCSResult REDUCEVCSPoint::integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list,
    const appended_asks_t& appended_asks)
{
  // Pointではintegrate関数無効
  assert(0);
  return VCSR_FALSE;
}

void REDUCEVCSPoint::send_cs() const
{


  HYDLA_LOGGER_VCS("---- Send Constraint Store -----");

  size_t or_cons_size = constraint_store_.first->children.size();
  HYDLA_LOGGER_VCS("or cons size: ", or_cons_size);

  if(or_cons_size <= 0)
  {
    HYDLA_LOGGER_VCS("no Constraints");
    cl_->send_string("{}");
    return;
  }

  // TODO: 複数解（or_cons_size>1）の場合の対処を考える
  for(size_t i=0; i<or_cons_size; i++){
    const_tree_iter_t or_cons_it = constraint_store_.first->children.begin()+i;
    size_t and_cons_size = or_cons_it->children.size();
    HYDLA_LOGGER_VCS("and cons size: ", and_cons_size);
    for(size_t j=0; j<and_cons_size; j++){
      const_tree_iter_t and_cons_it = or_cons_it->children.begin()+j;
      std::string relop = std::string(and_cons_it->value.begin(), and_cons_it->value.end());
      std::cout << "relop: " << relop << "\n";
      size_t var_info_size = and_cons_it->children.size();
      std::cout << "var info size: " << var_info_size << "\n";
      cl_->send_string("vars_:=");


    }
  }




}

void REDUCEVCSPoint::send_ps() const
{
  HYDLA_LOGGER_VCS("---- Send Parameter Store -----");

  // TODO: ちゃんと送る
  cl_->send_string("{}");
}

//TODO 定数返しの修正
void REDUCEVCSPoint::send_pars() const{
  assert(0);
}

//TODO 定数返しの修正
void REDUCEVCSPoint::send_cs_vars() const
{
   assert(0);
}

// MathematicaVCSPointより
std::ostream& REDUCEVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSPoint ***\n"
      << "--- constraint store ---\n";

  //
/*
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
*/

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

// MathematicaVCSPointより
std::ostream& operator<<(std::ostream& s, const REDUCEVCSPoint& m)
{
  return m.dump(s);
}


} // namespace reduce
} // namespace simulator
} // namespace hydla

