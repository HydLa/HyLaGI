
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
  // TODO: �`���C�l����
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

  // �ϐ��\�̒��g��\���悤��S���̕�����vm_s_exp_str�𓾂���
  // �܂��́AREDUCE�֑��镶����vm_str���쐬����
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

      // variable����
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

      // value����
      vm_str << ", "
             << sc.convert_symbolic_value_to_reduce_string(value)
             << ")"; // equal�̕�����


      // ����X�g�A���̕ϐ��ꗗ��variable��ǉ�
      constraint_store_.second.insert(
        boost::make_tuple(variable.name,
                          variable.derivative_count,
                          true));
      first_element = false;
    }
  }

  vm_str << "}"; // list�̕�����
  HYDLA_LOGGER_VCS("vm_str: ", vm_str.str());  

  cl_->send_string("str_:=");
  cl_->send_string(vm_str.str());
  cl_->send_string(";");
  cl_->send_string("symbolic redeval '(getSExpFromString str_);");


  cl_->read_until_redeval();
//  cl_->skip_until_redeval();

  std::string vm_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("vm_s_exp_str: ", vm_s_exp_str);
  
  // sp_��vm_s_exp_str��ǂݍ��܂��āAS���̃p�[�X�c���[���\�z���A�擪�̃|�C���^�𓾂�
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

          // REDUCEVariable���Ɋւ��镶������쐬
          val_str << MathematicaExpressionConverter::get_relation_math_string(and_it->relation) << "[" ;

          val_str << PacketSender::par_prefix
                  << it->first.get_name();

          val_str << ","
                  << and_it->value
                  << "]"; // ������
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

  // TODO: �s�����y�ыL���萔�ւ̑Ή�

  HYDLA_LOGGER_VCS(
    "#*** REDUCEVCSPoint::create_variable_map ***\n",
    "--- constraint_store ---\n",
    *this);

  size_t or_size = constraint_store_.first.size();
  HYDLA_LOGGER_VCS("or_size: ", or_size);
  // TODO: �������ɑΉ�
  assert(or_size==1);

  // S���p�[�T��p���āA����X�g�A�S�̂�\���悤�Ȗ؍\�����Ăѓ���
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


  // TODO:�ȉ��̃R�[�h��or_size==1���O��
  {
    create_result_t::maps_t maps;
    variable_t symbolic_variable;
    value_t symbolic_value;
    size_t and_cons_size = tree_root_ptr->children.size();

    for(size_t i=0; i<and_cons_size; i++){
      const_tree_iter_t and_ptr = tree_root_ptr->children.begin()+i;

      std::string and_cons_string =  sp_.get_string_from_tree(and_ptr);
      HYDLA_LOGGER_VCS("and_cons_string: ", and_cons_string);


      // �ϐ���
      const_tree_iter_t var_ptr = and_ptr->children.begin();
      std::string var_head_str = std::string(var_ptr->value.begin(),var_ptr->value.end());

      // prev�ϐ��͏������Ȃ�
      if(var_head_str=="prev") continue;

      std::string var_name;
      int var_derivative_count = sp_.get_derivative_count(var_ptr);

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

      symbolic_variable.name = var_name;
      symbolic_variable.derivative_count = var_derivative_count;

      // �l��
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
  // ����X�g�A���̕ϐ��̂����A�W�߂�tell����ɏo������ő�����񐔂�菬���������񐔂ł�����̂̂ݒǉ�
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
      // Prev�ϐ���
      // �ϐ���
      rss.put_var(
        boost::make_tuple(cs_vars_it->get<0>(),
                          cs_vars_it->get<1>(),
                          true));

      cl_->send_string("=");

      // Now�ϐ���
      // �ϐ���
      rss.put_var(
        boost::make_tuple(cs_vars_it->get<0>(),
                          cs_vars_it->get<1>(),
                          false));
      first_element = false;
    }
  }
  cl_->send_string("},{");


  // �W�߂�tell������̕ϐ��ɂ��Ă����ׂ�
  // ����0�i����X�g�A����j�΍�̂���
  HYDLA_LOGGER_VCS("--- in vars ---");

  // max_diff_map�ɂ��Ă���
  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();
  first_element = true;
  for(; md_it!=md_end; ++md_it) {
    if(constraint_store_.second.find(md_it->first)==constraint_store_.second.end()){
      for(int i=0; i<md_it->second; ++i){
        if(!first_element) cl_->send_string(",");
        // Prev�ϐ���
        // �ϐ���
        rss.put_var(
          boost::make_tuple(md_it->first,
                            i,
                            true));

        cl_->send_string("=");

        // Now�ϐ���
        // �ϐ���
        rss.put_var(
          boost::make_tuple(md_it->first,
                            i,
                            false));

        // ����X�g�A���̕ϐ���������
        // �������Ȃ��ƌ��vars_�𑗂�ۂ�prev�ϐ��B�𑗂�Ȃ��i����0��PP�݂̂ł̘b�j
        // TODO:�v����
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
  // TODO: ������������̂�������Ȃ̂ŁA�����񉻂��ăX�g�A�ɓ���邱�Ƃ���������H
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

  // sp_��new_cs_s_exp_str��ǂݍ��܂��āAS���̃p�[�X�c���[���\�z���A�擪�̃|�C���^�𓾂�
  // TODO: Or�ւ̑Ή��H
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

  // TODO: Or�ւ̑Ή��H
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


/////////////////// ���M����

  // checkentailment(guard_, store_, vars_)��n������

  HYDLA_LOGGER_VCS("----- send guard_ -----");
  // ask����̃K�[�h�̎��𓾂�Mathematica�ɓn��
  cl_->send_string("guard_:=");
  rss.put_node(negative_ask->get_guard());
  cl_->send_string(";");  


  // ����X�g�A�ƃp�����^�X�g�A���玮�𓾂�Mathematica�ɓn��
  HYDLA_LOGGER_VCS("----- send store_ -----");
  cl_->send_string("store_:=union(");
  send_cs();
  cl_->send_string(",");
  send_ps();
  cl_->send_string(");");


  // vars��n��
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=union(union(");

  // collected_tells���̕ϐ��ꗗ��n��
  rss.put_vars();
  cl_->send_string(",");

  // ����X�g�A���ɏo������ϐ��ꗗ���n��
  send_cs_vars();
  cl_->send_string("),");

  // �p�����^�ꗗ���n��
  send_pars();
  cl_->send_string(");");


  cl_->send_string("symbolic redeval '(checkentailment guard_ store_ vars_);");


/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

//  cl_->read_until_redeval();
  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("check_entailment_ans: ",
                   ans);

  VCSResult result;
  
  // TODO:S���p�[�T���g�����H
/*  
  // S���p�[�T�œǂݎ��
  sp_.parse_main(ans.c_str());

  // {�R�[�h}�̍\��
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();

  // �R�[�h���擾
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ",
                   ret_code_str);
*/


//  HYDLA_LOGGER_VCS(*this);
//  sp_.dump_tree(*(constraint_store_.first.begin()), 0);
  
  if(ans == "(list ccp_solver_error___)"){
    // �\���o�G���[
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
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//����
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
      // ����X�g�A�����Z�b�g
      //    reset();
      constraint_store_.first.clear();

      receive_constraint_store(constraint_store_);
    }
    break;
    case VCSR_FALSE:
    {
      HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//����
      // ����X�g�A�����Z�b�g
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
  // ����X�g�A�\�z
  // ����Ԃ�Or�Ɋւ��Ă�set�ŕێ�
  // TODO:�o�����vector������ň������������̏ꍇreset������ł��������K�؂ȏ������K�v��
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


//////////////////// ���M����

  // send_string��string�͂ǂ̂悤�ɋ�؂��đ��M���Ă�OK
  //   ex) cl_->send_string("expr_:={df(y,t,2) = -10,");
  //       cl_->send_string("y = 10, df(y,t,1) = 0, prev(y) = y, df(prev(y),t,1) = df(y,t,1)};");

  // isConsistent(expr_,vars_)��n������

  // expr_��n���itmp_constraints�Aconstraint_store�Aparameter_store�Aleft_continuity��4���琬��j
  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=union(union(union(");

  // �ꎞ�I�Ȑ���X�g�A(�V�����ǉ����ꂽ����̏W��)����expr�𓾂�REDUCE�ɓn��
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

  // ����X�g�A������n��
  HYDLA_LOGGER_VCS("--- send constraint_store ---");
  send_cs();
  cl_->send_string("),");

  // �p�����^�X�g�A��n��
  HYDLA_LOGGER_VCS("--- send parameter_store ---");
  send_ps();
  cl_->send_string("),");

  // ���A�����Ɋւ��鐧���n��
  // ���ݍ̗p���Ă��鐧��ɏo������ϐ��̍ő�����񐔂��������������񐔂̂��̂ɂ���prev(x)=x�ǉ�
  HYDLA_LOGGER_VCS("--- send left_continuity ---");
  max_diff_map_t tmp_max_diff_map = max_diff_map_;
  rss.create_max_diff_map(tmp_max_diff_map);
  add_left_continuity_constraint(rss, tmp_max_diff_map);
  cl_->send_string(");");


  // vars��n��
  //   ex) {y, prev(y), df(y,t,1), df(prev(y),t,1), df(y,t,2), y, prev(y), df(y,t,1), df(prev(y),t,1)}
  // vars_�Ɋւ��Ĉ�ԊO����"{}"�����́Aput_vars���ő����Ă���
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=union(");
  rss.put_vars();
  cl_->send_string(",");
  // ����X�g�A���ɏo������ϐ����n��
  send_cs_vars();
  cl_->send_string(");");


  cl_->send_string("symbolic redeval '(isconsistent expr_ vars_);");


/////////////////// ��M����
  HYDLA_LOGGER_VCS("--- receive ---");

  cl_->read_until_redeval();
//  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ",
                   ans);

  VCSResult result;

  // S���p�[�T�œǂݎ��
  sp_.parse_main(ans.c_str());

  // {�R�[�h, {{{�ϐ�, �֌W���Z�q�R�[�h, �l},...}, ...}}�̍\��
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();

  // �R�[�h���擾
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
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
  // Point�ł�integrate�֐�����
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

  // TODO: �������ior_cons_size>1�j�̏ꍇ�̑Ώ����l����
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

  // TODO: �����Ƒ���
  cl_->send_string("{}");
}

//TODO �萔�Ԃ��̏C��
void REDUCEVCSPoint::send_pars() const{
  // TODO: �����Ƒ���
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

  // ����X�g�A���ɑ��݂���ϐ��̃_���v
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

