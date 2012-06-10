
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

  // TODO: �s�����y�ыL���萔�ւ̑Ή�

  HYDLA_LOGGER_VCS("#*** Begin REDUCEVCSPoint::create_variable_map ***");

  REDUCEStringSender rss(*cl_);

  /////////////////// ���M����

  // expr_��n��
  HYDLA_LOGGER_VCS("--- send expr_ ---");
  cl_->send_string("expr_:=");
  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string("$");

  // vars_��n��
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


  /////////////////// ��M����                     

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

  // S���p�[�T��p���āA����X�g�A�S�̂�\���悤�Ȗ؍\���𓾂�
  std::string cs_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("cs_s_exp_str: ", cs_s_exp_str);
  sp_.parse_main(cs_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();


  // TODO:�ȉ��̃R�[�h��or_size==1���O��
  //  for(int or_it = 0; or_it < or_size; or_it++){
  {
    create_result_t::maps_t maps;
    std::set<variable_t> p_added_variables;  //�u����L���萔��ǉ����ꂽ�ϐ��v�̈ꗗ
    variable_t symbolic_variable;
    value_t symbolic_value;
    value_range_t tmp_range;
    SExpConverter::clear_parameter_map();
    size_t and_cons_size = tree_root_ptr->children.size();

    for(size_t i=0; i<and_cons_size; i++){
      const_tree_iter_t and_ptr = tree_root_ptr->children.begin()+i;

      std::string and_cons_string =  sp_.get_string_from_tree(and_ptr);
      HYDLA_LOGGER_VCS("and_cons_string: ", and_cons_string);


      // �ϐ���
      const_tree_iter_t var_ptr = and_ptr->children.begin();
      std::string var_head_str = std::string(var_ptr->value.begin(),var_ptr->value.end());

      // �֌W���Z�q�̃R�[�h
      const_tree_iter_t relop_code_ptr = and_ptr->children.begin()+1;      
      std::string relop_code_str = std::string(relop_code_ptr->value.begin(),relop_code_ptr->value.end());
      std::stringstream relop_code_ss;
      int relop_code;
      relop_code_ss << relop_code_str;
      relop_code_ss >> relop_code;
      assert(relop_code>=0 && relop_code<=4);

      // �l
      const_tree_iter_t value_ptr = and_ptr->children.begin()+2;


      // prev�̐擪�ɃX�y�[�X�����邱�Ƃ�����̂ŏ�������
      // TODO:S���p�[�T���C�����ăX�y�[�X����Ȃ��悤�ɂ���
      if(var_head_str.at(0) == ' ') var_head_str.erase(0,1);

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

      // �����̋L���萔�̏ꍇ
      if(var_name.find(REDUCEStringSender::var_prefix, 0) != 0){
	// 'p'�͎���Ă����K�v������
	assert(var_name.at(0) == REDUCEStringSender::par_prefix.at(0));
	var_name.erase(0, 1);
        parameter_t tmp_param(parameter_t::get_variable(var_name));
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        value_t tmp_value = SExpConverter::convert_s_exp_to_symbolic_value(sp_, value_ptr);
        SExpConverter::set_range(tmp_value, tmp_range, relop_code);
        maps.parameter_map.set_variable(tmp_param, tmp_range);
        continue;
      }

      // "usrVar"����菜��
      assert(var_name.find(REDUCEStringSender::var_prefix, 0) == 0);
      var_name.erase(0, REDUCEStringSender::var_prefix.length());

      // �啶���������\�L�ɕϊ�
      VariableNameEncoder vne;
      var_name = vne.UpperDecode(var_name);

      symbolic_variable.name = var_name;
      symbolic_variable.derivative_count = var_derivative_count;



      // �֌W���Z�q�R�[�h�����ɁA�ϐ��\�̑Ή����镔���ɑ������
      if(!relop_code){
        //����
        symbolic_value = SExpConverter::convert_s_exp_to_symbolic_value(sp_, value_ptr);
        symbolic_value.set_unique(true);
      }else{
        //�s�����D���̕ϐ��̒l�͈̔͂�\�����邽�߂̋L���萔���쐬
        value_t tmp_value = SExpConverter::convert_s_exp_to_symbolic_value(sp_, value_ptr);
	if(p_added_variables.find(symbolic_variable) == p_added_variables.end()){
	  //����ǉ����ꂽ�L���萔�Ɋ܂܂�Ȃ��ꍇ�̂݁C�V�K�쐬
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

      // Prev�ϐ���
      // �ϐ���
      rss.put_var(boost::make_tuple(cm_it->first, i, true, false));

      cl_->send_string("=");

      // Now�ϐ���
      // �ϐ���
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

  // �����n��
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


  // ���A���������n��
  HYDLA_LOGGER_VCS("--- send lcont_ ---");
  cl_->send_string("lcont_:=");
  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string("$");


  // vars��n��
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
  /////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

  std::string ans = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("add_constraint_ans: ", ans);

  VCSResult result;

  // S���p�[�T�œǂݎ��
  sp_.parse_main(ans.c_str());

  // {�R�[�h, {{{�ϐ�, �֌W���Z�q�R�[�h, �l},...}, ...}}�̍\��
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();

  // �R�[�h���擾
  const_tree_iter_t ret_code_ptr = tree_root_ptr->children.begin();
  std::string ret_code_str = std::string(ret_code_ptr->value.begin(), ret_code_ptr->value.end());
  HYDLA_LOGGER_VCS("ret_code_str: ", ret_code_str);

  if(ret_code_str=="RETERROR___"){
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
    assert(ret_code_str == "2");
    result = VCSR_FALSE;
  }

  HYDLA_LOGGER_VCS("#*** End REDUCEVCSPoint::check_consistency_receive ***");

  return result;  
}

} // namespace reduce
} // namespace simulator
} // namespace hydla

