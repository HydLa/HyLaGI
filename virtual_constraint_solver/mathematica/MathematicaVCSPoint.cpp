#include "MathematicaVCSPoint.h"

#include <cassert>

#include <boost/algorithm/string/predicate.hpp>

#include <sstream>

#include "mathlink_helper.h"
#include "PacketErrorHandler.h"
#include "Logger.h"
#include "PacketChecker.h"
#include "MathematicaExpressionConverter.h"
#include "SolveError.h"

using namespace hydla::vcs;
using namespace hydla::logger;
using namespace hydla::parser;


namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSPoint::MathematicaVCSPoint(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSPoint::~MathematicaVCSPoint()
{}

bool MathematicaVCSPoint::create_maps(create_result_t& create_result)
{
  HYDLA_LOGGER_VCS(
    "#*** MathematicaVCSPoint::create_variable_map ***\n");
    
/////////////////// ���M����

  PacketSender ps(*ml_);
  ml_->put_function("addConstraint", 2);
  add_left_continuity_constraint(continuity_map_, ps);
  ps.put_vars(PacketSender::VA_None);
  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLNewPacket();
  ml_->put_function("checkConsistency", 0);
  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLNewPacket();
  ml_->put_function("convertCSToVM", 0);

/////////////////// ��M����                        

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  ml_->skip_pkt_until(RETURNPKT);

  ml_->MLGetNext();

  // List�֐��̗v�f���i���̌��j�𓾂�
  int or_size = ml_->get_arg_count();
  HYDLA_LOGGER_VCS("or_size: ", or_size);
  ml_->MLGetNext();
  for(int or_it = 0; or_it < or_size; or_it++){
    create_result_t::maps_t maps;
    std::set<variable_t> p_added_variables;  //�u����L���萔��ǉ����ꂽ�ϐ��v�̈ꗗ
    variable_t symbolic_variable;
    value_t symbolic_value;
    ml_->MLGetNext();
    int and_size = ml_->get_arg_count();
    HYDLA_LOGGER_VCS("and_size: ", and_size);
    ml_->MLGetNext(); // List�Ƃ����֐���
    MathematicaExpressionConverter::clear_parameter_map();
    for(int i = 0; i < and_size; i++)
    {
      value_range_t tmp_range;
      ml_->MLGetNext();
      ml_->MLGetNext();
      
      // �ϐ����i���O�A�����񐔁Aprev�j
      ml_->MLGetNext();
      ml_->MLGetNext();
      ml_->MLGetNext(); // ?
      std::string variable_name = ml_->get_string();
      int variable_derivative_count = ml_->get_integer();
      int prev = ml_->get_integer();

      // �֌W���Z�q�̃R�[�h
      int relop_code = ml_->get_integer();
      // �l
      std::string value_str = ml_->get_string();

      // prev�ϐ��͏������Ȃ�
      if(prev==1) continue;
      
      symbolic_variable.name = variable_name;
      symbolic_variable.derivative_count = variable_derivative_count;

      if(prev==-1){//�����̋L���萔�̏ꍇ
        parameter_t tmp_param(parameter_t::get_variable(symbolic_variable.name));
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        value_t tmp_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);
        MathematicaExpressionConverter::set_range(tmp_value, tmp_range, relop_code);
        maps.parameter_map.set_variable(tmp_param, tmp_range);
        continue;
      }
      // �֌W���Z�q�R�[�h�����ɁA�ϐ��\�̑Ή����镔���ɑ������
      else if(!relop_code){
        //����
        symbolic_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);
        symbolic_value.set_unique(true);
      }else{
        //�s�����D���̕ϐ��̒l�͈̔͂�\�����邽�߂̋L���萔���쐬
        value_t tmp_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);
        if(p_added_variables.find(symbolic_variable) == p_added_variables.end()){
          //����ǉ����ꂽ�L���萔�Ɋ܂܂�Ȃ��ꍇ�̂݁C�V�K�쐬
          parameter_t::increment_id(symbolic_variable);
          p_added_variables.insert(symbolic_variable);
        }
        parameter_t tmp_param(symbolic_variable);
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        MathematicaExpressionConverter::set_range(tmp_value, tmp_range, relop_code);
        MathematicaExpressionConverter::add_parameter(symbolic_variable, tmp_param);
        maps.parameter_map.set_variable(tmp_param, tmp_range);
        symbolic_value.set_unique(false);
        MathematicaExpressionConverter::set_parameter_on_value(symbolic_value, tmp_param);
      }
      maps.variable_map.set_variable(symbolic_variable, symbolic_value);
    }
    create_result.result_maps.push_back(maps);

  }
  ml_->MLNewPacket();
  MathematicaExpressionConverter::clear_parameter_map();
  
  HYDLA_LOGGER_VCS("#*** END MathematicaVCSPoint::create_variable_map ***\n");
  return true;
}

void MathematicaVCSPoint::set_continuity(const continuity_map_t& continuity_map)
{
  continuity_map_ = continuity_map;
}


void MathematicaVCSPoint::add_left_continuity_constraint(const continuity_map_t& continuity_map, PacketSender& ps)
{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSPoint::add_left_continuity_constraint ----");
  // ���M���鐧��̌������߂�
  int left_cont_vars_count = 0;
  continuity_map_t::const_iterator md_it = continuity_map.begin();
  continuity_map_t::const_iterator md_end = continuity_map.end();
  for(; md_it!=md_end; ++md_it) {
    left_cont_vars_count += abs(md_it->second);
  }

  HYDLA_LOGGER_VCS("left_cont_vars_count: ", left_cont_vars_count);
  
  ml_->put_function("And", left_cont_vars_count);
  // ���ۂɑ��M����
  md_it = continuity_map.begin();
  md_end = continuity_map.end();
  for(; md_it!=md_end; ++md_it) {
    for(int i=0; i < abs(md_it->second); ++i){
      ml_->put_function("Equal", 2);
      
      // Prev�ϐ���
      ps.put_var(
        boost::make_tuple(md_it->first, i, true),
        PacketSender::VA_None);
      
      // Now�ϐ���
      ps.put_var(
        boost::make_tuple(md_it->first, i, false),
        PacketSender::VA_None);
    }
  }
}

void MathematicaVCSPoint::send_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCSPoint::send_constraint ***");

  PacketSender ps(*ml_);

  ml_->put_function("And", 2);
  ml_->put_function("And", constraints.size());
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it)
  {
    ps.put_node(*it, PacketSender::VA_None);
  }
  
  
  add_left_continuity_constraint(continuity_map_, ps);
  
  // vars��n��
  ps.put_vars(PacketSender::VA_None);
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSPoint::send_constraint ***");
  continuity_map_t continuity_map;
  ps.create_max_diff_map(continuity_map);
  return;
}


VCSResult MathematicaVCSPoint::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSPoint::check_consistency(tmp) ***");

  ml_->put_function("checkConsistencyTemporary", 2);
  send_constraint(constraints);
  
  VCSResult result = check_consistency_receive();
  
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSPoint::check_consistency(tmp) ***");
  return result;
}

VCSResult MathematicaVCSPoint::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSPoint::check_consistency() ***");
  ml_->put_function("checkConsistencyTemporary", 2);
  send_constraint(constraints_t());

  VCSResult result = check_consistency_receive();
  
  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCSPoint::check_consistency() ***");
  return result;
}


VCSResult MathematicaVCSPoint::check_entailment(const node_sptr &node)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSPoint::check_entailment() ***");
  ml_->put_function("checkConsistencyTemporary", 2);
  constraints_t constraints;
  constraints.push_back(node);
  send_constraint(constraints);

  VCSResult result = check_consistency_receive();
  
  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCSPoint::check_entailment() ***");
  return result;
}

VCSResult MathematicaVCSPoint::check_consistency_receive()
{
/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");
  
  //  PacketChecker pc(*ml_);
  //  pc.check();
  
  ml_->skip_pkt_until(TEXTPKT);
  std::string input_string = ml_->get_string();
  
  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    input_string);

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext();
  ml_->MLGetNext();
  ml_->MLGetNext();
  
  VCSResult result;
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    result = VCSR_SOLVER_ERROR;
    throw hydla::vcs::SolveError(input_string);
  }
  else if(ret_code==1) {
    // �[��
    result = VCSR_TRUE;
    HYDLA_LOGGER_VCS_SUMMARY("consistent");
  }
  else if(ret_code==3){
    // �[���\�����C�L���萔�̒l�ɂ���Ă͂����łȂ��ꍇ�����肤��
    result = VCSR_UNKNOWN;
    HYDLA_LOGGER_VCS_SUMMARY("undetermined"); 
  }
  else {
    // �[���s�\
    assert(ret_code==2);
    result = VCSR_FALSE;
    HYDLA_LOGGER_VCS_SUMMARY("inconsistent");
  }
  
  return result;
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

