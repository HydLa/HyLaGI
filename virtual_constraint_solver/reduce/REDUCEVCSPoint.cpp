
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

bool REDUCEVCSPoint::create_maps(create_result_t & create_result)
{

  // TODO: �s�����y�ыL���萔�ւ̑Ή�

  HYDLA_LOGGER_VCS("#*** REDUCEVCSPoint::create_variable_map ***\n");

  REDUCEStringSender rss(*cl_);

  /////////////////// ���M����

  // expr_��n��
  HYDLA_LOGGER_VCS("----- send expr_ -----");
  cl_->send_string("expr_:=");
  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string("$");

  // vars_��n��
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  rss.put_vars();
  cl_->send_string("$");


  cl_->send_string("symbolic redeval '(addConstraint expr_ vars_);");
  // cl_->read_until_redeval();
  cl_->skip_until_redeval();


  cl_->send_string("symbolic redeval '(checkConsistency);");
  // cl_->read_until_redeval();
  cl_->skip_until_redeval();


  // TODO:�{����exprCode�t���̌`���ŕԂ��֐����g���悤�ɂ�����
  // cl_->send_string("symbolic redeval '(convertCSToVM);");
  cl_->send_string("symbolic redeval '(returnCS);");


  /////////////////// ��M����                     

  // cl_->read_until_redeval();
  cl_->skip_until_redeval();

  // S���p�[�T��p���āA����X�g�A�S�̂�\���悤�Ȗ؍\���𓾂�
  std::string cs_s_exp_str = cl_->get_s_expr();
  HYDLA_LOGGER_VCS("cs_s_exp_str: ", cs_s_exp_str);
  sp_.parse_main(cs_s_exp_str.c_str());
  const_tree_iter_t tree_root_ptr = sp_.get_tree_iterator();


  // TODO:�ȉ��̃R�[�h��or_size==1���O��
  // TODO:�s�����ւ̑Ή�(exprCode���g��)
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

      // "usrVar"����菜��
      assert(var_name.find(REDUCEStringSender::var_prefix, 0) == 0);
      var_name.erase(0, REDUCEStringSender::var_prefix.length());

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

      // Prev�ϐ���
      // �ϐ���
      rss.put_var(boost::make_tuple(cm_it->first, i, true));

      cl_->send_string("=");

      // Now�ϐ���
      // �ϐ���
      rss.put_var(boost::make_tuple(cm_it->first, i, false));

      first_element = false;
    }
  }
  cl_->send_string("}");  

}

void REDUCEVCSPoint::send_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS(
                   "#*** Begin REDUCEVCSPoint::send_constraint ***");

  REDUCEStringSender rss(*cl_);

  // �����n��
  HYDLA_LOGGER_VCS("----- send expr_ -----");
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
  HYDLA_LOGGER_VCS("----- send lcont_ -----");
  cl_->send_string("lcont_:=");
  add_left_continuity_constraint(continuity_map_, rss);
  cl_->send_string("$");


  // vars��n��
  HYDLA_LOGGER_VCS("----- send vars_ -----");
  cl_->send_string("vars_:=");
  rss.put_vars();
  cl_->send_string("$");

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
  HYDLA_LOGGER_VCS(
    "#*** Begin REDUCEVCSPoint::check_consistency() ***");

  send_constraint(constraints_t());

  cl_->send_string("symbolic redeval '(checkConsistencyWithTmpCons expr_ lcont_ vars_);");

  VCSResult result = check_consistency_receive();

  HYDLA_LOGGER_VCS("\n#*** End REDUCEVCSPoint::check_consistency() ***");
  return result;
}

VCSResult REDUCEVCSPoint::check_consistency_receive()
{
  /////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  // cl_->read_until_redeval();
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
  else if(ret_code_str=="1") {
    // �[��
    // TODO: �X�y�[�X��""���c��Ȃ��悤�Ƀp�[�T���C��
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
  // Point�ł�integrate�֐�����
  assert(0);
  return VCSR_FALSE;
}

} // namespace reduce
} // namespace simulator
} // namespace hydla

