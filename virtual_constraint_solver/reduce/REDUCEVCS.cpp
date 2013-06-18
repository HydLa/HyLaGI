#include "REDUCEVCS.h"

#include "../../parser/SExpParser.h"
#include "../../simulator/Dumpers.h"
#include "../SolveError.h"
#include "Logger.h"
#include "REDUCEStringSender.h"
#include "SExpConverter.h"
#include "VariableNameEncoder.h"
#include <cassert>


using namespace hydla::parse_tree;
using namespace hydla::parser;
using namespace hydla::vcs;

namespace hydla {
namespace vcs {
namespace reduce {

void REDUCEVCS::change_mode(hydla::simulator::symbolic::Mode m, int approx_precision)
{
  mode_ = m;
}


REDUCEVCS::REDUCEVCS(const hydla::simulator::Opts &opts, variable_range_map_t &m)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  // デバッグプリントの設定
  std::stringstream debug_print_opt_str;
  debug_print_opt_str << "optUseDebugPrint_:=";
  debug_print_opt_str << (opts.debug_mode ? "t" : "nil");
  debug_print_opt_str << ";";
  cl_.send_string((debug_print_opt_str.str()).c_str());

  HYDLA_LOGGER_VCS("--- send depend statements of variables ---");

  std::ostringstream depend_str;
  depend_str << "depend {";
  variable_range_map_t::const_iterator it = m.begin();
  variable_range_map_t::const_iterator end = m.end();
  bool first_element = true;
  for(; it!=end; ++it)
  {
    // 回避 const REDUCEVariable& variable = it->first;
    // 微分回数が0のものだけdepend文を作成
    if(it->first->derivative_count == 0){
      VariableNameEncoder vne;
      if(!first_element) depend_str << ",";
      depend_str << REDUCEStringSender::var_prefix << vne.LowerEncode(it->first->name);
      first_element = false;
    }
  }
  depend_str << "},t;";

  HYDLA_LOGGER_VCS("depend_str: ", depend_str.str()); 
  cl_.send_string((depend_str.str()).c_str());


  // REDUCEの関数定義を送信
  cl_.send_string(vcs_reduce_source());
  // cl_.read_until_redeval();
  cl_.skip_until_redeval();

  SExpConverter::initialize();

  HYDLA_LOGGER_FUNC_END(VCS);
}

REDUCEVCS::~REDUCEVCS()
{}


bool REDUCEVCS::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  cl_.send_string("symbolic redeval '(resetConstraintStore);");
  // cl_.read_until_redeval();
  cl_.skip_until_redeval();

  REDUCEStringSender rss(cl_);
  {
    constraints_t constraints;

    HYDLA_LOGGER_VCS("------Parameter map------\n", parameter_map);
    parameter_map_t::const_iterator it = parameter_map.begin();
    parameter_map_t::const_iterator end = parameter_map.end();
    for(; it!=end; ++it){
      if(it->second.is_unique()){
        const value_t &value = it->second.get_lower_bound().value;
        constraints.push_back(SExpConverter::make_equal(*it->first, get_symbolic_value_t(value).get_node(), true));
      }else{
        const value_range_t& value = it->second;
        parameter_t& param = *it->first;
        if(value.get_lower_bound().value.get() && !value.get_lower_bound().value->undefined()){
          if(value.get_lower_bound().include_bound){
            // TODO 全部lower_boundでいいのか？
            const symbolic_value_t lower_bound = get_symbolic_value_t(value.get_lower_bound().value);
            constraints.push_back(node_sptr(new GreaterEqual(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), lower_bound.get_node())));
          }else{
            const symbolic_value_t lower_bound = get_symbolic_value_t(value.get_lower_bound().value);
            constraints.push_back(node_sptr(new Greater(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), lower_bound.get_node())));
          }
        }
        if(value.get_upper_bound().value.get() && !value.get_upper_bound().value->undefined()){
          if(value.get_upper_bound().include_bound){
            const symbolic_value_t lower_bound = get_symbolic_value_t(value.get_lower_bound().value);
            constraints.push_back(node_sptr(new LessEqual(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), lower_bound.get_node())));
          }else{
            const symbolic_value_t lower_bound = get_symbolic_value_t(value.get_lower_bound().value);
            constraints.push_back(node_sptr(new Less(node_sptr(
                      new Parameter(param.get_name(), param.get_derivative_count(), param.get_phase_id())), lower_bound.get_node())));
          }
        }
      }
    }

    HYDLA_LOGGER_VCS("size:", constraints.size());
    cl_.send_string("pcons_:=");
    rss.put_nodes(constraints);
    cl_.send_string("$");
  }
  cl_.send_string("pars_:=");
  rss.put_pars();
  cl_.send_string("$");

  cl_.send_string("symbolic redeval '(addParameterConstraint pcons_ pars_);");
  cl_.skip_until_redeval();
  
  {
    constraints_t constraints;

    HYDLA_LOGGER_VCS("--- Variable map ---\n", variable_map);
    variable_map_t::const_iterator it  = variable_map.begin();
    variable_map_t::const_iterator end = variable_map.end();
    for(; it!=end; ++it) {
      if(it->second.get() && !it->second->undefined()) {
        constraints.push_back(SExpConverter::make_equal(*it->first, get_symbolic_value_t(it->second).get_node(), true));
      }
    }

    HYDLA_LOGGER_VCS("size:", constraints.size());
    cl_.send_string("cons_:=");
    rss.put_nodes(constraints);
    cl_.send_string("$");
  }
  cl_.send_string("vars_:=");
  // is_unique()だったParameterについても送信する
  rss.put_vars();
  cl_.send_string("$");

  cl_.send_string("symbolic redeval '(addPrevConstraint cons_ vars_);");
  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return true;
}

/**
 * PP時: {prev変数 = 変数名}
 * IP時: {ラプラス変換初期値 = prev変数}, addInitConstraint
 * put_nodeする際ignore_prevをmode_で設定しない唯一のケース(要確認)
 */
void REDUCEVCS::set_continuity(const std::string &name, const int& dc){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::FalseConditionsMode){ assert(0); }

  REDUCEStringSender rss(cl_);

  const node_sptr prev_node(make_variable(name, dc, true));
  const node_sptr var_node(make_variable(name, dc));

  HYDLA_LOGGER_VCS("--- send co_ ---");
  cl_.send_string("co_:={");
  if(mode_==hydla::simulator::symbolic::DiscreteMode){
    rss.put_node(var_node, true);
    cl_.send_string("=");
    rss.put_node(prev_node, false);
  }else{ // case hydla::simulator::symbolic::ContinuousMode:
    rss.put_node(var_node, true, true);
    cl_.send_string("=");
    rss.put_node(prev_node, false);
  }

  cl_.send_string("}$");

  HYDLA_LOGGER_VCS("--- send va_ ---");
  cl_.send_string("va_:={");
  rss.put_node(prev_node, false);
  cl_.send_string(",");
  rss.put_node(var_node, true);
  cl_.send_string("}$");

  cl_.send_string("symbolic redeval '(addInitConstraint co_ va_);");

  cl_.skip_until_redeval();

  HYDLA_LOGGER_VCS("--- add_init_variable ---");
  cl_.send_string("vars_:={");
  rss.put_node(var_node, true, true);
  cl_.send_string("}$");

  cl_.send_string("symbolic redeval '(addInitVariables vars_);");
  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

void REDUCEVCS::add_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  if(mode_==hydla::simulator::symbolic::FalseConditionsMode){ assert(0); }

  REDUCEStringSender rss(cl_);
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::FalseConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");
  cl_.send_string("cons_:={");
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it){
    if(it!=constraints.begin()) cl_.send_string(",");
    rss.put_node(*it, ignore_prev);
  }
  cl_.send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_.send_string("vars_:=");
  rss.put_vars();
  cl_.send_string("$");

  
  cl_.send_string("symbolic redeval '(addConstraint cons_ vars_);");

  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

void REDUCEVCS::add_constraint(const node_sptr& constraint){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::FalseConditionsMode){ assert(0); }

  REDUCEStringSender rss(cl_);
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::FalseConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");
  cl_.send_string("cons_:={");
      rss.put_node(constraint, ignore_prev);
  cl_.send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_.send_string("vars_:=");
  rss.put_vars();
  cl_.send_string("$");

  
  cl_.send_string("symbolic redeval '(addConstraint cons_ vars_);");

  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

CheckConsistencyResult REDUCEVCS::check_consistency(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::FalseConditionsMode){ assert(0); }

  switch(mode_){ 
    case hydla::simulator::symbolic::DiscreteMode:
      // TODO myCheckConsistencyPointが本来、真偽が数式に依存する場合Trueが戻ってくるバグを持つ
      cl_.send_string("expr_:={}$ lcont_:={}$ vars:={}$");
      cl_.send_string("symbolic redeval '(myCheckConsistencyPoint);");
      break;
    case hydla::simulator::symbolic::FalseConditionsMode:
      // TODO 未実装
      assert(0);
      cl_.send_string("symbolic redeval '(checkFalseConditions);");
      break;
    default: // case hydla::simulator::symbolic::ContinuousMode:
      // TODO 記号定数による場合分けへの対応
      cl_.send_string("symbolic redeval '(myCheckConsistencyInterval);");
      break;
  }

  HYDLA_LOGGER_VCS( "--- REDUCEVCS::check_consistency receive ---");
  cl_.skip_until_redeval();

  std::string ans = cl_.get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ", ans);

  SExpParser sp;
  sp.parse_main(ans.c_str());
  // {true, false} または {false, true} の構造
  // TODO 記号定数を戻り値に取る場合の対応
  SExpParser::const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();

  // 第一要素を取得
  SExpParser::const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_first_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());

  CheckConsistencyResult ret;
  if(ret_first_str.find("true")!=std::string::npos){
    ret.true_parameter_maps.push_back(parameter_map_t());
  }else if(ret_first_str.find("false")!=std::string::npos){
    ret.false_parameter_maps.push_back(parameter_map_t());
  }else{
    // TODO: 上記以外の構造への対応
    assert(0);
  }

  HYDLA_LOGGER_FUNC_END(VCS);
  return ret;
}

SymbolicVirtualConstraintSolver::create_result_t REDUCEVCS::create_maps(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  if(mode_==hydla::simulator::symbolic::FalseConditionsMode){ assert(0); }

  //// REDUCEVCSPoint::create_maps
  // TODO: 不等式及び記号定数への対応
  REDUCEStringSender rss(cl_);
  /////////////////// 送信処理

  if(mode_==hydla::simulator::symbolic::DiscreteMode){
    cl_.send_string("symbolic redeval '(myConvertCSToVM);");
  }else{
    cl_.send_string("symbolic redeval '(convertCSToVMInterval);");
  }


  /////////////////// 受信処理                     

  cl_.skip_until_redeval();

  // S式パーサを用いて、制約ストア全体を表すような木構造を得る
  std::string cs_s_exp_str = cl_.get_s_expr();
  HYDLA_LOGGER_VCS("cs_s_exp_str: ", cs_s_exp_str);
  SExpParser sp;
  sp.parse_main(cs_s_exp_str.c_str());
  SExpParser::const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();

  create_result_t create_result;
  // TODO:以下のコードはor_size==1が前提
  //  for(int or_it = 0; or_it < or_size; or_it++){}
  {
    // {{(変数名), (関係演算子コード), (値のフル文字列)}, ...}の形式
    variable_range_map_t map;
    // TODO 不要？
    SExpConverter::clear_parameter_map();
    const size_t and_cons_size = tree_root_ptr->children.size();

    for(size_t i=0; i<and_cons_size; i++){
      SExpParser::const_tree_iter_t and_ptr = tree_root_ptr->children.begin()+i;

      std::string and_cons_string =  sp.get_string_from_tree(and_ptr);
      HYDLA_LOGGER_VCS("and_cons_string: ", and_cons_string);

      // 関係演算子のコード
      int relop_code;
      {
        SExpParser::const_tree_iter_t relop_code_ptr = and_ptr->children.begin()+1;      
        std::string relop_code_str = std::string(relop_code_ptr->value.begin(),relop_code_ptr->value.end());
        std::stringstream relop_code_ss;
        relop_code_ss << relop_code_str;
        relop_code_ss >> relop_code;
        assert(relop_code>=0 && relop_code<=4);
      }

      // 値
      SExpParser::const_tree_iter_t value_ptr = and_ptr->children.begin()+2;

      // 変数名
      std::string var_name;
      // 微分回数
      int var_derivative_count;
      {
        SExpParser::const_tree_iter_t var_ptr = and_ptr->children.begin();
        std::string var_head_str = std::string(var_ptr->value.begin(),var_ptr->value.end());

        // prevの先頭にスペースが入ることがあるので除去する
        // TODO:S式パーサを修正してスペース入らないようにする
        if(var_head_str.at(0) == ' ') var_head_str.erase(0,1);

        // prev変数は処理しない
        if(var_head_str=="prev") continue;

        var_derivative_count = sp.get_derivative_count(var_ptr);

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
        // TODO 要動作確認
        if(var_name.find(REDUCEStringSender::var_prefix, 0) != 0){
          // 'p'は取っておく必要がある
          assert(var_name.at(0) == REDUCEStringSender::par_prefix.at(0));
          var_name.erase(0, 1);

          variable_t* variable_ptr = get_variable(var_name, var_derivative_count);

          value_range_t tmp_range = map[variable_ptr];
          value_t tmp_value = SExpConverter::convert_s_exp_to_symbolic_value(sp, value_ptr);
          SExpConverter::set_range(tmp_value, tmp_range, relop_code);

          map[variable_ptr] = tmp_range;

          continue;
        }

        // "usrVar"を取り除く
        assert(var_name.find(REDUCEStringSender::var_prefix, 0) == 0);
        var_name.erase(0, REDUCEStringSender::var_prefix.length());

        // 大文字小文字表記に変換
        VariableNameEncoder vne;
        var_name = vne.UpperDecode(var_name);
      }

      // TODO: ↓の一行消す
      if(var_name == "t") continue;
      variable_t* variable_ptr = get_variable(var_name, var_derivative_count);

      // 変数表の対応する部分に代入する
      value_t symbolic_value = SExpConverter::convert_s_exp_to_symbolic_value(sp, value_ptr);
      if(symbolic_value->undefined()){
        throw SolveError("invalid value");
      }

      value_range_t tmp_range = map[variable_ptr];
      SExpConverter::set_range(symbolic_value, tmp_range, relop_code);
      map[variable_ptr] = tmp_range;
    }
    create_result.result_maps.push_back(map);
  }
  // TODO ?
  // SExpConverter::clear_parameter_map();

  for(unsigned int i=0; i < create_result.result_maps.size();i++){
    HYDLA_LOGGER_VCS("--- result map ", i, "/", create_result.result_maps.size(), "---\n");
    HYDLA_LOGGER_VCS(create_result.result_maps[i]);
  }

  HYDLA_LOGGER_FUNC_END(VCS);

  //TODO 戻り値を設定
  return create_result;

}

// TODO: 現状，制約に関しては変数自体を送り，変数リストは全部送るという，CalculateNextPPTime専用の仕様になってしまっているので，どうにかする．
void REDUCEVCS::reset_constraint(const variable_map_t& vm, const bool& send_derivatives){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  HYDLA_LOGGER_VCS("------Variable map------\n", vm);

  cl_.send_string("symbolic redeval '(resetConstraintForVariable);");
  cl_.skip_until_redeval();

  REDUCEStringSender rss(cl_);
  // ignore_prevは不要では？
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::FalseConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");

  cl_.send_string("cons_:={");
  variable_map_t::const_iterator it;
  bool isFirst = true;
  for(it=vm.begin(); it!=vm.end(); ++it){
    if((send_derivatives || it->first->get_derivative_count() == 0) && !it->second->undefined()){
      if(!isFirst) cl_.send_string(",");
      isFirst = false;

      const node_sptr lhs_node(make_variable(it->first->get_name(), it->first->get_derivative_count()));
      rss.put_node(node_sptr(new Equal(lhs_node, get_symbolic_value_t(it->second).get_node())), ignore_prev);
    }
  }
  cl_.send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_.send_string("vars_:={");
  for(it=vm.begin(); it!=vm.end(); ++it){
    if(it!=vm.begin()) cl_.send_string(",");
    const node_sptr node(make_variable(it->first->get_name(), it->first->get_derivative_count()));
    rss.put_node(node, ignore_prev);
  }
  cl_.send_string("}$");
  
  cl_.send_string("symbolic redeval '(addConstraint cons_ vars_);");

  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}


void REDUCEVCS::start_temporary(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  cl_.send_string("symbolic redeval '(startTemporary);");
  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

void REDUCEVCS::end_temporary(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  cl_.send_string("symbolic redeval '(endTemporary);");
  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);

  return;
}

// TODO: add_guardなのかset_guardなのかとか，仕様とかをはっきりさせる
void REDUCEVCS::add_guard(const node_sptr& constraint){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  REDUCEStringSender rss(cl_);
  const bool ignore_prev = (mode_==hydla::simulator::symbolic::ContinuousMode || mode_==hydla::simulator::symbolic::FalseConditionsMode);

  // cons_を渡す
  HYDLA_LOGGER_VCS("--- send cons_ ---");
  cl_.send_string("cons_:={");
      rss.put_node(constraint, ignore_prev);
  cl_.send_string("}$");

  // vars_を渡す
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_.send_string("vars_:=");
  rss.put_vars();
  cl_.send_string("$");

  switch(mode_){
    case hydla::simulator::symbolic::DiscreteMode:
      cl_.send_string("symbolic redeval '(addConstraint cons_ vars_);");
      break;
    case hydla::simulator::symbolic::FalseConditionsMode:
      cl_.send_string("symbolic redeval '(addGuard cons_ vars_);");
      break;
    default: // case hydla::simulator::symbolic::ContinuousMode:
      cl_.send_string("symbolic redeval '(setGuard cons_ vars_);");
      break;
  }

  cl_.skip_until_redeval();

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}


SymbolicVirtualConstraintSolver::PP_time_result_t REDUCEVCS::calculate_next_PP_time(
    const constraints_t& discrete_cause,
    const time_t& current_time,
    const time_t& max_time){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  ////////////////// 送信処理
  
  REDUCEStringSender rss(cl_);
  cl_.send_string("maxTime_:=");
  time_t tmp_time(max_time->clone());
  *tmp_time -= *current_time;
  HYDLA_LOGGER_VCS("%% current time:", *current_time);
  HYDLA_LOGGER_VCS("%% send time:", *tmp_time);
  rss.put_node(get_symbolic_value_t(tmp_time).get_node(), true);
  cl_.send_string("$");

  cl_.send_string("discCause_:={");
  for(constraints_t::const_iterator it = discrete_cause.begin(); it != discrete_cause.end();it++){
    if(it!=discrete_cause.begin()) cl_.send_string(",");
    rss.put_node(*it, true);
  }
  cl_.send_string("}$");

  cl_.send_string("symbolic redeval '(calculateNextPointPhaseTime maxTime_ discCause_);");

  ////////////////// 受信処理

  cl_.skip_until_redeval();
  // S式パーサを用いて、制約ストア全体を表すような木構造を得る
  std::string cs_s_exp_str = cl_.get_s_expr().c_str();
  HYDLA_LOGGER_VCS("cs_s_exp_str: ", cs_s_exp_str);
  SExpParser sp;
  sp.parse_main(cs_s_exp_str.c_str());
  SExpParser::const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();

  // {value_t{time_t), {}(parameter_map_t), true(bool)},...} のようなものが戻るはず
  PP_time_result_t result;

  const int candidate_size = (int)tree_root_ptr->children.size();
  for(int i = 0; i < candidate_size; i++){
    PP_time_result_t::candidate_t candidate;
    SExpParser::const_tree_iter_t and_ptr = tree_root_ptr->children.begin()+i;

    // create_mapsの受信処理を参考に以下を取得
    // 時刻を受け取る
    // TODO segfo
    candidate.time = SExpConverter::convert_s_exp_to_symbolic_value(sp, and_ptr->children.begin());

    // 条件を受け取る
    SExpParser::const_tree_iter_t param_ptr = and_ptr->children.begin()+1;
    std::string param_str = std::string(param_ptr->value.begin(), param_ptr->value.end());

    // TODO (list (list)) の判定を正確にする
    if(param_str.find("list")!=std::string::npos){
      candidate.parameter_map = parameter_map_t();
    }else{
      // TODO 空でない場合の対応
      assert(0);
    }

    // 終了時刻かどうかを受け取る
    SExpParser::const_tree_iter_t bool_ptr = and_ptr->children.begin()+2;
    std::string bool_str = std::string(bool_ptr->value.begin(), bool_ptr->value.end());
    candidate.is_max_time = (bool_str.find("1")!=std::string::npos);

    HYDLA_LOGGER_VCS("is_max_time: ",  candidate.is_max_time);
    HYDLA_LOGGER_VCS("--- parameter map ---\n",  candidate.parameter_map);
    result.candidates.push_back(candidate);
  }

  HYDLA_LOGGER_FUNC_END(VCS);

  return result;
}

void REDUCEVCS::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  REDUCEStringSender rss(cl_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", *(it->first));
    // 値
    value_t value;
    if(it->second->undefined()) {
      out_vm[it->first] = value;
      continue;
    }

    // applyTime2Expr(expr_, time_)を渡したい

    cl_.send_string("expr_:=");
    rss.put_node(get_symbolic_value_t(it->second).get_node(), true);
    cl_.send_string("$");


    cl_.send_string("time_:=");
    rss.put_node(get_symbolic_value_t(time).get_node(), true);
    //rss.put_node(get_symbolic_value_t(time).get_node(), true);
    //send_time(time);
    cl_.send_string("$");


    cl_.send_string("symbolic redeval '(applyTime2Expr expr_ time_);");

    
    ////////////////// 受信処理

    cl_.skip_until_redeval();

    std::string ans = cl_.get_s_expr();
    HYDLA_LOGGER_VCS("apply_time_to_expr_ans: ", ans);

    // S式パーサで読み取る
    SExpParser sp;
    sp.parse_main(ans.c_str());

    // {コード, 値}の構造
    SExpParser::const_tree_iter_t ct_it = sp.get_tree_iterator();

    // コードを取得
    SExpParser::const_tree_iter_t ret_code_it = ct_it->children.begin();
    std::string ret_code_str = std::string(ret_code_it->value.begin(), ret_code_it->value.end());
    HYDLA_LOGGER_VCS("ret_code_str: ",
                     ret_code_str);

    if(ret_code_str=="0") {
      // TODO: 適用に失敗（実数以外になる等）した場合。適切な処理をする
      assert(0);
    }
    else {
      assert(ret_code_str=="1");
      SExpParser::const_tree_iter_t value_it = ct_it->children.begin()+1;
      SExpConverter sc;
      value = sc.convert_s_exp_to_symbolic_value(sp, value_it);
      HYDLA_LOGGER_REST("new value : ", value->get_string());
    }

    out_vm[it->first] = value;
  }

  HYDLA_LOGGER_FUNC_END(VCS);
}

std::string REDUCEVCS::get_constraint_store(){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  std::string ret;

  cl_.send_string("symbolic redeval '(getConstraintStore);");
  cl_.skip_until_redeval();

  ret = cl_.get_s_expr();
  HYDLA_LOGGER_FUNC_END(VCS);
  return ret;
}

// deleted
////value_tを指定された精度で数値に変換する
//std::string REDUCEVCS::get_real_val(const value_t &val, int precision, hydla::simulator::symbolic::OutputFormat opfmt){
//  std::string ret;
//  REDUCEStringSender rss(cl_);
//
//  if(!val.undefined()) {
//    
//    cl_.send_string("on rounded$");
//
//    // getRealVal(value_, prec_)を渡したい
//    cl_.send_string("value_:=");
//    rss.put_node(val, true);
//    cl_.send_string("$");
//    
//    std::stringstream precision_str;
//    precision_str << precision;
//    cl_.send_string("prec_:="+ precision_str.str() +"$");
//    // 計算に用いる精度は6ケタ未満にできない（？）ようなので，表示桁を下げる
//    if(precision < 6){
//      cl_.send_string("print_precision(" + precision_str.str() + ")$");
//    }
//    cl_.send_string("getRealVal(value_, prec_);");
//    
//    cl_.skip_until_redeval();
//    cl_.get_line();
//    ret = cl_.get_line();
//    cl_.send_string("off rounded$");
//    // 精度を元に戻しておく
//    cl_.send_string("precision(defaultPrec_)$");
//  }
//  else {
//    ret = "UNDEF";
//  }
//  return ret;
//}


bool REDUCEVCS::less_than(const time_t &lhs, const time_t &rhs)
{
  assert(0);
  return true;
  //HYDLA_LOGGER_FUNC_BEGIN(VCS);

  //REDUCEStringSender rss(cl_);

  //// checkLessThan(lhs_, rhs_)を渡したい
  //

  //cl_.send_string("lhs_:=");
  //rss.put_node(lhs, true);
  //cl_.send_string("$");


  //cl_.send_string("rhs_:=");
  //rss.put_node(rhs, true);
  //cl_.send_string("$");

  //cl_.send_string("symbolic redeval '(checkLessThan lhs_ rhs_);");


  //////////////////// 受信処理

  //// cl_.read_until_redeval();
  //cl_.skip_until_redeval();

  //std::string ans = cl_.get_s_expr();
  //HYDLA_LOGGER_VCS("check_less_than_ans: ", ans);
  //HYDLA_LOGGER_FUNC_END(VCS);
  //return  boost::lexical_cast<int>(ans) == 1;
}


void REDUCEVCS::simplify(time_t &time) 
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  HYDLA_LOGGER_DEBUG("SymbolicTime::send_time : ", time);
  REDUCEStringSender rss(cl_);

  // simplifyExpr(expr_)を渡したい

  cl_.send_string("expr_:=");
  rss.put_node(get_symbolic_value_t(time).get_node(), true);
  cl_.send_string("$");


  cl_.send_string("symbolic redeval '(simplifyExpr expr_);");


  ////////////////// 受信処理

  // cl_.read_until_redeval();
  cl_.skip_until_redeval();

  std::string ans = cl_.get_s_expr();
  HYDLA_LOGGER_VCS("simplify_ans: ", ans);

  // S式パーサで読み取る
  SExpParser sp;
  sp.parse_main(ans.c_str());
  SExpParser::const_tree_iter_t time_it = sp.get_tree_iterator();
  SExpConverter sc;
  time = sc.convert_s_exp_to_symbolic_value(sp, time_it);

  HYDLA_LOGGER_FUNC_END(VCS);
}
/*
 * SymbolicValueの時間をずらす
 */
hydla::vcs::SymbolicVirtualConstraintSolver::value_t REDUCEVCS::shift_expr_time(const value_t& val, const time_t& time){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  REDUCEStringSender rss(cl_);

  // exprTimeShift(expr_, time_)を渡したい

  cl_.send_string("expr_:=");
  rss.put_node(get_symbolic_value_t(val).get_node(), true);
  cl_.send_string("$");


  cl_.send_string("time_:=");
  rss.put_node(get_symbolic_value_t(time).get_node(), true);
  cl_.send_string("$");


  cl_.send_string("symbolic redeval '(exprTimeShift expr_ time_);");


  ////////////////// 受信処理

  // cl_.read_until_redeval();
  cl_.skip_until_redeval();

  std::string ans = cl_.get_s_expr();
  HYDLA_LOGGER_VCS("expr_time_shift_ans: ", ans);

  // S式パーサで読み取る
  SExpParser sp;
  sp.parse_main(ans.c_str());
  SExpParser::const_tree_iter_t value_it = sp.get_tree_iterator();
  SExpConverter sc;

  HYDLA_LOGGER_FUNC_END(VCS);
  return  sc.convert_s_exp_to_symbolic_value(sp, value_it);
}

// TODO なんとかする
void REDUCEVCS::approx_vm(variable_range_map_t& vm){
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  HYDLA_LOGGER_FUNC_END(VCS);
}

const REDUCEVCS::symbolic_value_t REDUCEVCS::get_symbolic_value_t(value_t value){
  value->accept(*this);
  return visited_;
}


void REDUCEVCS::visit(symbolic_value_t& value){
  visited_ = value;
}

const node_sptr REDUCEVCS::make_variable(const std::string &name, const int& dc, const bool& is_prev) const {
  node_sptr var_node(new Variable(name));
  for (int i = 0; i < dc; i++) {
    var_node = node_sptr(new Differential(var_node));
  }
  if(is_prev){
    var_node = node_sptr(new Previous(var_node));
  }

  return var_node; 
  }
} // namespace reduce
} // namespace vcs
} // namespace hydla 
