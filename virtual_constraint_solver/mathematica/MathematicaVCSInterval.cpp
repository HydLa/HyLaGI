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

bool MathematicaVCSInterval::reset()
{
  constraint_store_ = constraint_store_t();
  return true;
}

bool MathematicaVCSInterval::reset(const variable_map_t& variable_map)
{

  HYDLA_LOGGER_VCS_SUMMARY("#*** Reset Constraint Store ***");
  if(variable_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Variables");
  }
  variable_map_t::variable_list_t::const_iterator it  = variable_map.begin();
  variable_map_t::variable_list_t::const_iterator end = variable_map.end();
  for(; it!=end; ++it) {
    constraint_store_.init_vars.insert(
      std::make_pair(it->first, it->second));
  }
  return true;
}

bool MathematicaVCSInterval::reset(const variable_map_t& variable_map,  const parameter_map_t& parameter_map)
{
  if(!reset(variable_map)){
    return false;
  }
  if(parameter_map.size() == 0)
  {
    HYDLA_LOGGER_VCS_SUMMARY("no Parameters");
  }
  parameter_map_=parameter_map;
  HYDLA_LOGGER_VCS(constraint_store_);
  return true;
}


void MathematicaVCSInterval::send_init_cons(
  PacketSender& ps, 
  const max_diff_map_t& max_diff_map,
  bool use_approx)
{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSInterval::send_init_cons ----");
    
  // ���M���鐧��̌������߂�
  constraint_store_t::init_vars_t::const_iterator init_vars_it;
  constraint_store_t::init_vars_t::const_iterator init_vars_end;
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  int init_vars_count = 0;

  // ����K�v�̂��鏉���l������J�E���g
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);
    if(md_it!=max_diff_map.end() && 
       md_it->second  > init_vars_it->first.derivative_count)
    {
      init_vars_count++;
    }
  }

  HYDLA_LOGGER_VCS("init_vars_count: ", init_vars_count);
  // �����Mathematica�֑��M
  ml_->put_function("List", init_vars_count);
  init_vars_it  = constraint_store_.init_vars.begin();
  init_vars_end = constraint_store_.init_vars.end();
  for(; init_vars_it!=init_vars_end; ++init_vars_it) {
    max_diff_map_t::const_iterator md_it = 
      max_diff_map.find(init_vars_it->first.name);
    if(md_it!=max_diff_map.end() &&
       md_it->second  > init_vars_it->first.derivative_count) 
    {
     ml_->put_function("Equal", 2);
      // �ϐ���
      ps.put_var(
       boost::make_tuple(init_vars_it->first.name, 
                          init_vars_it->first.derivative_count, 
                          false),
        PacketSender::VA_Zero);
      if(!init_vars_it->second.is_undefined()){
        // �l������Ȃ�l�𑗐M
        if(use_approx && approx_precision_ > 0) {
          // �ߎ����đ��M
          ml_->put_function("approxExpr", 2);
          ml_->put_integer(approx_precision_);
        }
        ps.put_node(init_vars_it->second.get_node(), PacketSender::VA_Zero, false);
      }
      else
      {
        // �l���Ȃ��Ȃ牽������̒萔������đ��M�D
        parameter_t tmp_param;
        tmp_param.name = PacketSender::par_prefix + init_vars_it->first.name;
        for(int i=0;i<init_vars_it->first.derivative_count;i++){
          //�Ƃ肠���������񐔕�d������
          tmp_param.name.append("d");
        }
        while(!parameter_map_.get_variable(tmp_param).is_undefined()){
          //�Ƃ肠�����d���񐔕�i������
          tmp_param.name.append("i");
        }
        ml_->put_symbol(tmp_param.name);
      }
    }
  }
}


void MathematicaVCSInterval::send_parameter_cons() const{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSInterval::send_parameter_cons ----");

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
  
  // �����Mathematica�֑��M
  ml_->put_function("List", para_size);
  
  for(par_it = parameter_map_.begin(); par_it!=par_end; ++par_it) {
    value_range_t::or_vector::const_iterator or_it = par_it->second.or_begin(), or_end = par_it->second.or_end();
    for(;or_it != or_end; or_it++){
      value_range_t::and_vector::const_iterator and_it = or_it->begin(), and_end = or_it->end();
      for(; and_it != and_end; and_it++){
      
        ml_->put_function(
          MathematicaExpressionConverter::get_relation_math_string(and_it->relation), 2);

        // �ϐ���
        ml_->put_symbol(PacketSender::par_prefix + par_it->first.get_name());

        // �l
        ml_->put_function("ToExpression", 1);
        ml_->put_string(and_it->value.get_string());
      }
    }
  }
}

void MathematicaVCSInterval::send_pars() const{
  HYDLA_LOGGER_VCS("---- Begin MathematicaVCSInterval::send_pars ----");

  parameter_map_t::const_iterator par_it = parameter_map_.begin();
  parameter_map_t::const_iterator par_end = parameter_map_.end();
  
  // �����Mathematica�֑��M
  ml_->put_function("List", parameter_map_.size());
  
  for(par_it = parameter_map_.begin(); par_it!=par_end; ++par_it) {
    ml_->put_symbol(PacketSender::par_prefix + par_it->first.get_name());
  }
}

void MathematicaVCSInterval::send_vars(
  PacketSender& ps, const max_diff_map_t& max_diff_map)
{
  HYDLA_LOGGER_VCS("---- MathematicaVCSInterval::send_vars ----");

  max_diff_map_t::const_iterator md_it = max_diff_map.begin();
  max_diff_map_t::const_iterator md_end = max_diff_map.end();

  // ���M�����
  int vars_count = 0;
  for(; md_it!=md_end; ++md_it) {
    for(int i=0; i<=md_it->second; i++) {
      vars_count++;
    }
  }

  HYDLA_LOGGER_VCS("count:", vars_count);

  ml_->put_function("List", vars_count);
  for(md_it=max_diff_map.begin(); md_it!=md_end; ++md_it) {
    for(int i=0; i<=md_it->second; i++) {
      ps.put_var(boost::make_tuple(md_it->first, i, false), 
                 PacketSender::VA_Time);

      HYDLA_LOGGER_VCS("put: ", 
                         "  name: ", md_it->first,
                         "  diff: ", i);
    }
  }
}

void MathematicaVCSInterval::add_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSInterval::add_constraint ***");

  // ����X�g�A��tell����̒ǉ�
  constraint_store_.constraints.insert(
    constraint_store_.constraints.end(),
    constraints.begin(), 
    constraints.end()
  );

  HYDLA_LOGGER_VCS(constraint_store_);

  return;
}


VCSResult MathematicaVCSInterval::check_consistency(const constraints_t& constraints)
{
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
    ml_->MLNewPacket();
    break;
  }
  tmp_constraints_.clear();
  return result;
}


VCSResult MathematicaVCSInterval::check_consistency()
{
  HYDLA_LOGGER_VCS("#*** Begin MathematicaVCSInterval::check_consistency ***");
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
    {
      std::string str = ml_->get_string();
      added_condition_.set(str);
      HYDLA_LOGGER_VCS(str);
    }
    break;
  }
  return result;
}



VCSResult MathematicaVCSInterval::check_consistency_sub()
{
  PacketSender ps(*ml_);

  // checkConsistencyInterval[expr, pexpr, vars, pars]��n������
  ml_->put_function("checkConsistencyInterval", 4);

  // expr����
  ml_->put_function("Join", 3);

  int tmp_size = tmp_constraints_.size();
  ml_->put_function("List", tmp_size);
  HYDLA_LOGGER_VCS(
    "tmp_size:", tmp_size);

  // �ꎞ�I�Ȑ���X�g�A�𑗐M����
  constraints_t::const_iterator tmp_it  = tmp_constraints_.begin();
  constraints_t::const_iterator tmp_end = tmp_constraints_.end();
  for(; tmp_it!= tmp_end; ++tmp_it) {
	  HYDLA_LOGGER_VCS("put node: ", (**tmp_it));
    ps.put_node(*tmp_it, PacketSender::VA_Time, true);
  }
  
  // ����X�g�Aconstraints��Mathematica�ɓn��
  send_cs(ps);

  // �ϐ��̍ő�����񐔂����Ƃ߂�
  ps.create_max_diff_map(max_diff_map_);

  // �����l����̑��M
  send_init_cons(ps, max_diff_map_, false);

  send_parameter_cons();
  
  // vars��n��
  ps.put_vars(PacketSender::VA_Time, true);
  
  // �萔��n��
  send_pars();

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
  HYDLA_LOGGER_VCS(*this);

////////////////// ���M����
  PacketSender ps(*ml_);
  
  // integrateCalc[cons, discCause, vars, maxTime]��n������
  ml_->put_function("integrateCalc", 4);

  // ����cons��n��
  ml_->put_function("Join", 4);

  send_cs(ps);
  // �ϐ��̍ő�����񐔂����Ƃ߂�
  max_diff_map_t max_diff_map;
  ps.create_max_diff_map(max_diff_map);
  // �����l����̑��M
  send_init_cons(ps, max_diff_map, true);
  
  //�萔�����̑��M
  send_parameter_cons();
  //�ǉ����ꂽ������n��
  ml_->put_function("List", 1);
  HYDLA_LOGGER_VCS("send added_condition:");
  ml_->put_function("ToExpression", 1);
  if(added_condition_.get_string() == ""){
    ml_->put_string("True");
  }else{
    ml_->put_string(added_condition_.get_string());
  }

  ml_->put_function("List", disc_cause.size());
  //���U�ω��̏�����n��
  for(constraints_t::const_iterator it = disc_cause.begin(); it != disc_cause.end();it++){
    HYDLA_LOGGER_VCS("send discCause:");
    ps.put_node(*it, PacketSender::VA_Time, true);
  }

  // �ϐ��̃��X�g��n��
  send_vars(ps, max_diff_map);

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
    ps.put_node(integrate_result.states[state_it].time.get_node(), PacketSender::VA_None, true);
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
  ps.put_node(time.get_node(), PacketSender::VA_None, false);
}

void MathematicaVCSInterval::apply_time_to_vm(const variable_map_t& in_vm, 
                                              variable_map_t& out_vm, 
                                              const time_t& time)
{
  HYDLA_LOGGER_VCS("--- apply_time_to_vm ---");

  PacketSender ps(*ml_);

  variable_map_t::const_iterator it  = in_vm.begin();
  variable_map_t::const_iterator end = in_vm.end();
  for(; it!=end; ++it) {
    HYDLA_LOGGER_VCS("variable : ", it->first);

    // �l
    value_t    value;
    if(!it->second.is_undefined()) {
      ml_->put_function("applyTime2Expr", 2);
      ps.put_node(it->second.get_node(), PacketSender::VA_Time, true);
      send_time(time);

    ////////////////// ��M����

      HYDLA_LOGGER_OUTPUT(
        "-- math debug print -- \n",
        (ml_->skip_pkt_until(TEXTPKT), ml_->get_string()));  

      ml_->skip_pkt_until(RETURNPKT);
      ml_->MLGetNext();ml_->MLGetNext();ml_->MLGetNext();

      int ret_code = ml_->get_integer();
      if(ret_code==0) {
        // TODO: �K�؂ȏ���������
        assert(0);
      }
      else {
        assert(ret_code==1);
        std::string tmp = ml_->get_string();
        MathematicaExpressionConverter mec;
        value = mec.convert_math_string_to_symbolic_value(tmp);
        HYDLA_LOGGER_OUTPUT("value : ", value.get_string());
      }
    }
    out_vm.set_variable(it->first, value);   
  }
}


void MathematicaVCSInterval::send_cs(PacketSender& ps) const
{
  HYDLA_LOGGER_VCS(
    "---- Send Constraint Store -----\n",
    "cons size: ", constraint_store_.constraints.size());

  ml_->put_function("List", 
                    constraint_store_.constraints.size());

  constraint_store_t::constraints_t::const_iterator 
    cons_it  = constraint_store_.constraints.begin();
  constraint_store_t::constraints_t::const_iterator 
    cons_end = constraint_store_.constraints.end();
  for(; (cons_it) != cons_end; ++cons_it) {
    ps.put_node((*cons_it), PacketSender::VA_Time, true);
  }
}

std::ostream& MathematicaVCSInterval::dump(std::ostream& s) const
{
  s << "#*** Dump MathematicaVCSInterval ***\n"
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
                         const MathematicaVCSInterval& vcs)
{
  return vcs.dump(s);
}

std::ostream& operator<<(std::ostream& s, 
                         const MathematicaVCSInterval::constraint_store_t& c)
{
  s << "---- MathematicaVCSInterval::consraint_store_t ----\n"
    << "-- init vars --\n";

  BOOST_FOREACH(
    const MathematicaVCSInterval::constraint_store_t::init_vars_t::value_type& i, 
    c.init_vars)
  {
    s << "variable: " << i.first
      << "   value: " << i.second
      << "\n";
  }
  s << "-- constraints --\n";
  BOOST_FOREACH(
    const MathematicaVCSInterval::constraint_store_t::constraints_t::value_type& i, 
    c.constraints)
  {
    s << *i;
  }
  return s;
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla 

