#include "ConsistencyChecker.h"
#include "Backend.h"

#include <iostream>

#include "Logger.h"

#include "ContinuityMapMaker.h"

using namespace std;
using namespace boost;

namespace hydla
{
namespace simulator
{

using namespace hierarchy;
using namespace symbolic_expression;
using namespace logger;
using namespace backend;


ConsistencyChecker::ConsistencyChecker(backend_sptr_t back) : backend(back){}
ConsistencyChecker::~ConsistencyChecker(){}

void ConsistencyChecker::add_continuity(const continuity_map_t& continuity_map, const PhaseType &phase){

  for(auto continuity : continuity_map){
    std::string fmt = "v";
    if(phase == PointPhase)
    {
      fmt += "n";
    }
    else
    {
      fmt += "z";
    }
    fmt += "vp";
    if(continuity.second>=0){
      for(int i=0; i<continuity.second;i++){
        variable_t var(continuity.first, i);
        backend->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
    }else{
      for(int i=0; i<=-continuity.second;i++){
        variable_t var(continuity.first, i);
        backend->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
      if(phase == IntervalPhase)
      {
        symbolic_expression::node_sptr rhs(new Number("0"));
        fmt = phase == PointPhase?"vn":"vt";
        fmt += "en";
        variable_t var(continuity.first, -continuity.second + 1);
        backend->call("addEquation", 2, fmt.c_str(), "", &var, &rhs);
      }
    }
  }
}

CheckConsistencyResult ConsistencyChecker::call_backend_check_consistency(const PhaseType& phase)
{
  CheckConsistencyResult ret;
  if(phase == PointPhase)
  {
    backend->call("checkConsistencyPoint", 0, "", "cc", &ret);
  }
  else
  {
    backend->call("checkConsistencyInterval", 0, "", "cc", &ret);
  }
  return ret;
}


ConsistencyChecker::CheckEntailmentResult ConsistencyChecker::check_entailment(
  CheckConsistencyResult &cc_result,
  const symbolic_expression::node_sptr& guard,
  const continuity_map_t& cont_map,
  const PhaseType& phase
  )
{
  CheckEntailmentResult ce_result;
  HYDLA_LOGGER_DEBUG(get_infix_string(guard) );
  backend->call("startTemporary", 0, "", "");
  add_continuity(cont_map, phase);
  const char* fmt = (phase == PointPhase)?"en":"et";
  backend->call("addConstraint", 1, fmt, "", &guard);
  cc_result = call_backend_check_consistency(phase);
  if(cc_result.consistent_store.consistent()){
    HYDLA_LOGGER_DEBUG("%% entailable");
    if(cc_result.inconsistent_store.consistent()){
      HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }
    else
    {
      backend->call("endTemporary", 0, "", "");
      backend->call("startTemporary", 0, "", "");
      add_continuity(cont_map, phase);
      symbolic_expression::node_sptr not_node = symbolic_expression::node_sptr(new Not(guard));
      const char* fmt = (phase == PointPhase)?"en":"et";
      backend->call("addConstraint", 1, fmt, "", &not_node);
      cc_result = call_backend_check_consistency(phase);
      if(cc_result.consistent_store.consistent()){
        HYDLA_LOGGER_DEBUG("%% entailablity branches");
        if(cc_result.inconsistent_store.consistent()){
          HYDLA_LOGGER_DEBUG("%% branches by parameters");
          ce_result = BRANCH_PAR;
        }
        ce_result = BRANCH_VAR;
      }else{
        ce_result = ENTAILED;
      }
    }
  }else{
    ce_result = CONFLICTING;
  }
  backend->call("endTemporary", 0, "", "");
  return ce_result;
}


CheckConsistencyResult ConsistencyChecker::check_consistency(RelationGraph &relation_graph, const PhaseType& phase)
{
  CheckConsistencyResult result;
  for(int i = 0; i < relation_graph.get_connected_count(); i++)
  {
    ConstraintStore tmp_constraint_store;
    ContinuityMapMaker maker;
    // TODO: ConstraintStoreでまとめる
    for(auto constraint : relation_graph.get_constraints(i))
    {
      tmp_constraint_store.add_constraint(constraint);
      maker.visit_node(constraint, phase == IntervalPhase, false);
    }
    
    CheckConsistencyResult tmp_result;
    tmp_result = check_consistency(tmp_constraint_store, maker.get_continuity_map(), phase);

    // TODO: consistent_storeしかまともに管理していないので、inconsistent_storeも正しく管理する（閉包計算内での分岐が発生しない限りは問題ない）
    if(!tmp_result.consistent_store.consistent())
    {
      result.consistent_store = tmp_result.consistent_store;
    }
    else
    {
      result.consistent_store.add_constraint_store(tmp_result.consistent_store);
    }
  }

  if(result.consistent_store.consistent())
  {
    result.inconsistent_store.set_consistency(false);
  }
  return result;
}


CheckConsistencyResult ConsistencyChecker::check_consistency(const ConstraintStore& constraint_store, const PhaseType& phase)
{
  ContinuityMapMaker maker;
  for(auto constraint : constraint_store){
    maker.visit_node(constraint, phase == IntervalPhase, false);
  }
  return check_consistency(constraint_store, maker.get_continuity_map(), phase);
}


CheckConsistencyResult ConsistencyChecker::check_consistency(const ConstraintStore& constraint_store, const continuity_map_t& continuity_map, const PhaseType& phase)
{
  backend->call("resetConstraintForVariable", 0, "", "");
  add_continuity(continuity_map, phase);

  const char* fmt = (phase == PointPhase)?"csn":"cst";
  backend->call("addConstraint", 1, fmt, "", &constraint_store);
      
  return call_backend_check_consistency(phase);
}

}
}
