#include "MathematicaVCSPoint.h"

#include <cassert>

#include <boost/algorithm/string/predicate.hpp>

#include <sstream>

#include "mathlink_helper.h"
#include "PacketErrorHandler.h"
#include "Logger.h"
#include "PacketChecker.h"
#include "MathematicaExpressionConverter.h"

using namespace hydla::vcs;
using namespace hydla::logger;

namespace hydla {
namespace vcs {
namespace mathematica {

MathematicaVCSPoint::MathematicaVCSPoint(MathLink* ml) :
  ml_(ml)
{
}

MathematicaVCSPoint::~MathematicaVCSPoint()
{}

bool MathematicaVCSPoint::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map){
  HYDLA_LOGGER_VCS(
    "#*** MathematicaVCSPoint::reset ***\n");

  ml_->put_function("resetConstraint", 0);
  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLNewPacket();

  ml_->put_function("addConstraint", 2);
  ml_->put_function("Join", 2);

  std::set<std::string> names;
  PacketSender ps(*ml_);

  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
    ml_->put_function("List", 0);
  }else{
    HYDLA_LOGGER_VCS_SUMMARY("------Parameter map------\n", parameter_map);

    parameter_map_t::variable_list_t::const_iterator it = 
      parameter_map.begin();
    parameter_map_t::variable_list_t::const_iterator end = 
      parameter_map.end();

    int constraints_size = 0;
    //��ɑ��M����K�v�̂��鐧��̌��𐔂���
    for(; it!=end; ++it)
    {
      const value_range_t&    value = it->second;
      if(!value.is_undefined()) {
        value_range_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
        for(;or_it != or_end; or_it++){
          constraints_size += or_it->size();
        }
      }
    }
    HYDLA_LOGGER_VCS("size:", constraints_size);
    ml_->put_function("List", constraints_size);
    
    for(it = parameter_map.begin(); it!=end; ++it)
    {
      const value_range_t&    value = it->second;
      if(!value.is_undefined()) {
        value_range_t::or_vector::const_iterator or_it = value.or_begin(), or_end = value.or_end();
        for(;or_it != or_end; or_it++){
          value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
          for(; and_it != and_end; and_it++){
            ml_->put_function(MathematicaExpressionConverter::get_relation_math_string(and_it->relation), 2);
            ml_->put_symbol(PacketSender::par_prefix + it->first.get_name());
            ps.put_node(and_it->get_value().get_node(), PacketSender::VA_None);
          }
        }
      }
      names.insert(PacketSender::par_prefix + it->first.get_name());
    }
  }
  
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
    ml_->put_function("List", 0);
  }else{
    HYDLA_LOGGER_VCS("------Variable map------\n", variable_map);

    variable_map_t::variable_list_t::const_iterator it = 
      variable_map.begin();
    variable_map_t::variable_list_t::const_iterator end = 
      variable_map.end();
      
    int constraints_size = 0;
    //��ɑ��M����K�v�̂��鐧��̌��𐔂���
    for(; it!=end; ++it){
      const value_t&    value = it->second;
      if(!value.is_undefined()){
        constraints_size++;
      }
    }
    HYDLA_LOGGER_VCS_SUMMARY("size:", constraints_size);
    ml_->put_function("List", constraints_size );
    PacketSender ps(*ml_);
    
    for(it = variable_map.begin(); it!=end; ++it)
    {
      const MathVariable& variable = it->first;
      const value_t&    value = it->second;
      if(!value.is_undefined()){
        ml_->put_function("Equal", 2);
        ps.put_var(boost::make_tuple(variable.name,
                               variable.derivative_count,
                               true), PacketSender::VA_None);
        ps.put_node(value.get_node(), PacketSender::VA_None);
      }
      if(variable.derivative_count > 0){
        std::stringstream sstr;
        sstr << variable.derivative_count;
        names.insert("Derivative[" + sstr.str() + "][prev[" + PacketSender::var_prefix + variable.name + "]]");
      }else{
        names.insert("prev[" + PacketSender::var_prefix + variable.name + "]");
      }
    }
  }
  
  ml_->put_function("List", names.size() );
  HYDLA_LOGGER_VCS("send ", names.size(), " names");
  // �ϐ���萔�̈ꗗ�𑗐M
  std::set<std::string>::iterator it = names.begin();
  std::set<std::string>::iterator end = names.end();
  for(;it!=end;it++){
    HYDLA_LOGGER_VCS("name: ", *it);
    ml_->put_function("ToExpression", 1);
    ml_->put_string(*it);
  }

/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN(
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
///////////////////

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLNewPacket();

  HYDLA_LOGGER_VCS("#*** END MathematicaVCSPoint::reset ***\n");
  return true;
}

bool MathematicaVCSPoint::create_maps(create_result_t& create_result)
{
  HYDLA_LOGGER_VCS(
    "#*** MathematicaVCSPoint::create_variable_map ***\n");
    
/////////////////// ���M����

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
    std::set<std::string> added_parameters;  //�u����ǉ����ꂽ�L���萔�v�̈ꗗ
    variable_t symbolic_variable;
    value_t symbolic_value;
    parameter_t tmp_param;
    ml_->MLGetNext();
    int and_size = ml_->get_arg_count();
    HYDLA_LOGGER_VCS("and_size: ", and_size);
    ml_->MLGetNext(); // List�Ƃ����֐���
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
        tmp_param.name = variable_name;
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        value_t tmp_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);
        tmp_range.add(value_range_t::Element(tmp_value,MathematicaExpressionConverter::get_relation_from_code(relop_code)));
        maps.parameter_map.set_variable(tmp_param, tmp_range);
        continue;
      }
      // �֌W���Z�q�R�[�h�����ɁA�ϐ��\�̑Ή����镔���ɑ������
      // TODO: Or�̈���
      else if(!relop_code){
        //����
        symbolic_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);
        symbolic_value.set_unique(true);
      }else{
        //�s�����D���̕ϐ��̒l�͈̔͂�\�����邽�߂̋L���萔���쐬
        tmp_param.name = variable_name;
        value_t tmp_value = MathematicaExpressionConverter::convert_math_string_to_symbolic_value(value_str);

        for(int i=0;i<variable_derivative_count;i++){
          //�Ƃ肠���������񐔕�d������
          tmp_param.name.append("d");
        }
        while(1){
          if(added_parameters.find(tmp_param.name)!=added_parameters.end())break; //����ǉ����ꂽ�L���萔�Ɋ܂܂��Ȃ�C�����ꏊ�ɓ����
          value_range_t &value = maps.parameter_map.get_variable(tmp_param);
          if(value.is_undefined()){
            added_parameters.insert(tmp_param.name);
            break;
          }
          //�Ƃ肠�����d���񐔕�i������
          tmp_param.name.append("i");
        }
        tmp_range = maps.parameter_map.get_variable(tmp_param);
        MathematicaExpressionConverter::set_parameter_on_value(symbolic_value, tmp_param.name);
        symbolic_value.set_unique(false);
        tmp_range.add(value_range_t::Element(tmp_value,MathematicaExpressionConverter::get_relation_from_code(relop_code)));
        maps.parameter_map.set_variable(tmp_param, tmp_range);
      }
      maps.variable_map.set_variable(symbolic_variable, symbolic_value);
    }
    create_result.result_maps.push_back(maps);
  }
  
  
  HYDLA_LOGGER_VCS("#*** END MathematicaVCSPoint::create_variable_map ***\n");
  return true;
}

void MathematicaVCSPoint::add_left_continuity_constraint(
  PacketSender& ps, max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSPoint::add_left_continuity_constraint ----");
  // ���M���鐧��̌������߂�
  int left_cont_vars_count = 0;
  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();
  for(; md_it!=md_end; ++md_it) {
    left_cont_vars_count += md_it->second;
  }

  HYDLA_LOGGER_VCS("left_cont_vars_count: ", left_cont_vars_count);
  
  ml_->put_function("List", left_cont_vars_count);
  // ���ۂɑ��M����
  md_it = max_diff_map.begin();
  md_end = max_diff_map.end();
  for(; md_it!=md_end; ++md_it) {
    for(int i=0; i<md_it->second; ++i){
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

void MathematicaVCSPoint::add_constraint(const constraints_t& constraints)
{
  
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCSPoint::add_constraint ***");

  ml_->put_function("addConstraint", 2);
  send_constraint(constraints);

/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");

  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN(
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
///////////////////

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLNewPacket();
  

  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCSPoint::add_constraint ***");
  return;
}

void MathematicaVCSPoint::send_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS(
    "#*** Begin MathematicaVCSPoint::send_constraint ***");

  PacketSender ps(*ml_);

  ml_->put_function("Join", 2);
  ml_->put_function("List", constraints.size());
  constraints_t::const_iterator it = constraints.begin();
  constraints_t::const_iterator end = constraints.end();
  for(; it!=end; ++it)
  {
    ps.put_node(*it, PacketSender::VA_None);
  }
  
  max_diff_map_t max_diff_map;
  ps.create_max_diff_map(max_diff_map);
  add_left_continuity_constraint(ps, max_diff_map);
  
  // vars��n��
  ps.put_vars(PacketSender::VA_None);
  
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSPoint::send_constraint ***");
}

VCSResult MathematicaVCSPoint::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSPoint::check_consistency(tmp) ***");

  ml_->put_function("checkConsistencyWithTemporaryConstraint", 2);
  send_constraint(constraints);
  VCSResult result = check_consistency_receive();
  
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSPoint::check_consistency(tmp) ***");
  return result;
}

VCSResult MathematicaVCSPoint::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSPoint::check_consistency() ***");
  ml_->put_function("checkConsistency", 0);

  VCSResult result = check_consistency_receive();
  
  HYDLA_LOGGER_VCS("\n#*** End MathematicaVCSPoint::check_consistency() ***");
  return result;
}

VCSResult MathematicaVCSPoint::check_consistency_receive()
{
/////////////////// ��M����
  HYDLA_LOGGER_VCS( "--- receive ---");
  
  //PacketChecker pc(*ml_);
  //pc.check();
 
  HYDLA_LOGGER_EXTERN(
    "-- math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext();
  ml_->MLGetNext();
  ml_->MLGetNext();
  
  VCSResult result;
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    result = VCSR_SOLVER_ERROR;
  }
  else if(ret_code==1) {
    // �[��
    result = VCSR_TRUE;
    HYDLA_LOGGER_VCS_SUMMARY("consistent"); //������
  }
  else {
    assert(ret_code==2);
    result = VCSR_FALSE;
    HYDLA_LOGGER_VCS_SUMMARY("inconsistent");//����
  }
  return result;
}

VCSResult MathematicaVCSPoint::integrate(integrate_result_t& integrate_result,
  const constraints_t& constraints, const time_t& current_time, const time_t& max_time)
{
  assert(0); // Point�ł�integrate�֐�����
  return VCSR_FALSE;
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

