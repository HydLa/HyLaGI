#include "REDUCEVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "Logger.h"
#include "SExpConverter.h"
#include "../SolveError.h"

using namespace hydla::vcs;
using namespace hydla::logger;
using namespace hydla::parse_tree;

namespace hydla {
namespace vcs {
namespace reduce {


REDUCEVCSInterval::REDUCEVCSInterval(REDUCELink* cl, int approx_precision) :
  cl_(cl),
  approx_precision_(approx_precision)
{
}

REDUCEVCSInterval::~REDUCEVCSInterval()
{}

void REDUCEVCSInterval::set_continuity(const continuity_map_t &continuity_map){
  continuity_map_ = continuity_map;
}

void REDUCEVCSInterval::send_init_cons(REDUCEStringSender& rss, const continuity_map_t& continuity_map)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::send_init_cons ***");

  constraints_t constraints;
  constraints_t t_continuity_constraints;

  continuity_map_t::const_iterator cm_it = continuity_map.begin(), cm_end = continuity_map.end();
  for(; cm_it != cm_end; ++cm_it) {
    if(cm_it->second < 0){
      t_continuity_constraints.push_back(SExpConverter::make_equal(variable_t(cm_it->first, abs(cm_it->second)+1), 
                                                                   node_sptr(new Number("0")), 
                                                                   false));
    }
    for(int i=0; i < (abs(cm_it->second) + (cm_it->second < 0)); i++)
      {
        node_sptr rhs = node_sptr(new Previous(node_sptr(new Variable(cm_it->first))));
        for(int j=0; j < i; j++){
          rhs = node_sptr(new Differential(rhs));
        }
        constraints.push_back(SExpConverter::make_equal(variable_t(cm_it->first, i), rhs, false, true));
      }
  }

  cl_->send_string("union(");
  rss.put_nodes(t_continuity_constraints);
  cl_->send_string(",");
  rss.put_nodes(constraints);
  cl_->send_string(")");

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::send_init_cons ***");

}

void REDUCEVCSInterval::send_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::send_constraint ***");

  REDUCEStringSender rss(*cl_);


//////////////////// ���M����

  // checkConsistencyInterval(tmpCons_, rcont_, vars_)��n������

  // �����n��
  HYDLA_LOGGER_VCS("--- send tmpCons_ ---");
  cl_->send_string("tmpCons_:={");
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it) {
    if(it!=constraints.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", (**it));
    rss.put_node(*it, true);
  }
  cl_->send_string("}$");


  // �E�A��������rcont_��n��
  HYDLA_LOGGER_VCS("--- send rcont_ ---");
  cl_->send_string("rcont_:=");
  send_init_cons(rss, continuity_map_);
  cl_->send_string("$");


  // vars��n��
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_->send_string("vars_:=union(");
  rss.put_vars(true);
  cl_->send_string(")$");

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::send_constraint ***");
  return;
}
  
VCSResult REDUCEVCSInterval::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::check_consistency(tmp) ***");
  send_constraint(constraints);
  cl_->send_string("symbolic redeval '(checkConsistencyInterval tmpCons_ rcont_ vars_);");
  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::check_consistency(tmp) ***");
  return result;
}


VCSResult REDUCEVCSInterval::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::check_consistency() ***");
  send_constraint(constraints_t());
  cl_->send_string("symbolic redeval '(checkConsistencyInterval tmpCons_ rcont_ vars_);");
  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("\n#*** End REDUCEVCSInterval::check_consistency() ***");
  return result;
}

VCSResult REDUCEVCSInterval::check_entailment(const node_sptr &node)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::check_entailment ***");

  constraints_t constraints;
  constraints.push_back(node);
  send_constraint(constraints);
  cl_->send_string("symbolic redeval '(checkConsistencyInterval tmpCons_ rcont_ vars_);");
  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::check_entailment ***");
  return result;
}

VCSResult REDUCEVCSInterval::check_consistency_receive()
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::check_consistency_receive ***");

/////////////////// ��M����
  HYDLA_LOGGER_EXTERN("--- receive  ---");

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ",
                   ans);
  
  VCSResult result;


  // S���p�[�T�œǂݎ��
  SExpParser sp;
  sp.parse_main(ans.c_str());

  // {�R�[�h}�̍\��
  const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();

  // �R�[�h���擾
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ",
                   ret_code_str);

  if(ret_code_str=="0"){
    // �\���o�G���[
    throw hydla::vcs::SolveError(ans);
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code_str=="1") {
    // �[��
    // TODO: �X�y�[�X��""���c��Ȃ��悤�Ƀp�[�T���C��
    result = VCSR_TRUE;
  }
  else {
    // ����G���[
    assert(ret_code_str == "2");
    result = VCSR_FALSE;
  }
  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::check_consistency_receive ***");

  return result;
}

VCSResult REDUCEVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const constraints_t &disc_cause,
  const time_t& current_time,
  const time_t& max_time)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::integrate ***");
//  HYDLA_LOGGER_VCS(*this);

/////////////////// ���M����
  REDUCEStringSender rss(*cl_);

  // integrateCalc(init_, discCause_, vars_, maxTime_]��n������

  // �����l����init_cons��n��
  HYDLA_LOGGER_VCS("--- send init_ ---");
  cl_->send_string("init_:=");
  send_init_cons(rss, continuity_map_);
  cl_->send_string("$");


  // ���U�ω��̏�����n��
  HYDLA_LOGGER_VCS("--- send discCause_ ---");
  cl_->send_string("discCause_:={");
  for(constraints_t::const_iterator it = disc_cause.begin(); it != disc_cause.end();it++){
    if(it!=disc_cause.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("--- send discCause ---");
    rss.put_node(*it, true);
  }
  cl_->send_string("}$");


  // �ϐ��̃��X�g��n��
  HYDLA_LOGGER_VCS("--- send vars_ ---");
  cl_->send_string("vars_:=union(");
  rss.put_vars(true);
  cl_->send_string(")$");


  // maxTime��n��
  HYDLA_LOGGER_VCS("--- send maxTime_ ---");
  cl_->send_string("maxTime_:=");
  time_t tmp_time(max_time);
  tmp_time -= current_time;
  HYDLA_LOGGER_VCS("current time:", current_time);
  HYDLA_LOGGER_VCS("send time:", tmp_time);
  if(approx_precision_ > 0) {
    // �ߎ����đ��M
    // TODO:���Ƃ�����
//    ml_->put_function("approxExpr", 2);
//    ml_->put_integer(approx_precision_);
  }
  send_time(tmp_time);
  cl_->send_string("$");


  cl_->send_string("symbolic redeval '(integrateCalc init_ discCause_ vars_ maxTime_);");


/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("integrate_ans: ",
                   ans);
  
  // S���p�[�T�œǂݎ��
  SExpParser sp;
  sp.parse_main(ans.c_str());
  SExpConverter sc;

  // {�R�[�h, {t�̎��ŕ\�����ϐ��\}, IP�I������}�̍\��
  // TODO:IP�I���̏������Ԃ��āA�I�������̏W����Ԃ���悤�ɂ���
  const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();

  // �R�[�h���擾
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ",
                   ret_code_str);

  if(ret_code_str=="0"){
    // �\���o�G���[
    throw hydla::vcs::SolveError(ans);
    return VCSR_SOLVER_ERROR;
  }
  assert(ret_code_str=="1");

  HYDLA_LOGGER_VCS("--- integrate calc result ---");
  virtual_constraint_solver_t::IntegrateResult::NextPhaseState state;


  // �ϐ��ƒl�̑g���󂯎��
  const_tree_iter_t variable_pair_list_ptr = ret_code_ptr+1;
  size_t variable_pair_size = variable_pair_list_ptr->children.size();
  HYDLA_LOGGER_VCS( "variable_pair_size: ", variable_pair_size);

  for(size_t i=0; i<variable_pair_size; i++)
  {
    const_tree_iter_t variable_pair_it = variable_pair_list_ptr->children.begin()+i;
    variable_t variable;
    value_t    value;
    HYDLA_LOGGER_VCS("--- add variable ---");


    // �ϐ���
    const_tree_iter_t var_ptr = variable_pair_it->children.begin();
    std::string var_head_str = std::string(var_ptr->value.begin(),var_ptr->value.end());

    std::string var_name;
    int var_derivative_count = sp.get_derivative_count(var_ptr);

    // �������܂ޕϐ�
    if(var_derivative_count > 0){
      var_name = std::string(var_ptr->children.begin()->value.begin(), 
                             var_ptr->children.begin()->value.end());
    }
    // �������܂܂Ȃ��ϐ�
    else{
      assert(var_derivative_count == 0);
      var_name = var_head_str;
    }

    // �ϐ����̐擪�ɃX�y�[�X�����邱�Ƃ�����̂ŏ�������
    // TODO:S���p�[�T���C�����ăX�y�[�X����Ȃ��悤�ɂ���
    if(var_name.at(0) == ' ') var_name.erase(0,1);

    assert(var_name.find(REDUCEStringSender::var_prefix, 0) == 0);
    variable.name = var_name.substr(REDUCEStringSender::var_prefix.length());
    variable.derivative_count = var_derivative_count;
    HYDLA_LOGGER_VCS("name  : ", variable.name);
    HYDLA_LOGGER_VCS("derivative : ", variable.derivative_count);

    // �l��
    const_tree_iter_t value_ptr = variable_pair_it->children.begin()+1;
    value = sc.convert_s_exp_to_symbolic_value(sp, value_ptr);
    HYDLA_LOGGER_VCS("value : ", value.get_string());

    state.variable_map.set_variable(variable, value); 
  }


  // ����PP�̎����ƁC���̏ꍇ�̏����̑g�C�X�ɏI���������ǂ����𓾂�
  HYDLA_LOGGER_VCS("--- receive next PP time ---");
  const_tree_iter_t next_pp_time_list_ptr = variable_pair_list_ptr+1;
  size_t next_time_size = next_pp_time_list_ptr->children.size();
  HYDLA_LOGGER_VCS("next_time_size: ", next_time_size);
  for(size_t time_it = 0; time_it < next_time_size; time_it++){
    const_tree_iter_t next_time_info_ptr = next_pp_time_list_ptr->children.begin()+time_it;
    // IP�I������
    const_tree_iter_t next_time_ptr = next_time_info_ptr->children.begin();
    state.time = sc.convert_s_exp_to_symbolic_value(sp, next_time_ptr) + current_time;
    HYDLA_LOGGER_VCS("next_phase_time: ", state.time);


    // ������
    const_tree_iter_t pp_condition_or_ptr = next_time_ptr+1;
    // �O��FOr�łȂ��ꂽ�_���ς�1�̂�
    // TODO�F2�ڈȍ~����M�ł���悤�ɂ���
    const_tree_iter_t pp_condition_and_ptr = pp_condition_or_ptr->children.begin();
    size_t pp_condition_size = pp_condition_and_ptr->children.size();

    state.parameter_map.clear();
    parameter_t tmp_param;
    for(size_t cond_it = 0; cond_it < pp_condition_size; cond_it++){
      const_tree_iter_t param_condition_ptr = pp_condition_and_ptr->children.begin()+cond_it;

      const_tree_iter_t param_name_ptr = param_condition_ptr->children.begin();
      std::string param_name = std::string(param_name_ptr->value.begin(), param_name_ptr->value.end());
      // �萔���̐擪�ɃX�y�[�X�����邱�Ƃ�����̂ŏ�������
      // TODO:S���p�[�T���C�����ăX�y�[�X����Ȃ��悤�ɂ���
      if(param_name.at(0) == ' ') param_name.erase(0,1);
      // 'p'�͎���Ă����K�v������
      assert(param_name.at(0) == REDUCEStringSender::par_prefix.at(0));
      param_name.erase(0, 1);
      tmp_param.set_variable(parameter_t::get_variable(param_name));
      HYDLA_LOGGER_VCS("returned parameter_name: ", tmp_param.get_name());

      const_tree_iter_t relop_code_ptr = param_name_ptr+1;
      std::string relop_code_str = std::string(relop_code_ptr->value.begin(), relop_code_ptr->value.end());
      std::stringstream relop_code_ss;
      int relop_code;
      relop_code_ss << relop_code_str;
      relop_code_ss >> relop_code;
      HYDLA_LOGGER_VCS("returned relop_code: ", relop_code);
      assert(relop_code>=0 && relop_code<=4);

      const_tree_iter_t param_value_ptr = relop_code_ptr+1;

      value_range_t tmp_range = state.parameter_map.get_variable(tmp_param);
      value_t tmp_value = SExpConverter::convert_s_exp_to_symbolic_value(sp, param_value_ptr);
      SExpConverter::set_range(tmp_value, tmp_range, relop_code);
      state.parameter_map.set_variable(tmp_param, tmp_range);
    }


    // �V�~�����[�V�����I�������ɒB�������ǂ���
    const_tree_iter_t max_time_flag_ptr = pp_condition_or_ptr+1;
    int max_time_flag;
    std::stringstream max_time_flag_ss;
    std::string max_time_flag_str = std::string(max_time_flag_ptr->value.begin(), max_time_flag_ptr->value.end());
    max_time_flag_ss << max_time_flag_str;
    max_time_flag_ss >> max_time_flag;
    state.is_max_time = (max_time_flag == 1);
    HYDLA_LOGGER_VCS("is_max_time: ",  state.is_max_time);
    HYDLA_LOGGER_VCS("--- parameter map ---\n", state.parameter_map);



    // ����`�̕ϐ���ϐ��\�ɔ��f
    // �����l����i����`�ϐ����܂ށj��variable_map�Ƃ̍���������
    add_undefined_vars_to_vm(state.variable_map);

    integrate_result.states.push_back(state);
  }

/////////////////// ��M�I��

  for(unsigned int state_it = 0; state_it < integrate_result.states.size(); state_it++){
    // �����̊Ȗ񉻁B�{���͊֐��g���Ă��ׂ������ǁA�Ƃ肠�������̂܂܂�����
    HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", integrate_result.states[state_it].time);

    cl_->send_string("expr_:=");
    rss.put_node(integrate_result.states[state_it].time.get_node(), true);
    cl_->send_string("$");
    cl_->send_string("symbolic redeval '(simplifyExpr expr_);");


    // cl_->read_until_redeval();
    cl_->skip_until_redeval();

    std::string simpl_time_str = cl_->get_s_expr();
    HYDLA_LOGGER_VCS("simpl_time_str: ", simpl_time_str);
    sp.parse_main(simpl_time_str.c_str());

    const_tree_iter_t tree_root_ptr = sp.get_tree_iterator();
    integrate_result.states[state_it].time = sc.convert_s_exp_to_symbolic_value(sp, tree_root_ptr);

    /*
    // �����̋ߎ�
    if(approx_precision_ > 0) {

      ml_->put_function("ToString", 1);
      ml_->put_function("FullForm", 1);
      ml_->put_function("approxExpr", 2);
      ml_->put_integer(approx_precision_);
      send_time(integrate_result.states[state_it].time);
      ml_->skip_pkt_until(RETURNPKT);
      ml_->MLGetNext();
      integrate_result.states[state_it].time = mec.convert_math_string_to_symbolic_value(ml_->get_string());
    }
    */
  }

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::integrate ***");

  return VCSR_TRUE;
}

void REDUCEVCSInterval::send_time(const time_t& time){
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::send_time ***");
  HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", time);
  REDUCEStringSender rss(*cl_);
  rss.put_node(time.get_node(), false);

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::send_time ***");
}

void REDUCEVCSInterval::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::apply_time_to_vm ***");

  REDUCEStringSender rss(*cl_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", it->first);
    HYDLA_LOGGER_VCS("value : ", it->second);
    HYDLA_LOGGER_VCS("time : ", time);

    // �l
    value_t    value;

    if(it->second.is_undefined()) {
      out_vm.set_variable(it->first, value);
      continue;
    }

    // applyTime2Expr(expr_, time_)��n������

    cl_->send_string("expr_:=");
    rss.put_node(it->second.get_node(), true);
    cl_->send_string("$");


    cl_->send_string("time_:=");
    send_time(time);
    cl_->send_string("$");


    cl_->send_string("symbolic redeval '(applyTime2Expr expr_ time_);");

    
    ////////////////// ��M����

    // cl_->read_until_redeval();
    cl_->skip_until_redeval();

    std::string ans = cl_->get_s_expr();
    HYDLA_LOGGER_VCS("apply_time_to_expr_ans: ", ans);

    // S���p�[�T�œǂݎ��
    SExpParser sp;
    sp.parse_main(ans.c_str());

    // {�R�[�h, �l}�̍\��
    const_tree_iter_t ct_it = sp.get_tree_iterator();

    // �R�[�h���擾
    const_tree_iter_t ret_code_it = ct_it->children.begin();
    std::string ret_code_str = std::string(ret_code_it->value.begin(), ret_code_it->value.end());
    HYDLA_LOGGER_VCS("ret_code_str: ",
                     ret_code_str);

    if(ret_code_str=="0") {
      // TODO: �K�p�Ɏ��s�i�����ȊO�ɂȂ铙�j�����ꍇ�B�K�؂ȏ���������
      assert(0);
    }
    else {
      assert(ret_code_str=="1");
      const_tree_iter_t value_it = ct_it->children.begin()+1;
      SExpConverter sc;
      value = sc.convert_s_exp_to_symbolic_value(sp, value_it);
      HYDLA_LOGGER_REST("value : ", value.get_string());
    }

    out_vm.set_variable(it->first, value);
  }

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::apply_time_to_vm ***");

}

void REDUCEVCSInterval::add_undefined_vars_to_vm(variable_map_t& vm)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::add_undefined_vars_to_vm ***");

  /*
  // �ϐ��\�ɓo�^����Ă���ϐ����ꗗ
  HYDLA_LOGGER_VCS("-- variable_name_list --");
  std::set<REDUCEVariable> variable_name_list;
  variable_map_t::const_iterator vm_it = vm.begin();
  variable_map_t::const_iterator vm_end = vm.end();
  for(; vm_it!=vm_end; ++vm_it){
    variable_name_list.insert(vm_it->first);
    HYDLA_LOGGER_VCS(vm_it->first);
  }

  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  HYDLA_LOGGER_VCS("-- search undefined variable --");
  // �����l����ϐ��̂����A�ϐ��\�ɓo�^����Ă���ϐ����ꗗ���ɂȂ����̂𒊏o�H
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    variable_t variable = init_vars_it->first;
    std::set<REDUCEVariable>::const_iterator vlist_it = variable_name_list.find(variable);
    if(vlist_it==variable_name_list.end()){      
      value_t value;
      HYDLA_LOGGER_VCS("variable : ", variable);
      HYDLA_LOGGER_VCS("value : ", value);
      vm.set_variable(variable, value);
    }
  }
  */

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSInterval::add_undefined_vars_to_vm ***");

}


} // namespace reduce
} // namespace simulator
} // namespace hydla 

