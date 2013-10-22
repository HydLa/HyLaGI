/*
#include "SymbolicSolver.h"


namespace hydla{
namespace solver{



void SymbolicSolver::change_mode(hydla::simulator::symbolic::Mode m, int approx_precision)
{
  mode_ = m;
}

SymbolicSolver::SymbolicSolver()
{
}

SymbolicSolver::~SymbolicSolver()
{}


void SymbolicSolver::start_temporary(){
  backend_->call("startTemporary", "", "");
}

void SymbolicSolver::end_temporary(){
  backend_->call("endTemporary", "", "");
}

void SymbolicSolver::set_conditions(const node_sptr& constraint)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  
  backend_->call("setConditions", "ep", "", &constraint);

  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}


bool SymbolicSolver::reset(const variable_map_t& variable_map, const parameter_map_t& parameter_map)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);

  backend_->call("resetConstraint", 0);
  
  backend_->call("addParameterConstraint", "mpn", "",  &parameter_map);
  
  backend_->call("addPrevConstraint", "mvp", "", &variable_map);

  HYDLA_LOGGER_FUNC_END(VCS);
  return true;
}


SymbolicSolver::create_result_t SymbolicSolver::create_maps()
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
    
  create_result_t create_result;
  if(mode_==hydla::simulator::symbolic::DiscreteMode || mode_==hydla::simulator::symbolic::ConditionsMode){
    backend_->call("createVariableMap", "", "lm", create_result);
  }else{
    backend_->call("createVariableMapInterval", "", "lm", create_result);
  }

  return create_result;
}

void SymbolicSolver::add_constraint(const constraints_t& constraints)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  std::string fmt = "le";
  fmt += (mode_==hydla::simulator::symbolic::DiscreteMode || mode_== hydla::simulator::symbolic::ConditionsMode)?"n":"t";

  backend_->call("addConstraint", fmt.c_str(), "", constraints);
  HYDLA_LOGGER_FUNC_END(VCS);
  return;
}

void SymbolicSolver::add_constraint(const node_sptr& constraint)
{
  return add_constraint(constraints_t(1, constraint));
}

void SymbolicSolver::reset_constraint(const variable_map_t& vm, const bool& send_derivatives)
{
  PacketSender ps(ml_);
  
  backend_->call("resetConstraintForVariable", "", "");
  std::string fmt = "mv";
  if(!send_derivatives)fmt+="0";
  fmt += (mode_==hydla::simulator::symbolic::DiscreteMode || mode_== hydla::simulator::symbolic::ConditionsMode)?"n":"t";
  backend_->call("addConstraint", fmt, "", vm);
}

bool SymbolicSolver::reset_parameters(const parameter_map_t& pm)
{
  backend_->call("resetConstraintForParameter", "mpn", "", pm);
  return true;
}

SymbolicSolver::ConditionsResult SymbolicSolver::find_conditions(node_sptr& node)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  backend_->call("findConditions", "", "");

/////////////////// 受信処理
  HYDLA_LOGGER_VCS( "%%receive");
  ml_.receive();
  PacketErrorHandler::handle(&ml_);
  ConditionsResult node_type = CONDITIONS_FALSE;
  node = receive_condition_node(node_type);
  //終わりなのでパケットの最後尾までスキップ
  ml_.MLNewPacket();
  if(node != NULL){
    HYDLA_LOGGER_VCS("condition : ", *node);
  }

  HYDLA_LOGGER_FUNC_END(VCS);
  return node_type; 
}

CheckConsistencyResult SymbolicSolver::check_consistency()
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  std::string f_name;
  switch(mode_){ 
  case hydla::simulator::symbolic::DiscreteMode:
    f_name = "checkConsistencyPoint";
    break;
  case hydla::simulator::symbolic::ConditionsMode:
    f_name = "checkConditions";
    break;
  default:
    f_name = "checkConsistencyInterval";
    break;
  }
  std::vector<std::vector<parametet_map_t> > p_map_lists;
  backend_->call(f_name, "", "l2lm", &p_maps_lists);
  
  CheckConsistencyResult ret;
  assert(p_maps_lists.size() == 2);
  ret.true_parameter_maps = p_maps_lists[0];
  ret.false_parameter_maps = p_maps_lists[1];
  HYDLA_LOGGER_FUNC_END(VCS);
  return ret;
}

SymbolicSolver::PP_time_result_t SymbolicSolver::calculate_next_PP_time(
  const constraints_t& discrete_cause,
  const time_t& current_time,
  const time_t& max_time)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);


  // maxTimeを渡す
  time_t tmp_time(max_time->clone());
  *tmp_time -= *current_time;
  hydla::backend::SymbolicInterface::PPTimeResult res;
  backend_->call("calculateNextPointPhaseTime", "enlet", "pr", tmp_time, disrete_cause, &res);
  return res;
}


void SymbolicSolver::set_continuity(const std::string& name, const int& derivative_count)
{
  HYDLA_LOGGER_FUNC_BEGIN(VCS);
  PacketSender ps(ml_);
  
  backend::SymbolicInterface::Variable var(name, derivative_count);
  std::string fmt = "v";
  if(mode_==hydla::simulator::symbolic::DiscreteMode || mode_==hydla::simulator::symbolic::ConditionsMode){
    fmt += "n";
  }
  else
  {
    fmt += "z";
  }

  backend_->call("addInitConstraint", fmt, "", var);

  HYDLA_LOGGER_FUNC_END(VCS);
}

} //namespace solver
} //namespace hydla 
*/
