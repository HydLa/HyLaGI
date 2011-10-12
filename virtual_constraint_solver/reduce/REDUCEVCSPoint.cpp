
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
  
  cl_->send_string("symbolic redeval '(resetConstraintStore);");
  //  cl_->read_until_redeval();
  cl_->skip_until_redeval();

  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", variable_map);

  SExpConverter sc;

  // まずは、REDUCEへ送る文字列vm_strとvars_strを作成する
  std::ostringstream vm_str;
  std::ostringstream vars_str;
  vm_str << "{";
  vars_str << "{";

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
      if(!first_element){
        vm_str << ", ";
        vars_str << ",";
      }

      vm_str << "equal(";

      // variable部分
      if(variable.derivative_count > 0)
      {
        vm_str << "prev(df("
               << variable.name
               << ", t, "
               << variable.derivative_count
               << "))";
        vars_str << "prev(df("
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
        vars_str << "prev("
                 << variable.name
                 << ")";
      }

      // value部分
      vm_str << ", "
             << sc.convert_symbolic_value_to_reduce_string(value)
             << ")"; // equalの閉じ括弧

      first_element = false;
    }
  }

  vm_str << "}"; // listの閉じ括弧
  vars_str << "}"; // listの閉じ括弧
  HYDLA_LOGGER_VCS("vm_str: ", vm_str.str());  
  HYDLA_LOGGER_VCS("vars_str: ", vars_str.str());  


  cl_->send_string("vm_str_:=");
  cl_->send_string(vm_str.str());
  cl_->send_string(";");
  cl_->send_string("vars_str_:=");
  cl_->send_string(vars_str.str());
  cl_->send_string(";");
  cl_->send_string("symbolic redeval '(addConstraint vm_str_ vars_str_);");


  //  cl_->read_until_redeval();
  cl_->skip_until_redeval();

/*
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
*/

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

  HYDLA_LOGGER_VCS("#*** REDUCEVCSPoint::create_variable_map ***\n");

  REDUCEStringSender rss(*cl_);

  /////////////////// 送信処理

  // expr_を渡す
  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=");
  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string(";");

  // vars_を渡す
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  rss.put_vars();
  cl_->send_string(";");


  cl_->send_string("symbolic redeval '(addConstraint expr_ vars_);");
  // cl_->read_until_redeval();
  cl_->skip_until_redeval();


  cl_->send_string("symbolic redeval '(checkConsistency);");
  // cl_->read_until_redeval();
  cl_->skip_until_redeval();


  // TODO:本来はexprCode付きの形式で返す関数を使うようにしたい
  //  cl_->send_string("symbolic redeval '(convertCSToVM);");
  cl_->send_string("symbolic redeval '(returnCS);");


  /////////////////// 受信処理                     

  //  cl_->read_until_redeval();
  cl_->skip_until_redeval();

  // S式パーサを用いて、制約ストア全体を表すような木構造を得る
  std::string cs_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("cs_s_exp_str: ", cs_s_exp_str);
  sp_.parse_main(cs_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();


  // TODO:以下のコードはor_size==1が前提
  // TODO:不等式への対応(exprCodeを使う)
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
    const continuity_map_t& continuity_map, REDUCEStringSender& rss)
{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSPoint::add_left_continuity_constraint ----");

  cl_->send_string("{");
  continuity_map_t::const_iterator cm_it = continuity_map.begin();
  continuity_map_t::const_iterator cm_end = continuity_map.end();
  bool first_element = true;
  for(; cm_it!=cm_end; ++cm_it) {
    for(int i=0; i < abs(cm_it->second); ++i){
      if(!first_element) cl_->send_string(",");

      // Prev変数側
      // 変数名
      rss.put_var(boost::make_tuple(cm_it->first, i, true));

      cl_->send_string("=");

      // Now変数側
      // 変数名
      rss.put_var(boost::make_tuple(cm_it->first, i, false));

      first_element = false;
    }
  }
  cl_->send_string("}");  

}

void REDUCEVCSPoint::add_constraint(const constraints_t& constraints)
{

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::add_constraint ***");

  //  SExpConverter sc;

  REDUCEStringSender rss(*cl_);

  // cons_を渡す
  HYDLA_LOGGER_VCS("----- send cons_ -----");
  cl_->send_string("cons_:={");
  bool first_element = true;
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it)
    {
      if(!first_element) cl_->send_string(",");
      rss.put_node(*it);
      first_element = false;
    }
  cl_->send_string("};");

  // vars_を渡す
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  rss.put_vars();
  cl_->send_string(";");

  
  cl_->send_string("symbolic redeval '(addConstraint cons_ vars_);");

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();


  //  sc.create_max_diff_map(max_diff_map_);

  HYDLA_LOGGER_VCS("\n#*** End REDUCEVCSPoint::add_constraint ***");
  return;
}

void REDUCEVCSPoint::send_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS(
                   "#*** Begin REDUCEVCSPoint::send_constraint ***");

  REDUCEStringSender rss(*cl_);

  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=union({");
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  bool first_element = true;
  for(; it!=end; ++it)
    {
      if(!first_element) cl_->send_string(",");
      rss.put_node(*it);
      first_element = false;
    }
  cl_->send_string("},");

  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string(");");


  // varsを渡す
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  rss.put_vars();
  cl_->send_string(";");

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::send_constraint ***");
  continuity_map_t continuity_map;
  rss.create_max_diff_map(continuity_map);
  return;
}

VCSResult REDUCEVCSPoint::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_consistency(tmp) ***");

  send_constraint(constraints);

  cl_->send_string("symbolic redeval '(checkConsistencyWithTmpCons expr_ vars_);");

  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::check_consistency(tmp) ***");
  return result;
}

VCSResult REDUCEVCSPoint::check_consistency()
{
  HYDLA_LOGGER_VCS(
    "#*** Begin REDUCEVCSPoint::check_consistency() ***");

  send_constraint(constraints_t());

  cl_->send_string("symbolic redeval '(checkConsistencyWithTmpCons expr_ vars_);");

  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("\n#*** End REDUCEVCSPoint::check_consistency() ***");
  return result;
}

VCSResult REDUCEVCSPoint::check_consistency_receive()
{
  /////////////////// 受信処理
  HYDLA_LOGGER_VCS( "--- receive ---");

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

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
  else if(ret_code_str=="1") {
    // 充足
    // TODO: スペースや""が残らないようにパーサを修正
    result = VCSR_TRUE;
  }
  else {
    assert(ret_code_str == "2");
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

} // namespace reduce
} // namespace simulator
} // namespace hydla

