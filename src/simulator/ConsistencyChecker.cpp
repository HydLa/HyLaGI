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

CheckConsistencyResult ConsistencyChecker::call_backend_check_consistency(const PhaseType &phase)
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
  RelationGraph &relation_graph,
  CheckConsistencyResult &cc_result,
  const ask_t &guard,
  const PhaseType &phase
  )
{
  CheckEntailmentResult ce_result;
  HYDLA_LOGGER_DEBUG(get_infix_string(guard) );
  ContinuityMapMaker maker;
  ConstraintStore constraint_store;
  module_set_t module_set;
  // get constraints related with the guard 
  relation_graph.get_related_constraints(guard->get_guard(), constraint_store, module_set);
  for(auto constraint : constraint_store)
  {
    maker.visit_node(constraint, phase == IntervalPhase, false);
  }
  maker.visit_node(guard->get_child(), phase == IntervalPhase, true);
  continuity_map_t cont_map = maker.get_continuity_map();
  backend->call("resetConstraintForVariable", 0, "", "");
  add_continuity(cont_map, phase);
  backend->call("addConstraint", 1, (phase == PointPhase)?"en":"et", "", &guard->get_guard());
  backend->call("addConstraint", 1, (phase == PointPhase)?"csn":"cst", "", &constraint_store);
  cc_result = call_backend_check_consistency(phase);
  if(cc_result.consistent_store.consistent()){
    HYDLA_LOGGER_DEBUG("%% entailable");
    if(cc_result.inconsistent_store.consistent()){
      HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }
    else
    {
      backend->call("resetConstraintForVariable", 0, "", "");
      add_continuity(cont_map, phase);
      backend->call("addConstraint", 1, (phase == PointPhase)?"csn":"cst", "", &constraint_store);
      symbolic_expression::node_sptr not_node = symbolic_expression::node_sptr(new Not(guard->get_guard()));
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
  return ce_result;
}


CheckConsistencyResult ConsistencyChecker::check_consistency(RelationGraph &relation_graph, const PhaseType& phase, change_variables_t *change_variables)
{
  CheckConsistencyResult result;
  inconsistent_module_sets.clear();
  HYDLA_LOGGER_DEBUG("");
  for(int i = 0; i < relation_graph.get_connected_count(); i++)
  {
    ConstraintStore tmp_constraint_store = relation_graph.get_constraints(i);
    ContinuityMapMaker maker;
    variable_set_t related_variables = relation_graph.get_variables(i);
    
    
    if(change_variables != nullptr)
    {
      // check whether current constraints include any of change_variables
      bool related = false;
      for(auto related_variable : related_variables)
      {
        if(change_variables->count(related_variable.get_name()))
        {
          related = true;
          break;
        }
      }
      if(!related)continue;
    }

    for(auto constraint : tmp_constraint_store)
    {
      maker.visit_node(constraint, phase == IntervalPhase, false);
    }
    CheckConsistencyResult tmp_result;
    tmp_result = check_consistency(tmp_constraint_store, maker.get_continuity_map(), phase);

    // TODO: consistent_storeしかまともに管理していないので、inconsistent_storeも正しく管理する（閉包計算内での分岐が発生しない限りは問題ない）
    if(!tmp_result.consistent_store.consistent())
    {
      result.consistent_store = tmp_result.consistent_store;
      inconsistent_module_sets.push_back(relation_graph.get_modules(i));
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
  HYDLA_LOGGER_DEBUG("");
  return result;
}

vector<module_set_t> ConsistencyChecker::get_inconsistent_module_sets()
{
  return inconsistent_module_sets;
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
