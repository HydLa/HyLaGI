
#include <cassert>

#include "REDUCEVCSPoint.h"
#include "REDUCEStringSender.h"
#include "Logger.h"
#include "SExpConverter.h"

using namespace hydla::vcs;
using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace reduce {

REDUCEVCSPoint::REDUCEVCSPoint(REDUCELink* cl) :
  cl_(cl)
{

}

REDUCEVCSPoint::~REDUCEVCSPoint()
{

}

bool REDUCEVCSPoint::reset()
{
  // TODO: チョイ考える
  assert(0);
  //   constraint_store_.first.clear();
  //   constraint_store_.second.clear();
  return true;
}

bool REDUCEVCSPoint::reset(const variable_map_t& variable_map)
{

  HYDLA_LOGGER_VCS_SUMMARY("#*** Reset Constraint Store ***");

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", variable_map);

  SExpConverter sc;

  // 変数表の中身を表すようなS式の文字列vm_s_exp_strを得たい
  // まずは、REDUCEへ送る文字列vm_strを作成する
  std::ostringstream vm_str;
  vm_str << "{";

  variable_map_t::variable_list_t::const_iterator it =
    variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end =
    variable_map.end();
  bool first_element = true;
  for(; it!=end; ++it)
  {
    const REDUCEVariable& variable = (*it).first;
    const value_t&    value = it->second;

    if(!value.is_undefined()) {
      if(!first_element) vm_str << ", ";

      vm_str << "equal(";

      // variable部分
      if(variable.derivative_count > 0)
      {
        vm_str << "prev(df("
               << variable.name
               << ", t, "
               << variable.derivative_count
               << "))";
      }
      else
      {
        vm_str << "prev("
               << variable.name
               << ")";
      }

      // value部分
      vm_str << ", "
             << sc.convert_symbolic_value_to_reduce_string(value)
             << ")"; // equalの閉じ括弧


      // 制約ストア内の変数一覧にvariableを追加
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
      first_element = false;
    }
  }

  vm_str << "}"; // listの閉じ括弧
  HYDLA_LOGGER_VCS("vm_str: ", vm_str.str());  

  cl_->send_string("str_:=");
  cl_->send_string(vm_str.str());
  cl_->send_string(";");
  cl_->send_string("symbolic redeval '(getSExpFromString str_);");


  cl_->read_until_redeval();
//  cl_->skip_until_redeval();

  std::string vm_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("vm_s_exp_str: ", vm_s_exp_str);
  
  // sp_にvm_s_exp_strを読み込ませて、S式のパースツリーを構築し、先頭のポインタを得る
  sp_.parse_main(vm_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();
  size_t and_cons_size = tree_root_ptr->children.size();

  std::set<REDUCEValue> and_cons_set;
  for(size_t i=0; i<and_cons_size; i++){
    const_tree_iter_t and_cons_ptr = tree_root_ptr->children.begin()+i;
    std::string and_cons_str = sp_.get_string_from_tree(and_cons_ptr);

    REDUCEValue new_reduce_value;
    new_reduce_value.set(and_cons_str);
    and_cons_set.insert(new_reduce_value);
  }
  constraint_store_.first.insert(and_cons_set);

  HYDLA_LOGGER_VCS(*this);

  return true;
}

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


  std::set<REDUCEValue> and_cons_set;
  par_names_.clear();

  parameter_map_t::variable_list_t::const_iterator it =
    parameter_map.begin();
  parameter_map_t::variable_list_t::const_iterator end =
    parameter_map.end();

/*
  for(; it!=end; ++it)
  {
    const value_range_t&    value = it->second;
    if(!value.is_undefined()) {
      value_range_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
      for(;or_it != or_end; or_it++){
        value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
        for(; and_it != and_end; and_it++){
          std::ostringstream val_str;

          // REDUCEVariable側に関する文字列を作成
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
*/

//  parameter_store_.first.insert(and_cons_set);
  return true;
}

bool REDUCEVCSPoint::create_maps(create_result_t & create_result)
{

  // TODO: 不等式及び記号定数への対応

  HYDLA_LOGGER_VCS(
    "#*** REDUCEVCSPoint::create_variable_map ***\n",
    "--- constraint_store ---\n",
    *this);

  size_t or_size = constraint_store_.first.size();
  HYDLA_LOGGER_VCS("or_size: ", or_size);
  // TODO: 複数解に対応
  assert(or_size==1);

  // S式パーサを用いて、制約ストア全体を表すような木構造を再び得る
  cl_->send_string("str_:=");
  send_cs();
  cl_->send_string(";");
  cl_->send_string("symbolic redeval '(getSExpFromString str_);");


  cl_->read_until_redeval();
//  cl_->skip_until_redeval();

  std::string cs_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("cs_s_exp_str: ", cs_s_exp_str);
  sp_.parse_main(cs_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();


  // TODO:以下のコードはor_size==1が前提
  {
    create_result_t::maps_t maps;
    variable_t symbolic_variable;
    value_t symbolic_value;
    size_t and_cons_size = tree_root_ptr->children.size();

    for(size_t i=0; i<and_cons_size; i++){
      const_tree_iter_t and_ptr = tree_root_ptr->children.begin()+i;

      std::string and_cons_string =  sp_.get_string_from_tree(and_ptr);
      HYDLA_LOGGER_VCS("and_cons_string: ", and_cons_string);


      // 変数側
      const_tree_iter_t var_ptr = and_ptr->children.begin();
      std::string var_head_str = std::string(var_ptr->value.begin(),var_ptr->value.end());

      // prev変数は処理しない
      if(var_head_str=="prev") continue;

      std::string var_name;
      int var_derivative_count = sp_.get_derivative_count(var_ptr);

      // 微分を含む変数
      if(var_derivative_count > 0){
          var_name = std::string(var_ptr->children.begin()->value.begin(), 
                                 var_ptr->children.begin()->value.end());
      }
      // 微分を含まない変数
      else{
        assert(var_derivative_count == 0);
        var_name = var_head_str;
      }
      
      // 変数名の先頭にスペースが入ることがあるので除去する
      // TODO:S式パーサを修正してスペース入らないようにする
      if(var_name.at(0) == ' ') var_name.erase(0,1);

      symbolic_variable.name = var_name;
      symbolic_variable.derivative_count = var_derivative_count;

      // 値側
      const_tree_iter_t value_ptr = and_ptr->children.begin()+1;
      SExpConverter sc;
      symbolic_value = sc.convert_s_exp_to_symbolic_value(sp_, value_ptr);
      symbolic_value.set_unique(true);
      
      maps.variable_map.set_variable(symbolic_variable, symbolic_value);
    }

    HYDLA_LOGGER_VCS_SUMMARY(maps.variable_map);
    HYDLA_LOGGER_VCS_SUMMARY(maps.parameter_map);
    create_result.result_maps.push_back(maps);
  }

  return true;
}

void REDUCEVCSPoint::set_continuity(const continuity_map_t& continuity_map)
{
  continuity_map_ = continuity_map;
}

void REDUCEVCSPoint::add_left_continuity_constraint(
    REDUCEStringSender& rss, max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSPoint::add_left_continuity_constraint ----");


  cl_->send_string("union({");
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

void REDUCEVCSPoint::add_constraint(const constraints_t& constraints)
{

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::add_constraint ***");

  SExpConverter sc;

/*
  // TODO: いちいち送るのも非効率なので、文字列化してストアに入れることを検討する？
  std::stringstream added_constraints_str;
  added_constraints_str << "{";
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it)
  {
    if(it != constraints.begin()) added_constraints_str << ",";
    added_constraints_str << sc.convert_node_to_reduce_string(*it);
  }
  added_constraints_str << "}";
  HYDLA_LOGGER_VCS("added_constraints_str: ", added_constraints_str.str());  

  cl_->send_string("str_:=union(");
  send_cs();
  cl_->send_string(",");
  cl_->send_string(added_constraints_str.str());
  cl_->send_string(");");
  cl_->send_string("symbolic redeval '(getSExpFromString str_);");


  cl_->read_until_redeval();
//  cl_->skip_until_redeval();

  std::string new_cs_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("new_cs_s_exp_str: ", new_cs_s_exp_str);

  // sp_にnew_cs_s_exp_strを読み込ませて、S式のパースツリーを構築し、先頭のポインタを得る
  // TODO: Orへの対応？
  sp_.parse_main(new_cs_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();

  for(int i=0; i<tree_root_ptr->children.size(); i++){
    const_tree_iter_t and_cons_ptr = tree_root_ptr->children.begin()+i;
    std::string and_cons_str = sp_.get_string_from_tree(and_cons_ptr);

    REDUCEValue new_reduce_value;
    new_reduce_value.set(and_cons_str);
    and_cons_set.insert(new_reduce_value);
  }
*/

  // TODO: Orへの対応？
  std::set<REDUCEValue> and_cons_set = *(constraint_store_.first.begin());
  constraint_store_.first.erase(constraint_store_.first.begin());

  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it)
  {
    REDUCEValue new_reduce_value;
    new_reduce_value.set(sc.convert_symbolic_value_to_reduce_string(*it));
    and_cons_set.insert(new_reduce_value);
  }

  constraint_store_.first.insert(and_cons_set);
  constraint_store_.second.insert(sc.vars_begin(), sc.vars_end());

  sc.create_max_diff_map(max_diff_map_);

  HYDLA_LOGGER_VCS(
    *this,
    "\n#*** End REDUCEVCSPoint::add_constraint ***");
  return;
}

VCSResult REDUCEVCSPoint::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t &appended_asks)
{

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_entailment ***",
                   "ask: ", *negative_ask);
  REDUCEStringSender rss = REDUCEStringSender(*cl_);


/////////////////// 送信処理

  // checkentailment(guard_, store_, vars_)を渡したい

  HYDLA_LOGGER_VCS("----- send guard_ -----");
  // ask制約のガードの式を得てMathematicaに渡す
  cl_->send_string("guard_:=");
  rss.put_node(negative_ask->get_guard());
  cl_->send_string(";");  


  // 制約ストアとパラメタストアから式を得てMathematicaに渡す
  HYDLA_LOGGER_VCS("----- send store_ -----");
  cl_->send_string("store_:=union(");
  send_cs();
  cl_->send_string(",");
  send_ps();
  cl_->send_string(");");


  // varsを渡す
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=union(union(");

  // collected_tells内の変数一覧を渡す
  rss.put_vars();
  cl_->send_string(",");

  // 制約ストア内に出現する変数一覧も渡す
  send_cs_vars();
  cl_->send_string("),");

  // パラメタ一覧も渡す
  send_pars();
  cl_->send_string(");");


  cl_->send_string("symbolic redeval '(checkentailment guard_ store_ vars_);");


/////////////////// 受信処理
  HYDLA_LOGGER_VCS( "--- receive ---");

//  cl_->read_until_redeval();
  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("check_entailment_ans: ",
                   ans);

  VCSResult result;
  
  // TODO:S式パーサを使う→？
/*  
  // S式パーサで読み取る
  sp_.parse_main(ans.c_str());

  // {コード}の構造
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();

  // コードを取得
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ",
                   ret_code_str);
*/


//  HYDLA_LOGGER_VCS(*this);
//  sp_.dump_tree(*(constraint_store_.first.begin()), 0);
  
  if(ans == "(list ccp_solver_error___)"){
    // ソルバエラー
    result = VCSR_SOLVER_ERROR;
  }
  else if(ans == "ccp_entailed___") {
    result = VCSR_TRUE;
    HYDLA_LOGGER_VCS_SUMMARY("entailed");
  }
  else if(ans == "ccp_not_entailed___") {
    result = VCSR_FALSE;
    HYDLA_LOGGER_VCS_SUMMARY("not entailed");
  }
  else if(ans == "(list ccp_unknown___)"){
    assert(ans == "(list ccp_unknown___)");
    result = VCSR_UNKNOWN;
  }

  return result;
}

VCSResult REDUCEVCSPoint::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_consistency(tmp) ***");
  tmp_constraints_ = constraints;

  VCSResult result = check_consistency_sub();
  switch(result){
    default: assert(0); break;
    case VCSR_TRUE:
      HYDLA_LOGGER_VCS_SUMMARY("consistent");
      break;
    case VCSR_FALSE:
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//矛盾
    case VCSR_SOLVER_ERROR:
      break;
  }
  tmp_constraints_.clear();
  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::check_consistency(tmp) ***");
  HYDLA_LOGGER_VCS(
    *this);
  return result;
}

VCSResult REDUCEVCSPoint::check_consistency()
{
  HYDLA_LOGGER_VCS(
    "#*** Begin REDUCEVCSPoint::check_consistency() ***");

  VCSResult result = check_consistency_sub();

  switch(result){
    default: assert(0); break;
    case VCSR_TRUE:
    {
      HYDLA_LOGGER_VCS_SUMMARY("consistent");
      // 制約ストアをリセット
      //    reset();
      constraint_store_.first.clear();

      receive_constraint_store(constraint_store_);
    }
    break;
    case VCSR_FALSE:
    {
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//矛盾
      // 制約ストアをリセット
      //    reset();
      constraint_store_.first.clear();
    }
    break;
    case VCSR_SOLVER_ERROR:
      break;
  }
  HYDLA_LOGGER_VCS(
    *this,
    "\n#*** End REDUCEVCSPoint::check_consistency() ***");
  return result;
}

void REDUCEVCSPoint::receive_constraint_store(constraint_store_t& constraint_store){

  HYDLA_LOGGER_VCS( "--- REDUCEVCSPoint::receive constraint store---");
  // 制約ストア構築
  // 制約間のOrに関してはsetで保持
  // TODO:出来ればvectorあたりで扱いたいがその場合resetあたりでもう少し適切な処理が必要か
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  const_tree_iter_t or_list_ptr = ret_code_ptr+1;
  size_t or_size = or_list_ptr->children.size();
  HYDLA_LOGGER_VCS( "or_size: ", or_size);

  for(size_t i=0; i<or_size; i++)
  {
    const_tree_iter_t and_list_ptr = or_list_ptr->children.begin()+i;
    size_t and_size = and_list_ptr->children.size();

    std::set<REDUCEValue> and_cons_set;
    for(size_t j=0; j<and_size; j++){
      const_tree_iter_t and_cons_ptr = and_list_ptr->children.begin()+j;
      std::string and_cons_str = sp_.get_string_from_tree(and_cons_ptr);
      
      REDUCEValue new_reduce_value;
      new_reduce_value.set(and_cons_str);
      and_cons_set.insert(new_reduce_value);
    }
    constraint_store.first.insert(and_cons_set);
  }
}

VCSResult REDUCEVCSPoint::check_consistency_sub()
{

  REDUCEStringSender rss = REDUCEStringSender(*cl_);


//////////////////// 送信処理

  // send_stringのstringはどのように区切って送信してもOK
  //   ex) cl_->send_string("expr_:={df(y,t,2) = -10,");
  //       cl_->send_string("y = 10, df(y,t,1) = 0, prev(y) = y, df(prev(y),t,1) = df(y,t,1)};");

  // isConsistent(expr_,vars_)を渡したい

  // expr_を渡す（tmp_constraints、constraint_store、parameter_store、left_continuityの4つから成る）
  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=union(union(union(");

  // 一時的な制約ストア(新しく追加された制約の集合)からexprを得てREDUCEに渡す
  HYDLA_LOGGER_VCS("--- send tmp_constraints ---");
  cl_->send_string("{");
  constraints_t::const_iterator tmp_it  = tmp_constraints_.begin();
  constraints_t::const_iterator tmp_end = tmp_constraints_.end();
  for(; tmp_it!= tmp_end; ++tmp_it) {
    if(tmp_it != tmp_constraints_.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", (**tmp_it));
    rss.put_node(*tmp_it);
  }
  cl_->send_string("},");

  // 制約ストアからも渡す
  HYDLA_LOGGER_VCS("--- send constraint_store ---");
  send_cs();
  cl_->send_string("),");

  // パラメタストアを渡す
  HYDLA_LOGGER_VCS("--- send parameter_store ---");
  send_ps();
  cl_->send_string("),");

  // 左連続性に関する制約を渡す
  // 現在採用している制約に出現する変数の最大微分回数よりも小さい微分回数のものについてprev(x)=x追加
  HYDLA_LOGGER_VCS("--- send left_continuity ---");
  max_diff_map_t tmp_max_diff_map = max_diff_map_;
  rss.create_max_diff_map(tmp_max_diff_map);
  add_left_continuity_constraint(rss, tmp_max_diff_map);
  cl_->send_string(");");


  // varsを渡す
  //   ex) {y, prev(y), df(y,t,1), df(prev(y),t,1), df(y,t,2), y, prev(y), df(y,t,1), df(prev(y),t,1)}
  // vars_に関して一番外側の"{}"部分は、put_vars内で送っている
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=union(");
  rss.put_vars();
  cl_->send_string(",");
  // 制約ストア内に出現する変数も渡す
  send_cs_vars();
  cl_->send_string(");");


  cl_->send_string("symbolic redeval '(isconsistent expr_ vars_);");


/////////////////// 受信処理
  HYDLA_LOGGER_VCS("--- receive ---");

  cl_->read_until_redeval();
//  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ",
                   ans);

  VCSResult result;

  // S式パーサで読み取る
  sp_.parse_main(ans.c_str());

  // {コード, {{{変数, 関係演算子コード, 値},...}, ...}}の構造
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();

  // コードを取得
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
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
  }
  else {
    assert(ret_code_str == " \"RETFALSE___\"");
    result = VCSR_FALSE;
  }

  return result;
  
}

VCSResult REDUCEVCSPoint::integrate(
    integrate_result_t& integrate_result,
    const constraints_t &constraints,
    const time_t& current_time,
    const time_t& max_time)
{
  // Pointではintegrate関数無効
  assert(0);
  return VCSR_FALSE;
}

void REDUCEVCSPoint::send_cs() const
{


  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSPoint::send_cs ----");
  HYDLA_LOGGER_VCS("---- Send Constraint Store -----");

  size_t or_cons_size = constraint_store_.first.size();
  HYDLA_LOGGER_VCS("or cons size: ", or_cons_size);

  if(or_cons_size <= 0)
  {
    HYDLA_LOGGER_VCS("no Constraints");
    cl_->send_string("{}");
    return;
  }

  // TODO: 複数解（or_cons_size>1）の場合の対処を考える
  assert(or_cons_size==1);

  std::set<std::set<REDUCEValue> >::const_iterator or_cons_it = constraint_store_.first.begin();
  std::set<std::set<REDUCEValue> >::const_iterator or_cons_end = constraint_store_.first.end();
  for(; or_cons_it!=or_cons_end; ++or_cons_it){
    int and_cons_size = or_cons_it->size();
    HYDLA_LOGGER_VCS("and cons size: ", and_cons_size);

    cl_->send_string("{");
    std::set<REDUCEValue>::const_iterator and_cons_it = or_cons_it->begin();
    std::set<REDUCEValue>::const_iterator and_cons_end = or_cons_it->end();
    for(; and_cons_it!=and_cons_end; ++and_cons_it)
    {
      if(and_cons_it!=or_cons_it->begin()) cl_->send_string(",");
      std::string str = and_cons_it->get_string();
      HYDLA_LOGGER_VCS("put cons: ", str);
      cl_->send_string(str);
    }
    cl_->send_string("}");
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
  // TODO: ちゃんと送る
  cl_->send_string("{}");
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
    if(it!=constraint_store_.second.begin()) cl_->send_string(",");
    rss.put_var(*it);
  }
  cl_->send_string("}");

}

std::ostream& REDUCEVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSPoint ***\n"
      << "--- constraint store ---\n";

  std::set<std::set<REDUCEValue> >::const_iterator or_cons_it = constraint_store_.first.begin();
  while(or_cons_it != constraint_store_.first.end())
  {
    std::set<REDUCEValue>::const_iterator and_cons_it = or_cons_it->begin();
    while(and_cons_it != or_cons_it->end())
    {
      s << and_cons_it->get_string() << " ";
      and_cons_it++;
    }
    s << "\n";
    or_cons_it++;
  }

  // 制約ストア内に存在する変数のダンプ
  s << "-- vars --\n";
  constraint_store_vars_t::const_iterator vars_it = constraint_store_.second.begin();
  while((vars_it) != constraint_store_.second.end())
  {
    s << *(vars_it) << "\n";
    vars_it++;
  }

  return s;
}

std::ostream& operator<<(std::ostream& s, const REDUCEVCSPoint& r)
{
  return r.dump(s);
}


} // namespace reduce
} // namespace simulator
} // namespace hydla

