
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


  // まず、変数表の中身を表すようなS式の文字列を作る
  std::ostringstream vm_str;
  vm_str << "(list ";

  variable_map_t::variable_list_t::const_iterator it =
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end =
    variable_map.end();
  for(; it!=end; ++it)
  {
    if(it!=variable_map.begin()) vm_str << " ";

    const MathVariable& variable = (*it).first;
    const value_t&    value = it->second;

    if(!value.is_undefined()) {
      vm_str << "(equal ";

      // variable部分
      if(variable.derivative_count > 0)
      {
        vm_str << "(prev (df "
               << variable.name
               << " t "
               << variable.derivative_count
               << "))";
      }
      else
      {
        vm_str << "(prev "
               << variable.name
               << ")";
      }

      // value部分
      vm_str << " "
             << value // TODO:正しい処理を行うように。現在は中置記法になってしまう？ので、S式の形式にしたい
             << ")"; // equalの閉じ括弧


      // 制約ストア内の変数一覧にvariableを追加
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
    }
  }

  vm_str << ")"; // listの閉じ括弧
  HYDLA_LOGGER_VCS("vm_str: ", vm_str.str());  
  
  // sp_にvm_strを読み込ませて、S式のパースツリーを構築し、先頭のポインタを得る
  sp_.parse_main((vm_str.str()).c_str());
  const_tree_iter_t ct_it = sp_.get_tree_iterator();
  constraint_store_.first = ct_it;


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

void REDUCEVCSPoint::create_max_diff_map(
    REDUCEStringSender& rss, max_diff_map_t& max_diff_map)
{
  REDUCEStringSender::vars_const_iterator vars_it  = rss.vars_begin();
  REDUCEStringSender::vars_const_iterator vars_end = rss.vars_end();
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

void REDUCEVCSPoint::add_left_continuity_constraint(
    REDUCEStringSender& rss, max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSPoint::add_left_continuity_constraint ----");


  cl_->send_string("append({");
  // 制約ストア中の変数のうち、集めたtell制約に出現する最大微分回数より小さい微分回数であるもののみ追加
  HYDLA_LOGGER_VCS("--- in cs_var ---");

  constraint_store_vars_t::const_iterator cs_vars_it  = constraint_store_.second.begin();
  constraint_store_vars_t::const_iterator cs_vars_end = constraint_store_.second.end();
  bool first_element = true;
  for(; cs_vars_it!=cs_vars_end; ++cs_vars_it) {
    max_diff_map_t::const_iterator md_it =
      max_diff_map.find(cs_vars_it->get<0>());
    if(md_it!=max_diff_map.end() &&
       md_it->second  > cs_vars_it->get<1>())
    {
      if(!first_element) cl_->send_string(",");
      // Prev変数側
      // 変数名
      rss.put_var(
        boost::make_tuple(cs_vars_it->get<0>(),
                          cs_vars_it->get<1>(),
                          true));

      cl_->send_string("=");

      // Now変数側
      // 変数名
      rss.put_var(
        boost::make_tuple(cs_vars_it->get<0>(),
                          cs_vars_it->get<1>(),
                          false));
      first_element = false;
    }
  }
  cl_->send_string("},{");


  // 集めたtell制約内の変数についても調べる
  // 時刻0（制約ストアが空）対策のため
  HYDLA_LOGGER_VCS("--- in vars ---");

  // max_diff_mapについてつくる
  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();
  first_element = true;
  for(; md_it!=md_end; ++md_it) {
    if(constraint_store_.second.find(md_it->first)==constraint_store_.second.end()){
      for(int i=0; i<md_it->second; ++i){
        if(!first_element) cl_->send_string(",");
        // Prev変数側
        // 変数名
        rss.put_var(
          boost::make_tuple(md_it->first,
                            i,
                            true));

        cl_->send_string("=");

        // Now変数側
        // 変数名
        rss.put_var(
          boost::make_tuple(md_it->first,
                            i,
                            false));

        // 制約ストア内の変数扱いする
        // こうしないと後でvars_を送る際にprev変数達を送れない（時刻0のPPのみでの話）
        // TODO:要検討
        constraint_store_.second.insert(boost::make_tuple(md_it->first,
                                                          i,
                                                          true));

        constraint_store_.second.insert(boost::make_tuple(md_it->first,
                                                          i,
                                                          false));
        first_element = false;
      }
    }
  }  
  cl_->send_string("})");

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

  // expr_を渡す（collected_tells、appended_asks、constraint_store、left_continuityの4つから成る）
  cl_->send_string("expr_:=append(append(append(");
//  cl_->send_string("expr_:=append(append(append(append(");

  // tell制約の集合からexprを得てREDUCEに渡す
  std::cout << "collected_tells" << std::endl;
//  cl_->send_string("expr_:={df(y,t,2) = -10,");
//  cl_->send_string("y = 10, df(y,t,1) = 0, prev(y) = y, df(prev(y),t,1) = df(y,t,1)};");
  cl_->send_string("{");
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    if(tells_it != collected_tells.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", *(*tells_it)->get_child());
    rss.put_node((*tells_it)->get_child());
  }
  cl_->send_string("},");

  // appended_asksからガード部分を得てREDUCEに渡す
  std::cout << "appended_asks" << std::endl;
  cl_->send_string("{");
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    if(append_it != appended_asks.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()), "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_child());
  }
  cl_->send_string("}),");

  // 制約ストアからも渡す
  send_cs();
  cl_->send_string("),");

  // 左連続性に関する制約を渡す
  // 現在採用している制約に出現する変数の最大微分回数よりも小さい微分回数のものについてprev(x)=x追加
  max_diff_map_t max_diff_map;
  create_max_diff_map(rss, max_diff_map);
  add_left_continuity_constraint(rss, max_diff_map);
  cl_->send_string(");");
//  cl_->send_string("),{prev(df(v,t))=1, prev(v)=2});");


  // pexprを渡す
  cl_->send_string("pexpr_:=");
  send_ps();
  cl_->send_string(";");


  // varsを渡す
  //   ex) {y, prev(y), df(y,t,1), df(prev(y),t,1), df(y,t,2), y, prev(y), df(y,t,1), df(prev(y),t,1)}
  // vars_に関して一番外側の"{}"部分は、put_vars内で送っている
  cl_->send_string("vars_:=");
//  cl_->send_string("vars_:=append(");
  rss.put_vars();
  cl_->send_string(";");
//  cl_->send_string(",{prev(df(v,t)), prev(v)});");


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
    result = VCSR_TRUE;
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

  std::cout << "guard" << std::endl;
  // ask制約のガードの式を得てMathematicaに渡す
  cl_->send_string("guard_:=");
  rss.put_node(negative_ask->get_guard());
  cl_->send_string(";");  


  // 制約ストアとパラメタストアから式を得てMathematicaに渡す
  std::cout << "constraint store and parameter store" << std::endl;
  cl_->send_string("store_:=append(");
  send_cs();
  cl_->send_string(",");
  send_ps();
  cl_->send_string(");");


  // varsを渡す
  std::cout << "vars" << std::endl;
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

  VCSResult result;
  
  // S式パーサで読み取る
  sp_.parse_main(ans.c_str());

  // {コード}の構造
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
    result = VCSR_TRUE;
  }
  else if(ret_code_str==" \"RETTRUE___\"") {
    result = VCSR_TRUE;
  }
    






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
  assert(or_cons_size==1);

  for(size_t i=0; i<or_cons_size; i++){
    const_tree_iter_t or_cons_it = constraint_store_.first->children.begin()+i;

/*
    size_t and_cons_size = or_cons_it->children.size();
    HYDLA_LOGGER_VCS("and cons size: ", and_cons_size);
    for(size_t j=0; j<and_cons_size; j++){
      const_tree_iter_t and_cons_it = or_cons_it->children.begin()+j;
      std::string relop = std::string(and_cons_it->value.begin(), and_cons_it->value.end());
      std::cout << "relop: " << relop << "\n";
      size_t var_info_size = and_cons_it->children.size();
      std::cout << "var info size: " << var_info_size << "\n";
    }
*/

    std::string or_string = sp_.get_string_from_tree(or_cons_it);
    std::cout << "or_string: " << or_string << "\n";
    cl_->send_string(or_string);
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

void REDUCEVCSPoint::send_cs_vars() const
{
  int vars_size = constraint_store_.second.size();


  HYDLA_LOGGER_VCS(
    "---- Send Constraint Store Vars -----\n",
    "vars_size: ", vars_size);


  REDUCEStringSender rss(*cl_);

  cl_->send_string("{");

  constraint_store_vars_t::const_iterator it =
    constraint_store_.second.begin();
  constraint_store_vars_t::const_iterator end =
    constraint_store_.second.end();
  for(; it!=end; ++it) {
    if(it!=constraint_store_.second.begin())   cl_->send_string(",");
    rss.put_var(*it);
  }
  cl_->send_string("}");

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

