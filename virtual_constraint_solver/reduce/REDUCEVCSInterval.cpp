#include "REDUCEVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "Logger.h"
#include "SExpConverter.h"

using namespace hydla::vcs;
using namespace hydla::logger;

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

bool REDUCEVCSInterval::reset()
{
  constraint_store_ = constraint_store_t();
  return true;
}

bool REDUCEVCSInterval::reset(const variable_map_t& variable_map)
{

  HYDLA_LOGGER_VCS_SUMMARY("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Variable map------\n", 
                     variable_map);

  variable_map_t::variable_list_t::const_iterator it  = variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = variable_map.end();
  for(; it!=end; ++it) {
    if(!it->second.is_undefined()) {
      constraint_store_.init_vars.insert(
        std::make_pair(it->first, it->second));
    }
    else {
      value_t value;
      constraint_store_.init_vars.insert(
        std::make_pair(it->first, value));
    }
  }
  return true;
}



bool REDUCEVCSInterval::reset(const variable_map_t& variable_map,  const parameter_map_t& parameter_map)
{
  if(!reset(variable_map)){
    return false;
  }
  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", 
                     parameter_map);
  parameter_map_=parameter_map;
  HYDLA_LOGGER_VCS(constraint_store_);
  return true;
}

void REDUCEVCSInterval::set_continuity(const continuity_map_t &continuity_map){
  continuity_map_ = continuity_map;
}

void REDUCEVCSInterval::send_init_cons(
  REDUCEStringSender& rss, 
  const max_diff_map_t& max_diff_map,
  bool use_approx)
{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSInterval::send_init_cons ----");
    
  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();

  cl_->send_string("{");
  bool first_element = true;
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);

    // �����l����̂����A�W�߂�tell����ɏo������ۂ̍ő�����񐔂��������������񐔂̂��̂̂ݑ���
    if(md_it!=max_diff_map.end() &&
       md_it->second  > init_vars_it->first.derivative_count) 
    {
      if(!first_element) cl_->send_string(",");
      // �ϐ���
      rss.put_var(boost::make_tuple(init_vars_it->first.name, 
                                    init_vars_it->first.derivative_count, 
                                    false),
                  true);

      cl_->send_string("=");
  
      // �l
      if(use_approx && approx_precision_ > 0) {
        // �ߎ����đ��M
        // TODO:���Ƃ�����
//        ml_->put_function("approxExpr", 2);
//        ml_->put_integer(approx_precision_);
      }
      rss.put_node(init_vars_it->second.get_node(), false);
      first_element = false;
    }
  }
  cl_->send_string("}");

}



void REDUCEVCSInterval::send_parameter_cons() const{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSInterval::send_parameter_cons ----");


  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();
  int para_size=0;
  for(; par_it!=par_end; ++par_it) {
    value_range_t::or_vector::const_iterator or_it = par_it->second.or_begin(), or_end = par_it->second.or_end();
    for(;or_it != or_end; or_it++){
      para_size += or_it->size();
    }
  }

  HYDLA_LOGGER_VCS("parameter_cons_count: ", para_size);
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", 
                     parameter_map_);

  // TODO: �����Ƒ���
  cl_->send_string("{}");
}


void REDUCEVCSInterval::send_pars() const{
  HYDLA_LOGGER_VCS("---- Begin REDUCEVCSInterval::send_pars ----");


  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();

  // TODO: �����Ƒ���
  cl_->send_string("{}");
}



void REDUCEVCSInterval::send_vars(REDUCEStringSender& rss, const max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- REDUCEVCSInterval::send_vars ----");

  cl_->send_string("{");
  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();
  bool first_element = true;
  for(md_it=max_diff_map.begin(); md_it!=md_end; ++md_it) {
    for(int i=0; i<=md_it->second; i++) {
      if(!first_element) cl_->send_string(",");
      rss.put_var(boost::make_tuple(md_it->first, i, false));

      HYDLA_LOGGER_VCS("put: ",
                       "  name: ", md_it->first,
                       "  diff: ", i);
      first_element = false;
    }
  }
  cl_->send_string("}");
}

void REDUCEVCSInterval::add_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::add_constraint ***");

  // ����X�g�A��tell����̒ǉ�
  constraint_store_.constraints.insert(
    constraint_store_.constraints.end(),
    constraints.begin(),
    constraints.end()
    );

  HYDLA_LOGGER_VCS(
    constraint_store_,
    "\n#*** End REDUCEVCSInterval::add_constraint ***");
  return;
}
  
VCSResult REDUCEVCSInterval::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::check_consistency(tmp) ***");
  tmp_constraints_ = constraints;
  VCSResult result = check_consistency_sub();
  switch(result){
    case VCSR_TRUE:
      HYDLA_LOGGER_VCS_SUMMARY("consistent");
      break;

    case VCSR_FALSE:
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");
      break;

    default:
      assert(0);
      break;

    case VCSR_UNKNOWN:
      // �Ƃ肠����
      HYDLA_LOGGER_VCS_SUMMARY("consistent in some case");
      result = VCSR_TRUE;
//      ml_->MLNewPacket();
      break;
  }
  tmp_constraints_.clear();
  return result;
}


VCSResult REDUCEVCSInterval::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::check_consistency() ***");
  VCSResult result = check_consistency_sub();
  switch(result){
    case VCSR_TRUE:
      HYDLA_LOGGER_VCS_SUMMARY("consistent");
      break;

    case VCSR_FALSE:
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");
      break;

    default:
      assert(0);
      break;

    case VCSR_UNKNOWN:
      // �Ƃ肠����
      HYDLA_LOGGER_VCS_SUMMARY("consistent in some case");
      result = VCSR_TRUE;
      HYDLA_LOGGER_VCS( "--- receive added condition ---");
/*
      {
        std::string str = ml_->get_string();
        added_condition_.set(str);
        HYDLA_LOGGER_VCS(str);
      }
*/
      break;
  }
  return result;
}

VCSResult REDUCEVCSInterval::check_consistency_sub()
{

  REDUCEStringSender rss(*cl_);


//////////////////// ���M����

  // isConsistentInterval(tmpCons_, expr_, pexpr_, init_, vars_)��n������

  // �ꎞ�I�Ȑ���X�g�A(�V�����ǉ����ꂽ����̏W��)����expr�𓾂�REDUCE�ɓn��
  HYDLA_LOGGER_VCS("----- send tmpCons_ -----");
  cl_->send_string("tmpCons_:={");
  constraints_t::const_iterator tmp_it  = tmp_constraints_.begin();
  constraints_t::const_iterator tmp_end = tmp_constraints_.end();
  for(; tmp_it!= tmp_end; ++tmp_it) {
    if(tmp_it != tmp_constraints_.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", (**tmp_it));
    rss.put_node(*tmp_it, true);
  }
  cl_->send_string("}$");  


  // ����X�g�Aconstraints��REDUCE�ɓn��
  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=");
  send_cs(rss);
  cl_->send_string("$");


  // �p�����^����parameter_cons��REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send pexpr_ ---");
  cl_->send_string("pexpr_:=");
  send_parameter_cons();
  cl_->send_string("$");


  // max_diff_map�����ƂɁA�����l����init_cons��n��
  HYDLA_LOGGER_VCS("----- send init_ -----");  
  cl_->send_string("init_:=");
  max_diff_map_t max_diff_map;
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  rss.create_max_diff_map(max_diff_map);
  // �����l����̑��M
  send_init_cons(rss, max_diff_map, false);
  cl_->send_string("$");


  // vars��n��
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  rss.put_vars(true);
  cl_->send_string("$");


  cl_->send_string("symbolic redeval '(isConsistentInterval tmpCons_ expr_ pexpr_ init_ vars_);");


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
  return result;
}

VCSResult REDUCEVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const constraints_t &disc_cause,
  const time_t& current_time,
  const time_t& max_time)
{
  HYDLA_LOGGER_VCS("#*** REDUCEVCSInterval::integrate ***");
//  HYDLA_LOGGER_VCS(*this);

/////////////////// ���M����
  REDUCEStringSender rss(*cl_);

  // integrateCalc(cons_, init_, discCause_, vars_, maxTime_]��n������

  // cons_��n���iconstraint_store�Aparameter_cons�Aadded_condition��3���琬��j
  HYDLA_LOGGER_VCS("----- send cons_ -----");
  cl_->send_string("cons_:=union(union(");

  // ����X�g�Aconstraints��REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send constraint_store ---");
  send_cs(rss);
  cl_->send_string(",");

  // parameter_cons��n��
  HYDLA_LOGGER_VCS("--- send parameter_cons ---");
  send_parameter_cons();
  cl_->send_string("),");

  // added_condition��n��
  HYDLA_LOGGER_VCS("--- send added_condition ---");
  cl_->send_string("{");
  cl_->send_string(added_condition_.get_string());
  cl_->send_string("})$");


  // max_diff_map�����ƂɁA�����l����init_cons��n��
  HYDLA_LOGGER_VCS("----- send init_ -----");  
  cl_->send_string("init_:=");
  max_diff_map_t max_diff_map;
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  rss.create_max_diff_map(max_diff_map);
  // �����l����̑��M
  send_init_cons(rss, max_diff_map, false);
  cl_->send_string("$");


  // ���U�ω��̏�����n��
  HYDLA_LOGGER_VCS("----- send discCause_ -----");
  cl_->send_string("discCause_:={");
  for(constraints_t::const_iterator it = disc_cause.begin(); it != disc_cause.end();it++){
    if(it!=disc_cause.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("send discCause:");
    rss.put_node(*it, true);
  }
  cl_->send_string("}$");


  // �ϐ��̃��X�g��n��
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  send_vars(rss, max_diff_map);
  cl_->send_string("$");


  // maxTime��n��
  HYDLA_LOGGER_VCS("----- send maxTime_ -----");
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


  cl_->send_string("symbolic redeval '(integrateCalc cons_ init_ discCause_ vars_ maxTime_);");


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
    return VCSR_SOLVER_ERROR;
  }
  assert(ret_code_str=="1");

  HYDLA_LOGGER_VCS("---integrate calc result---");
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
  HYDLA_LOGGER_VCS("-- receive next PP time --");
  const_tree_iter_t next_pp_time_list_ptr = variable_pair_list_ptr+1;
  size_t next_time_size = next_pp_time_list_ptr->children.size();
  HYDLA_LOGGER_VCS("next_time_size: ", next_time_size);
  for(size_t time_it = 0; time_it < next_time_size; time_it++){
    const_tree_iter_t next_time_info_ptr = next_pp_time_list_ptr->children.begin()+time_it;
    // IP�I������
    const_tree_iter_t next_time_ptr = next_time_info_ptr->children.begin();
    state.time = sc.convert_s_exp_to_symbolic_value(sp, next_time_ptr) + current_time;
    HYDLA_LOGGER_VCS_SUMMARY("next_phase_time: ", state.time);

/*
    // ������
    const_tree_iter_t pp_condition_list_ptr = next_time_ptr+1;
    size_t pp_condition_size = pp_condition_list_ptr->children.size();

    state.parameter_map.clear();
    parameter_t tmp_param, prev_param;
    for(int cond_it = 0; cond_it < condition_size; cond_it++){
      value_range_t tmp_range;
      ml_->MLGetNext(); ml_->MLGetNext();
      tmp_param.name = ml_->get_string();
      HYDLA_LOGGER_VCS("returned parameter_name: ", tmp_param.name);
      ml_->MLGetNext();
      int relop_code = ml_->get_integer();
      HYDLA_LOGGER_VCS("returned relop_code: ", relop_code);
      ml_->MLGetNext();
      std::string parameter_value_string = ml_->get_string();
      HYDLA_LOGGER_VCS("returned value: ", parameter_value_string);
      ml_->MLGetNext();
      tmp_range = state.parameter_map.get_variable(tmp_param);
      tmp_range.add(value_range_t::Element(MathematicaExpressionConverter::convert_math_string_to_symbolic_value(parameter_value_string),
                                           MathematicaExpressionConverter::get_relation_from_code(relop_code)));
      state.parameter_map.set_variable(tmp_param, tmp_range);
      prev_param.name = tmp_param.name;
    }
*/

    // �V�~�����[�V�����I�������ɒB�������ǂ���
    const_tree_iter_t max_time_flag_ptr = next_time_ptr+1;
    // const_tree_iter_t max_time_flag_ptr = next_time_ptr+2;
    int max_time_flag;
    std::stringstream max_time_flag_ss;
    std::string max_time_flag_str = std::string(max_time_flag_ptr->value.begin(), max_time_flag_ptr->value.end());
    max_time_flag_ss << max_time_flag_str;
    max_time_flag_ss >> max_time_flag;
    state.is_max_time = (max_time_flag == 1);
    HYDLA_LOGGER_VCS("is_max_time: ",  state.is_max_time);
    HYDLA_LOGGER_VCS("--parameter map--\n",  state.parameter_map);



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

  return VCSR_TRUE;
}

void REDUCEVCSInterval::send_time(const time_t& time){
  HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", time);
  REDUCEStringSender rss(*cl_);
  rss.put_node(time.get_node(), false);
}

void REDUCEVCSInterval::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("--- apply_time_to_vm ---");

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
      HYDLA_LOGGER_OUTPUT("value : ", value.get_string());
    }

    out_vm.set_variable(it->first, value);
  }
}

void REDUCEVCSInterval::add_undefined_vars_to_vm(variable_map_t& vm)
{
  HYDLA_LOGGER_VCS("--- add undefined vars to vm ---");  

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
}

void REDUCEVCSInterval::send_cs(REDUCEStringSender& rss) const
{
  HYDLA_LOGGER_VCS(
    "---- Send Constraint Store -----\n",
    "cons size: ", constraint_store_.constraints.size());

  cl_->send_string("{");
  constraint_store_t::constraints_t::const_iterator
    cons_it  = constraint_store_.constraints.begin();
  constraint_store_t::constraints_t::const_iterator
    cons_end = constraint_store_.constraints.end();
  for(; (cons_it) != cons_end; ++cons_it) {
    if(cons_it!=constraint_store_.constraints.begin()) cl_->send_string(",");
    rss.put_node(*cons_it, true);
  }
  cl_->send_string("}");
}

std::ostream& REDUCEVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSInterval ***\n"
    << "--- constraint store ---\n";

  s << "-- init_vars --\n";
  constraint_store_t::init_vars_t::const_iterator init_vars_it = constraint_store_.init_vars.begin(),
    init_vars_end = constraint_store_.init_vars.end();
  for(;init_vars_it != init_vars_end; init_vars_it++){
    s << init_vars_it->first << " <=> " << init_vars_it->second << "\n";
  }

  s << "-- constraints --\n";
  constraint_store_t::constraints_t::const_iterator constraints_it = constraint_store_.constraints.begin(),
    constraints_end = constraint_store_.constraints.end();
  for(; constraints_it != constraints_end; constraints_it++){
    s << **constraints_it << "\n";
  }

  return s;
}

std::ostream& operator<<(std::ostream& s,
                         const REDUCEVCSInterval& vcs)
{
  return vcs.dump(s);
}

std::ostream& operator<<(std::ostream& s, 
                         const REDUCEVCSInterval::constraint_store_t& c)
{
  s << "---- REDUCEVCSInterval::consraint_store_t ----\n"
    << "-- init vars --\n";

  BOOST_FOREACH(
    const REDUCEVCSInterval::constraint_store_t::init_vars_t::value_type& i, 
    c.init_vars)
  {
    s << "variable: " << i.first
      << "   value: " << i.second
      << "\n";
  }

  s << "-- constraints --\n";
  BOOST_FOREACH(
    const REDUCEVCSInterval::constraint_store_t::constraints_t::value_type& i, 
    c.constraints)
  {
    s << *i;
  }
  
  return s;
}


} // namespace reduce
} // namespace simulator
} // namespace hydla 

