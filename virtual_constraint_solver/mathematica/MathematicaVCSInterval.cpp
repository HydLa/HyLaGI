#include "MathematicaVCSInterval.h"

#include <string>
#include <cassert>
#include <boost/foreach.hpp>

#include "Logger.h"
#include "PacketChecker.h"
#include "PacketErrorHandler.h"
#include "MathematicaExpressionConverter.h"

using namespace hydla::vcs;
using namespace hydla::logger;
using namespace hydla::parse_tree;

namespace hydla {
namespace vcs {
namespace mathematica {


MathematicaVCSInterval::MathematicaVCSInterval(MathLink* ml, int approx_precision) :
  ml_(ml),
  approx_precision_(approx_precision)
{
}

MathematicaVCSInterval::~MathematicaVCSInterval()
{}


void MathematicaVCSInterval::set_continuity(const continuity_map_t &continuity_map){
  continuity_map_ = continuity_map;
}

void MathematicaVCSInterval::send_init_cons(PacketSender &ps, const continuity_map_t &continuity_map){
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSInterval::send_init_cons ----");

  constraints_t constraints;
  continuity_map_t::const_iterator md_it = continuity_map.begin(), md_end = continuity_map.end(); 
  for(; md_it != md_end; ++md_it) {
    for(int i=0; i < abs(md_it->second); i++)
    {
      constraints.push_back(MathematicaExpressionConverter::make_equal(variable_t(md_it->first, i), node_sptr(new Previous(node_sptr(new Variable(md_it->first)))), false));
    }
    if(md_it->second < 0){
      constraints.push_back(MathematicaExpressionConverter::make_equal(variable_t(md_it->first, abs(md_it->second)+1), node_sptr(new Number("0")), false)); 
    }
  }
  ps.put_nodes(constraints, PacketSender::VA_Zero);
}


VCSResult MathematicaVCSInterval::check_consistency(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSInterval::check_consistency(tmp) ***");
  //ml_->put_function("checkConsistencyIntervalTemporary", 2);
  ml_->put_function("checkConsistencyInterval", 0);
  VCSResult result = check_consistency_sub(constraints);
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSInterval::check_consistency(tmp) ***");
  return result;
}

VCSResult MathematicaVCSInterval::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSInterval::check_consistency ***");
  ml_->put_function("checkConsistencyIntervalTemporary", 2);
  VCSResult result = check_consistency_sub(constraints_t());
  HYDLA_LOGGER_VCS("#*** End MathematicaVCSInterval::check_consistency ***");
  return result;
}

VCSResult MathematicaVCSInterval::check_consistency_sub(const constraints_t& constraints)
{/*
  PacketSender ps(*ml_);
  // expr����
  ml_->put_function("And", 2);
  ps.put_nodes(constraints, PacketSender::VA_Time);
  // �����l����̑��M
  send_init_cons(ps, continuity_map_);

  // vars��n��
  ps.put_vars(PacketSender::VA_Time);*/

////////// ��M����
  HYDLA_LOGGER_EXTERN("--- receive  ---");

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
  }
  else if(ret_code==3){
    // �萔�l�ɂ���Ă͏[���\
    result = VCSR_UNKNOWN;
  }
  else { 
    // ����G���[
    assert(ret_code==2);
    result = VCSR_FALSE;
  }
  return result;
}


VCSResult MathematicaVCSInterval::integrate(
  integrate_result_t& integrate_result,
  const constraints_t& disc_cause,
  const time_t& current_time,
  const time_t& max_time)
{
  HYDLA_LOGGER_VCS("#*** MathematicaVCSInterval::integrate ***");

////////////////// ���M����
  PacketSender ps(*ml_);
  
  // integrateCalc[cons, discCause, vars, maxTime]��n������
  ml_->put_function("integrateCalc", 4);

  // �����l����̑��M
  send_init_cons(ps, continuity_map_);

  ml_->put_function("List", disc_cause.size());
  //���U�ω��̏�����n��
  for(constraints_t::const_iterator it = disc_cause.begin(); it != disc_cause.end();it++){
    HYDLA_LOGGER_VCS("send discCause:");
    ps.put_node(*it, PacketSender::VA_Time);
  }

  // �ϐ��̃��X�g��n��
  ps.put_vars(PacketSender::VA_Time);

  // maxTime��n��
  time_t tmp_time(max_time);
  tmp_time -= current_time;
  

  HYDLA_LOGGER_VCS("current time:", current_time);
  HYDLA_LOGGER_VCS("send time:", tmp_time);
  if(approx_precision_ > 0) {
    // �ߎ����đ��M
    ml_->put_function("approxExpr", 2);
    ml_->put_integer(approx_precision_);
  }
  send_time(tmp_time);


////////////////// ��M����

//   PacketChecker pc(*ml_);
//   pc.check();

  MathematicaExpressionConverter mec;

  HYDLA_LOGGER_EXTERN(
    "-- integrate math debug print -- \n",
    (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));
  HYDLA_LOGGER_EXTERN((ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));

  ml_->skip_pkt_until(RETURNPKT);
  ml_->MLGetNext(); 
  ml_->MLGetNext(); // List�Ƃ����֐���
  ml_->MLGetNext(); // List�̒��̐擪�v�f

  // �����ɐ����������ǂ���
  int ret_code = ml_->get_integer();
  if(PacketErrorHandler::handle(ml_, ret_code)) {
    return VCSR_SOLVER_ERROR;
  }
  HYDLA_LOGGER_VCS("---integrate calc result---");
  virtual_constraint_solver_t::IntegrateResult::NextPhaseState state;

  
  // �ϐ��ƒl�̑g���󂯎��
  ml_->MLGetNext(); // List�Ƃ����֐���
  int variable_list_size = ml_->get_arg_count();
  HYDLA_LOGGER_VCS("variable_list_size : ", variable_list_size);  
  ml_->MLGetNext(); ml_->MLGetNext();
  for(int i=0; i<variable_list_size; i++)
  {
    ml_->MLGetNext(); 
    ml_->MLGetNext();
    variable_t variable;
    value_t    value;
    HYDLA_LOGGER_VCS("--- add variable ---");

      // �ϐ���
    variable.name = ml_->get_symbol().substr(6);
    HYDLA_LOGGER_VCS("name  : ", variable.name);
    ml_->MLGetNext();

      // ������
    variable.derivative_count = ml_->get_integer();
    HYDLA_LOGGER_VCS("derivative : ", variable.derivative_count);
    ml_->MLGetNext();

      // �l
    value = mec.convert_math_string_to_symbolic_value(ml_->get_string());
    HYDLA_LOGGER_VCS("value : ", value.get_string());
    ml_->MLGetNext();

    state.variable_map.set_variable(variable, value); 
  }
  
  
  // ����PP�̎����ƁC���̏ꍇ�̏����̑g�C�X�ɏI���������ǂ����𓾂�
  HYDLA_LOGGER_VCS("-- receive next PP time --");
  int next_time_size = ml_->get_arg_count();
  HYDLA_LOGGER_VCS("next_time_size: ", next_time_size);
  ml_->MLGetNext();
  for(int time_it = 0; time_it < next_time_size; time_it++){
    ml_->MLGetNext();ml_->MLGetNext(); ml_->MLGetNext();
    // �������󂯎��
    state.time = mec.convert_math_string_to_symbolic_value(ml_->get_string()) + current_time;
    HYDLA_LOGGER_VCS_SUMMARY("next_phase_time: ", state.time);
    ml_->MLGetNext();
    int condition_size = ml_->get_arg_count();//�������̐�
    ml_->MLGetNext(); ml_->MLGetNext();
    
    state.parameter_map.clear();
    parameter_t tmp_param, prev_param;
    // �������󂯎��
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
    
    // �I���������ǂ������󂯎��
    state.is_max_time = ml_->get_integer();
    HYDLA_LOGGER_VCS("is_max_time: ",  state.is_max_time);
    HYDLA_LOGGER_VCS("--parameter map--\n",  state.parameter_map);
    integrate_result.states.push_back(state);
  }

////////////////// ��M�I��

  for(unsigned int state_it = 0; state_it < integrate_result.states.size(); state_it++){
  // �����̊Ȗ񉻁B�{���͊֐��g���Ă��ׂ������ǁA�Ƃ肠�������̂܂܂�����
    HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", integrate_result.states[state_it].time);
    ml_->put_function("ToString", 1);
    ml_->put_function("FullForm", 1);
    ml_->put_function("Simplify", 1);
    ps.put_node(integrate_result.states[state_it].time.get_node(), PacketSender::VA_None);
    ml_->skip_pkt_until(RETURNPKT);
    integrate_result.states[state_it].time = mec.convert_math_string_to_symbolic_value(ml_->get_string());


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
  }


  return VCSR_TRUE;
}

void MathematicaVCSInterval::send_time(const time_t& time){
  HYDLA_LOGGER_VCS("SymbolicTime::send_time : ", time);
  PacketSender ps(*ml_);
  ps.put_node(time.get_node(), PacketSender::VA_None);
}

} // namespace mathematica
} // namespace simulator
} // namespace hydla 

