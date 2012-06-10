
#include <cassert>

#include "REDUCEVCSPoint.h"
#include "REDUCEStringSender.h"
#include "Logger.h"
#include "SExpConverter.h"
#include "../SolveError.h"

#include "VariableNameEncoder.h"

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

bool REDUCEVCSPoint::create_maps(create_result_t & create_result)
{

  // TODO: 不等式及び記号定数への対応

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::create_variable_map ***");

  REDUCEStringSender rss(*cl_);

  /////////////////// 送信処理

  // expr_を渡す
  HYDLA_LOGGER_VCS("--- send expr_ ---");
  cl_->send_string("expr_:=");
  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string("$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_->send_string("vars_:=union(");
  rss.put_vars();
  cl_->send_string(")$");


  cl_->send_string("symbolic redeval '(addConstraint expr_ vars_);");
  // cl_->read_until_redeval();
  cl_->skip_until_redeval();


  cl_->send_string("symbolic redeval '(checkConsistency);");
  // cl_->read_until_redeval();
  cl_->skip_until_redeval();


  cl_->send_string("symbolic redeval '(convertCSToVM);");


  /////////////////// 受信処理                     

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

  // S式パーサを用いて、制約ストア全体を表すような木構造を得る
  std::string cs_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("cs_s_exp_str: ", cs_s_exp_str);
  sp_.parse_main(cs_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();


  // TODO:以下のコードはor_size==1が前提
  //  for(int or_it = 0; or_it < or_size; or_it++){
  {
    create_result_t::maps_t maps;
    std::set<variable_t> p_added_variables;  //「今回記号定数を追加された変数」の一覧
    variable_t symbolic_variable;
    value_t symbolic_value;
    value_range_t tmp_range;
    SExpConverter::clear_parameter_map();
    size_t and_cons_size = tree_root_ptr->children.size();

    for(size_t i=0; i<and_cons_size; i++){
      const_tree_iter_t and_ptr = tree_root_ptr->children.begin()+i;

      std::string and_cons_string =  sp_.get_string_from_tree(and_ptr);
      HYDLA_LOGGER_VCS("and_cons_string: ", and_cons_string);


      // 変数名
      const_tree_iter_t var_ptr = and_ptr->children.begin();
      std::string var_head_str = std::string(var_ptr->value.begin(),var_ptr->value.end());

      // 関係演算子のコード
      const_tree_iter_t relop_code_ptr = and_ptr->children.begin()+1;      
      std::string relop_code_str = std::string(relop_code_ptr->value.begin(),relop_code_ptr->value.end());
      std::stringstream relop_code_ss;
      int relop_code;
      relop_code_ss << relop_code_str;
      relop_code_ss >> relop_code;
      assert(relop_code>=0 && relop_code<=4);

      // 値
      const_tree_iter_t value_ptr = and_ptr->children.begin()+2;


      // prevの先頭にスペースが入ることがあるので除去する
      // TODO:S式パーサを修正してスペース入らないようにする
      if(var_head_str.at(0) == ' ') var_head_str.erase(0,1);

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

      // 既存の記号定数の場合
      if(var_name.find(REDUCEStringSender::var_prefix, 0) != 0){
	// 'p'は取っておく必要がある
	assert(var_name.at(0) == REDUCEStringSender::par_prefix.at(0));
	var_name.erase(0, 1);
        parameter_t tmp_param(parameter_t::get_variable(var_name));
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        value_t tmp_value = SExpConverter::convert_s_exp_to_symbolic_value(sp_, value_ptr);
        SExpConverter::set_range(tmp_value, tmp_range, relop_code);
        maps.parameter_map.set_variable(tmp_param, tmp_range);
        continue;
      }

      // "usrVar"を取り除く
      assert(var_name.find(REDUCEStringSender::var_prefix, 0) == 0);
      var_name.erase(0, REDUCEStringSender::var_prefix.length());

      // 大文字小文字表記に変換
      VariableNameEncoder vne;
      var_name = vne.UpperDecode(var_name);

      symbolic_variable.name = var_name;
      symbolic_variable.derivative_count = var_derivative_count;



      // 関係演算子コードを元に、変数表の対応する部分に代入する
      if(!relop_code){
        //等号
        symbolic_value = SExpConverter::convert_s_exp_to_symbolic_value(sp_, value_ptr);
        symbolic_value.set_unique(true);
      }else{
        //不等号．この変数の値の範囲を表現するための記号定数を作成
        value_t tmp_value = SExpConverter::convert_s_exp_to_symbolic_value(sp_, value_ptr);
	if(p_added_variables.find(symbolic_variable) == p_added_variables.end()){
	  //今回追加された記号定数に含まれない場合のみ，新規作成
	  parameter_t::increment_id(symbolic_variable);
	  p_added_variables.insert(symbolic_variable);
	}
	parameter_t tmp_param(symbolic_variable);
	tmp_range = maps.parameter_map.get_variable(tmp_param);
	SExpConverter::set_range(tmp_value, tmp_range, relop_code);
	SExpConverter::add_parameter(symbolic_variable, tmp_param);
	maps.parameter_map.set_variable(tmp_param, tmp_range);
	symbolic_value.set_unique(false);
	SExpConverter::set_parameter_on_value(symbolic_value, tmp_param);
      }
      maps.variable_map.set_variable(symbolic_variable, symbolic_value);
    }
    HYDLA_LOGGER_VCS("variable_map: ", maps.variable_map);
    HYDLA_LOGGER_VCS("parameter_map: ", maps.parameter_map);
    create_result.result_maps.push_back(maps);
  }
  SExpConverter::clear_parameter_map();

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::create_maps ***");
  return true;
}

void REDUCEVCSPoint::set_continuity(const continuity_map_t& continuity_map)
{
  continuity_map_ = continuity_map;
}

void REDUCEVCSPoint::add_left_continuity_constraint(
    const continuity_map_t& continuity_map, REDUCEStringSender& rss)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::add_left_continuity_constraint ***");

  cl_->send_string("{");
  continuity_map_t::const_iterator cm_it = continuity_map.begin();
  continuity_map_t::const_iterator cm_end = continuity_map.end();
  bool first_element = true;
  for(; cm_it!=cm_end; ++cm_it) {
    for(int i=0; i < abs(cm_it->second); ++i){
      if(!first_element) cl_->send_string(",");

      // Prev変数側
      // 変数名
      rss.put_var(boost::make_tuple(cm_it->first, i, true, false));

      cl_->send_string("=");

      // Now変数側
      // 変数名
      rss.put_var(boost::make_tuple(cm_it->first, i, false, false));

      first_element = false;
    }
  }
  cl_->send_string("}");  

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::add_left_continuity_constraint ***");

}

void REDUCEVCSPoint::send_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::send_constraint ***");

  REDUCEStringSender rss(*cl_);

  // 制約を渡す
  HYDLA_LOGGER_VCS("--- send expr_ ---");
  cl_->send_string("expr_:={");
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it)
    {
      if(it!=constraints.begin()) cl_->send_string(",");
      rss.put_node(*it);
    }
  cl_->send_string("}$");


  // 左連続性制約を渡す
  HYDLA_LOGGER_VCS("--- send lcont_ ---");
  cl_->send_string("lcont_:=");
  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string("$");


  // varsを渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_->send_string("vars_:=union(");
  rss.put_vars();
  cl_->send_string(")$");

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::send_constraint ***");
  return;
}

VCSResult REDUCEVCSPoint::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_consistency(tmp) ***");

  send_constraint(constraints);

  cl_->send_string("symbolic redeval '(checkConsistencyWithTmpCons expr_ lcont_ vars_);");

  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::check_consistency(tmp) ***");
  return result;
}

VCSResult REDUCEVCSPoint::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_consistency() ***");

  send_constraint(constraints_t());

  cl_->send_string("symbolic redeval '(checkConsistencyWithTmpCons expr_ lcont_ vars_);");

  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::check_consistency() ***");
  return result;
}

VCSResult REDUCEVCSPoint::check_entailment(const node_sptr &node)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_entailment() ***");

  constraints_t constraints;
  constraints.push_back(node);
  send_constraint(constraints);
  cl_->send_string("symbolic redeval '(checkConsistencyWithTmpCons expr_ lcont_ vars_);");

  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::check_entailment() ***");
  return result;
}

VCSResult REDUCEVCSPoint::check_consistency_receive()
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::check_consistency_receive ***");
  /////////////////// 受信処理
  HYDLA_LOGGER_VCS( "--- receive ---");

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ", ans);

  VCSResult result;

  // S式パーサで読み取る
  sp_.parse_main(ans.c_str());

  // {コード, {{{変数, 関係演算子コード, 値},...}, ...}}の構造
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();

  // コードを取得
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ", ret_code_str);

  if(ret_code_str=="RETERROR___"){
    // ソルバエラー
    throw hydla::vcs::SolveError(ans);
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

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::check_consistency_receive ***");

  return result;  
}

} // namespace reduce
} // namespace simulator
} // namespace hydla

