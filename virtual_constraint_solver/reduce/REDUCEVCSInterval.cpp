#include "REDUCEVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "Logger.h"

using namespace hydla::vcs;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace reduce {

namespace {

struct MaxDiffMapDumper
{
  template<typename T>
  MaxDiffMapDumper(T it, T end)
  {
    for(; it!=end; ++it) {
      s << "name: " << it->first
        << "  diff: " << it->second
        << "\n";      
    }
  }

  std::stringstream s;
};

}


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
    return true;
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

    // �����l����Ɋւ���max_diff_map�ɒǉ�
    constraint_store_t::init_vars_max_diff_map_t::iterator ivmd_it = 
      constraint_store_.init_vars_max_diff_map.find(it->first.name);
    if(ivmd_it==constraint_store_.init_vars_max_diff_map.end()) 
    {
      constraint_store_.init_vars_max_diff_map.insert(
        std::make_pair(it->first.name, it->first.derivative_count));
    }
    else if(it->first.derivative_count > ivmd_it->second)
    {
      ivmd_it->second = it->first.derivative_count;
    }

  }

  HYDLA_LOGGER_VCS(constraint_store_);

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
    return true;
  }
  HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", 
                     parameter_map);

  parameter_map_=parameter_map;

  return true;
}



void REDUCEVCSInterval::create_max_diff_map(
  REDUCEStringSender& rss, max_diff_map_t& max_diff_map)
{
  PacketSender::vars_const_iterator vars_it  = rss.vars_begin();
  PacketSender::vars_const_iterator vars_end = rss.vars_end();
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
//  std::cout << "{";
  bool first_element = true;
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);

    // �����l����̂����A�W�߂�tell����ɏo������ۂ̍ő�����񐔂��������������񐔂̂��̂̂ݑ���
    if(md_it!=max_diff_map.end() &&
       md_it->second  > init_vars_it->first.derivative_count) 
    {
      if(!first_element) cl_->send_string(",");
//      if(!first_element) std::cout << ",";
      cl_->send_string("init");
//      std::cout << "init";
      // �ϐ���
      rss.put_var(boost::make_tuple(init_vars_it->first.name, 
                                    init_vars_it->first.derivative_count, 
                                    false));
//      std::cout << init_vars_it->first.name;
      cl_->send_string("lhs");
//      std::cout << "lhs";

      cl_->send_string("=");
//      std::cout << "=";
  
      // �l
      if(use_approx && approx_precision_ > 0) {
        // �ߎ����đ��M
        // TODO:���Ƃ�����
//        ml_->put_function("approxExpr", 2);
//        ml_->put_integer(approx_precision_);
      }
      rss.put_node(init_vars_it->second.get_node(), false);
//      std::cout << init_vars_it->second.get_string();
      first_element = false;
    }
  }
  cl_->send_string("}");
//  std::cout << "}";

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
  for(md_it=max_diff_map.begin(); md_it!=md_end; ++md_it) {
    for(int i=0; i<=md_it->second; i++) {
      rss.put_var(boost::make_tuple(md_it->first, i, false));

      HYDLA_LOGGER_VCS("put: ",
                       "  name: ", md_it->first,
                       "  diff: ", i);
    }
  }
  cl_->send_string("}");
}

VCSResult REDUCEVCSInterval::add_constraint(const tells_t& collected_tells, const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSInterval::add_constraint ***");

  REDUCEStringSender rss(*cl_);


//////////////////// ���M����

  // isConsistentInterval(expr_, init_, vars_)��n������

  // expr_��n���icollected_tells�Aappended_asks�Aconstraint_store�Aparameter_cons��4���琬��j
  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=union(union(union(");

  // tell����̏W������tells�𓾂�REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send collected_tells ---");
  cl_->send_string("{");
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; (tells_it) != tells_end; ++tells_it) {
    if(tells_it != collected_tells.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", *(*tells_it)->get_child());
    rss.put_node((*tells_it)->get_child(), true);
  }
  cl_->send_string("},");  

  // appended_asks����K�[�h�����𓾂�REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send appended_asks ---");
  cl_->send_string("{");
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    if(append_it != appended_asks.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()),
                     "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_guard(), true, append_it->entailed);
  }
  cl_->send_string("}),");

  // ����X�g�Aconstraints��REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send constraint_store ---");
  send_cs(rss);
  cl_->send_string("),");

  // �p�����^����parameter_cons��REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send parameter_cons ---");
  send_parameter_cons();
  cl_->send_string(");");


  // max_diff_map�����ƂɁA�����l����init_cons��n��
  HYDLA_LOGGER_VCS("----- send init_ -----");  
  cl_->send_string("init_:=");
  max_diff_map_t max_diff_map;
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  create_max_diff_map(rss, max_diff_map);
  // �����l����̑��M
  send_init_cons(rss, max_diff_map, false);
  cl_->send_string(";");


  // vars��n��
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  rss.put_vars(true);
  cl_->send_string(";");


  cl_->send_string("symbolic redeval '(isConsistentInterval expr_ init_ vars_);");


/////////////////// ��M����
  HYDLA_LOGGER_EXTERN("--- receive  ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ",
                   ans);
  
  VCSResult result;


  // S���p�[�T�œǂݎ��
  SExpParser sp;
  sp.parse_main(ans.c_str());

  // {�R�[�h}�̍\��
  const_tree_iter_t ct_it = sp.get_tree_iterator();

  // �R�[�h���擾
  const_tree_iter_t ret_code_it = ct_it->children.begin();
  std::string ret_code_str = std::string(ret_code_it->value.begin(), ret_code_it->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ",
                   ret_code_str);

  if(ret_code_str=="RETERROR___"){
    // �\���o�G���[
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code_str==" \"RETTRUE___\"") {
    // �[��
    // TODO: �X�y�[�X��""���c��Ȃ��悤�Ƀp�[�T���C��
    result = VCSR_TRUE;
    HYDLA_LOGGER_VCS_SUMMARY("consistent");

    HYDLA_LOGGER_VCS( "---build constraint store---");
    constraint_store_.constraints.insert(
      constraint_store_.constraints.end(),
      collected_tells.begin(),
      collected_tells.end());

    // ����X�g�A���Ŏg�p�����ϐ��̈ꗗ�̍X�V
    REDUCEStringSender::vars_const_iterator rss_vars_it  = rss.vars_begin();
    REDUCEStringSender::vars_const_iterator rss_vars_end = rss.vars_end();
    for(; rss_vars_it!=rss_vars_end; ++rss_vars_it) {
      REDUCEVariable rv;
      rv.name             = rss_vars_it->get<0>();
      rv.derivative_count = rss_vars_it->get<1>();
      constraint_store_.cons_vars.insert(rv);
    }
  }
  else {
    // ����G���[
    assert(ret_code_str == " \"RETFALSE___\"");
    result = VCSR_FALSE;
    HYDLA_LOGGER_VCS_SUMMARY("inconsistent");
  }

  HYDLA_LOGGER_VCS(
    constraint_store_,
    "\n#*** End REDUCEVCSInterval::add_constraint ***");

  return result;
}
  
VCSResult REDUCEVCSInterval::check_entailment(const ask_node_sptr& negative_ask, const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS_SUMMARY(
    "#*** REDUCEVCSInterval::check_entailment ***\n", 
    "ask: ", *negative_ask);

  REDUCEStringSender rss(*cl_);


//////////////////// ���M����

  // checkEntailmentInterval(guard_, store_, init_, vars_, pars_)��n������

  // guard_����
  // ask����̃K�[�h�̎��𓾂�REDUCE�ɓn��
  HYDLA_LOGGER_VCS("----- send guard_ -----");
  cl_->send_string("guard_:=");
  rss.put_node(negative_ask->get_guard(), true);

  
  // store_��n���iconstraint_store�Aappended_asks�Aparameter_cons��3���琬��j
  HYDLA_LOGGER_VCS("----- send store_ -----");
  cl_->send_string("store_:=union(union(");

  // ����X�g�Aconstraints��REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send constraint_store ---");
  send_cs(rss);
  cl_->send_string(",");

  // appended_asks����K�[�h�����𓾂�Mathematica�ɓn��
  HYDLA_LOGGER_VCS("--- send appended_asks ---");
  cl_->send_string("{");
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    if(append_it != appended_asks.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()),
                     "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_guard(), true, append_it->entailed);
  }
  cl_->send_string("}),");

  // parameter_cons��n��
  HYDLA_LOGGER_VCS("--- send parameter_cons ---");
  send_parameter_cons();
  cl_->send_string(");");


  // max_diff_map�����ƂɁA�����l����init_cons��n��
  HYDLA_LOGGER_VCS("----- send init_ -----");  
  cl_->send_string("init_:=");
  max_diff_map_t max_diff_map;
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  create_max_diff_map(rss, max_diff_map);
  // �����l����̑��M
  send_init_cons(rss, max_diff_map, false);
  cl_->send_string(";");


  // �ϐ��̃��X�gvars_��n��
  cl_->send_string("vars_:=");
  send_vars(rss, max_diff_map);
  cl_->send_string(";");


  // �L���萔�̃��X�gpars_��n��
  cl_->send_string("pars_:=");
  send_pars();
  cl_->send_string(";");


  cl_->send_string("symbolic redeval '(checkEntailmentInterval guard_ store_ init_ vars_ pars_);");


/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("check_entailment_ans: ",
                   ans);

  VCSResult result;
    
  // S���p�[�T�œǂݎ��
  SExpParser sp;
  sp.parse_main(ans.c_str());

  // {�R�[�h}�̍\��
  const_tree_iter_t ct_it = sp.get_tree_iterator();

  // �R�[�h���擾
  const_tree_iter_t ret_code_it = ct_it->children.begin();
  std::string ret_code_str = std::string(ret_code_it->value.begin(), ret_code_it->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ",
                   ret_code_str);
  
  if(ret_code_str == "(list ccp_solver_error___)"){
    // �\���o�G���[
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code_str == "ccp_entailed___") {
    result = VCSR_TRUE;
    HYDLA_LOGGER_VCS_SUMMARY("entailed");
  }
  else if(ret_code_str == "ccp_not_entailed___") {
    result = VCSR_FALSE;
    HYDLA_LOGGER_VCS_SUMMARY("not entailed");
  }
  else{
    assert(ret_code_str == "(list ccp_unknown___)");
    result = VCSR_UNKNOWN;
  }

  return result;
}

void REDUCEVCSInterval::send_ask_guards(
  REDUCEStringSender& rss, 
  const hydla::simulator::ask_set_t& asks) const
{
  // {�K�[�h�̎��Aask��ID}�̃��X�g�`���ő��M����

  cl_->send_string("{");
  hydla::simulator::ask_set_t::const_iterator it  = asks.begin();
  hydla::simulator::ask_set_t::const_iterator end = asks.end();
  for(; it!=end; ++it)
  {
    if(it!=asks.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("send ask : ", **it);
    cl_->send_string("{");

    // guard�����𑗂�
    rss.put_node((*it)->get_guard(), true);

    cl_->send_string(",");

    // ID�𑗂�
    std::stringstream id_ss;
    int ask_id = (*it)->get_id();
    id_ss << ask_id;
    cl_->send_string(id_ss.str());

    cl_->send_string("}");
  }
  cl_->send_string("}");
}

VCSResult REDUCEVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const positive_asks_t& positive_asks,
  const negative_asks_t& negative_asks,
  const time_t& current_time,
  const time_t& max_time,
  const not_adopted_tells_list_t& not_adopted_tells_list,
  const appended_asks_t& appended_asks)
{
  HYDLA_LOGGER_VCS("#*** REDUCEVCSInterval::integrate ***");

  HYDLA_LOGGER_VCS(constraint_store_);

/////////////////// ���M����
  REDUCEStringSender rss(*cl_);

  // integrateCalc(cons_, init_, posAsk_, negAsk_, NACons_, vars_, maxTime_]��n������

  // cons_��n���iconstraint_store�Aappended_asks�Aparameter_cons��3���琬��j
  HYDLA_LOGGER_VCS("----- send cons_ -----");
  cl_->send_string("cons_:=union(union(");

  // ����X�g�Aconstraints��REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send constraint_store ---");
  send_cs(rss);
  cl_->send_string(",");

  // appended_asks����K�[�h�����𓾂�Mathematica�ɓn��
  HYDLA_LOGGER_VCS("--- send appended_asks ---");
  cl_->send_string("{");
  appended_asks_t::const_iterator append_it  = appended_asks.begin();
  appended_asks_t::const_iterator append_end = appended_asks.end();
  for(; append_it!=append_end; ++append_it) {
    if(append_it != appended_asks.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node (guard): ", *(append_it->ask->get_guard()),
                     "  entailed:", append_it->entailed);
    rss.put_node(append_it->ask->get_guard(), true, append_it->entailed);
  }
  cl_->send_string("}),");

  // parameter_cons��n��
  HYDLA_LOGGER_VCS("--- send parameter_cons ---");
  send_parameter_cons();
  cl_->send_string(");");


  // max_diff_map�����ƂɁA�����l����init_cons��n��
  HYDLA_LOGGER_VCS("----- send init_ -----");  
  cl_->send_string("init_:=");
  max_diff_map_t max_diff_map;
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  create_max_diff_map(rss, max_diff_map);
  // �����l����̑��M
  send_init_cons(rss, max_diff_map, false);
  cl_->send_string(";");


  // posAsk��n���i{�K�[�h�̎��Aask��ID}�����ꂼ��j
  HYDLA_LOGGER_VCS("----- send posAsk_ -----");
  cl_->send_string("posAsk_:=");
  send_ask_guards(rss, positive_asks);
  cl_->send_string(";");


  // negAsk��n���i{�K�[�h�̎��Aask��ID}�����ꂼ��j
  HYDLA_LOGGER_VCS("----- send negAsk_ -----");
  cl_->send_string("negAsk_:=");
  send_ask_guards(rss, negative_asks);
  cl_->send_string(";");


  // �̗p���Ă��Ȃ����W���[������tell����NACons��n���i{{���AID}, {���AID}, ...}�����ꂼ��̃��W���[���Ɋւ��āj
  HYDLA_LOGGER_VCS("----- send NACons_ -----");
  cl_->send_string("NACons_:=");
  send_not_adopted_tells(rss, not_adopted_tells_list);
  cl_->send_string(";");


  // �ϐ��̃��X�g��n��
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  send_vars(rss, max_diff_map);
  cl_->send_string(";");


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
  cl_->send_string(";");


  cl_->send_string("symbolic redeval '(integrateCalc cons_ init_ posAsk_ negAsk_ NACons_ vars_ maxTime_);");


/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  cl_->read_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("integrate_ans: ",
                   ans);
  











/////////////////// ��M�I��

  assert(0);

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

  assert(0);

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

void REDUCEVCSInterval::send_not_adopted_tells(REDUCEStringSender& rss, const not_adopted_tells_list_t& na_tells_list) const
{
  // {tell�̎��Atell��ID}�̃��X�g�̃��X�g�`���ő��M����
  HYDLA_LOGGER_VCS("----- send not adopted constraint -----");

  // �̗p���Ă��Ȃ����W���[��
  cl_->send_string("{");
  not_adopted_tells_list_t::const_iterator na_tells_list_it = na_tells_list.begin();
  not_adopted_tells_list_t::const_iterator na_tells_list_end = na_tells_list.end();
  for(; na_tells_list_it!=na_tells_list_end; ++na_tells_list_it){
    if(na_tells_list_it!=na_tells_list.begin()) cl_->send_string(",");
    cl_->send_string("{");
    // �̗p���Ă��Ȃ��A���郂�W���[������tell����
    tells_t::const_iterator na_tells_it  = (*na_tells_list_it).begin();
    tells_t::const_iterator na_tells_end = (*na_tells_list_it).end();
    for(; na_tells_it!=na_tells_end; ++na_tells_it) {
      if(na_tells_it!=(*na_tells_list_it).begin()) cl_->send_string(",");
      HYDLA_LOGGER_VCS("send not adopted tell : ", **na_tells_it);
      cl_->send_string("{");

      // tell����𑗂�
      rss.put_node((*na_tells_it)->get_child(), true);

      cl_->send_string(",");

      // ID�𑗂�
      std::stringstream id_ss;
      int tell_id = (*na_tells_it)->get_id();
      id_ss << tell_id;
      cl_->send_string(id_ss.str());

      cl_->send_string("}");      
    }
    cl_->send_string("}");
  }
  cl_->send_string("}");
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
    rss.put_node((*cons_it)->get_child(), true);
  }
  cl_->send_string("}");
}

void REDUCEVCSInterval::send_cs_vars() const
{
  HYDLA_LOGGER_VCS("---- Send Constraint Store Vars -----");
}

std::ostream& REDUCEVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSInterval ***\n"
    << "--- constraint store ---\n";


  return s;
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

  s << "-- init vars max diff map --\n";
  BOOST_FOREACH(
    const REDUCEVCSInterval::constraint_store_t::init_vars_max_diff_map_t::value_type& i, 
    c.init_vars_max_diff_map)
  {
    s << "name: " << i.first
      << "  diff: " << i.second
      << "\n";
  }
  
  return s;
}


} // namespace reduce
} // namespace simulator
} // namespace hydla 

