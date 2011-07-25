
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
             << sc.convert_symbolic_value_to_string(value)
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

  cl_->send_string("vmStr_:=");
  cl_->send_string(vm_str.str());
  cl_->send_string(";");
  cl_->send_string("symbolic redeval '(getVMFromString vmStr_);");


//  cl_->read_until_redeval();
  cl_->skip_until_redeval();

  std::string vm_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("vm_s_exp_str: ", vm_s_exp_str);
  
  // sp_��vm_s_exp_str��ǂݍ��܂��āAS���̃p�[�X�c���[���\�z���A�擪�̃|�C���^�𓾂�
  sp_.parse_main(vm_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();
  constraint_store_.first.insert(tree_root_ptr);


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

  for(size_t i = 0; i < or_size; i++){
    const_tree_iter_t or_it = (*constraint_store_.first.begin())+i; 
    create_result_t::maps_t maps;
    variable_t symbolic_variable;
    value_t symbolic_value;
    size_t and_size = or_it->children.size();
    HYDLA_LOGGER_VCS("and_size: ", and_size);  
    for(size_t j = 0; j < and_size; j++){
      const_tree_iter_t and_it = or_it->children.begin()+j;

      std::string and_cons_string =  sp_.get_string_from_tree(and_it);
      HYDLA_LOGGER_VCS("and_cons_string: ", and_cons_string);


      // �ϐ���
      const_tree_iter_t var_ptr = and_it->children.begin();
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
      const_tree_iter_t value_ptr = and_it->children.begin()+1;
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

VCSResult REDUCEVCSPoint::add_constraint(const tells_t& collected_tells, const appended_asks_t &appended_asks)
{

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::add_constraint ***");
  REDUCEStringSender rss = REDUCEStringSender(*cl_);


//////////////////// ���M����

  // send_string��string�͂ǂ̂悤�ɋ�؂��đ��M���Ă�OK
  //   ex) cl_->send_string("expr_:={df(y,t,2) = -10,");
  //       cl_->send_string("y = 10, df(y,t,1) = 0, prev(y) = y, df(prev(y),t,1) = df(y,t,1)};");

  // isConsistent(vars_,pexpr_,expr_)��n������
  // TODO:�����̑��鏇�ԂƑΉ�����������������₷������

  // expr_��n���icollected_tells�Aappended_asks�Aconstraint_store�Aleft_continuity��4���琬��j
  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=union(union(union(");

  // tell����̏W������expr�𓾂�REDUCE�ɓn��
  HYDLA_LOGGER_VCS("--- send collected_tells ---");
  cl_->send_string("{");
  tells_t::const_iterator tells_it  = collected_tells.begin();
  tells_t::const_iterator tells_end = collected_tells.end();
  for(; tells_it!=tells_end; ++tells_it) {
    if(tells_it != collected_tells.begin()) cl_->send_string(",");
    HYDLA_LOGGER_VCS("put node: ", *(*tells_it)->get_child());
    rss.put_node((*tells_it)->get_child());
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
    rss.put_node(append_it->ask->get_child());
  }
  cl_->send_string("}),");

  // ����X�g�A������n��
  HYDLA_LOGGER_VCS("--- send constraint_store ---");
  send_cs();
  cl_->send_string("),");

  // ���A�����Ɋւ��鐧���n��
  // ���ݍ̗p���Ă��鐧��ɏo������ϐ��̍ő�����񐔂��������������񐔂̂��̂ɂ���prev(x)=x�ǉ�
  HYDLA_LOGGER_VCS("--- send left_continuity ---");
  max_diff_map_t max_diff_map;
  create_max_diff_map(rss, max_diff_map);
  add_left_continuity_constraint(rss, max_diff_map);
  cl_->send_string(");");


  // pexpr��n��
  HYDLA_LOGGER_VCS("----- send pexpr_ -----");
  cl_->send_string("pexpr_:=");
  send_ps();
  cl_->send_string(";");


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


  cl_->send_string("symbolic redeval '(isconsistent vars_ pexpr_ expr_);");


/////////////////// ��M����
  HYDLA_LOGGER_VCS("--- receive ---");

//  cl_->read_until_redeval();
  cl_->skip_until_redeval();

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
    HYDLA_LOGGER_VCS_SUMMARY("consistent");
    HYDLA_LOGGER_VCS( "---build constraint store---");

    // ����X�g�A�����Z�b�g
    //    reset();
    constraint_store_.first.clear();

    // ����X�g�A�\�z
    // ����Ԃ�Or�Ɋւ��Ă�set�ŕێ�
    // TODO:�o�����vector������ň������������̏ꍇreset������ł��������K�؂ȏ������K�v��
    const_tree_iter_t cons_list_ptr = ret_code_ptr+1;
    size_t or_size = cons_list_ptr->children.size();
    HYDLA_LOGGER_VCS( "or_size: ", or_size);

    for(size_t i=0; i<or_size; i++)
    {
      const_tree_iter_t or_cons_it = cons_list_ptr->children.begin()+i;
      constraint_store_.first.insert(or_cons_it);
    }

    constraint_store_.second.insert(rss.vars_begin(), rss.vars_end()); 
  }
  else {
    assert(ret_code_str == " \"RETFALSE___\"");
    HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//����

    // ����X�g�A�����Z�b�g
    //    reset();
    constraint_store_.first.clear();

    result = VCSR_FALSE;
  }

  HYDLA_LOGGER_VCS(
    *this,
    "\n#*** End REDUCEVCSPoint::add_constraint ***");

  return result;

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


VCSResult REDUCEVCSPoint::integrate(
    integrate_result_t& integrate_result,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time,
    const not_adopted_tells_list_t& not_adopted_tells_list,
    const appended_asks_t& appended_asks)
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

  std::set<const_tree_iter_t>::const_iterator or_cons_it = constraint_store_.first.begin();
  std::set<const_tree_iter_t>::const_iterator or_cons_end = constraint_store_.first.end();
  for(; or_cons_it!=or_cons_end; or_cons_it++){

    std::string or_string = sp_.get_string_from_tree(*or_cons_it);
    HYDLA_LOGGER_VCS("or_string: ", or_string);

    // ������"list"�݂̂ł���Ƃ��A��W�����Ӗ�����
    if (or_string == hydla::parser::SExpParser::empty_list_s_exp) cl_->send_string("{}");
    else cl_->send_string(or_string);
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
    if(it!=constraint_store_.second.begin())   cl_->send_string(",");
    rss.put_var(*it);
  }
  cl_->send_string("}");

}

std::ostream& REDUCEVCSPoint::dump(std::ostream& s) const
{
  s << "#*** Dump REDUCEVCSPoint ***\n"
      << "--- constraint store ---\n";

  std::set<const_tree_iter_t>::const_iterator or_cons_it =
      constraint_store_.first.begin();
  while((or_cons_it) != constraint_store_.first.end())
  {
    s << sp_.get_string_from_tree(*or_cons_it) << "\n";
    or_cons_it++;
  }

  // ����X�g�A���ɑ��݂���ϐ��̃_���v
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

std::ostream& operator<<(std::ostream& s, const REDUCEVCSPoint& r)
{
  return r.dump(s);
}


} // namespace reduce
} // namespace simulator
} // namespace hydla

