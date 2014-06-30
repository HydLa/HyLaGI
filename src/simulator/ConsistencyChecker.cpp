#include "ConsistencyChecker.h"
#include "Backend.h"

#include <iostream>

#include "Logger.h"

#include "VariableFinder.h"
#include "VariableReplacer.h"

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


ConsistencyChecker::ConsistencyChecker(backend_sptr_t back) : backend(back), prev_map(nullptr), backend_check_consistency_count(0){}
ConsistencyChecker::~ConsistencyChecker(){}

void ConsistencyChecker::send_prev_constraint(Variable &var)
{
  HYDLA_LOGGER_DEBUG_VAR(var);
  if(!prev_map->count(var))return;
  auto range = prev_map->find(var)->second;
  HYDLA_LOGGER_DEBUG_VAR(range);
  if(range.unique())
  {
    value_t value = range.get_unique_value();
    backend->call("addPrevEqual", 2, "vpvlp", "", &var, &value);
  }
  else
  {
    // replace variables in the range with their values
    VariableReplacer v_replacer(*prev_map);
    v_replacer.replace_range(range);
    if(range.get_upper_cnt())
    {
      value_t value = range.get_upper_bound().value;
      if(range.get_upper_bound().include_bound)
      {
        backend->call("addPrevLessEqual", 2, "vpvlp", "", &var, &value);
      }
      else
      {
        backend->call("addPrevLess", 2, "vpvlp", "", &var, &value);       
      }
    }
    if(range.get_lower_cnt())
    {
      value_t value = range.get_lower_bound().value;
      if(range.get_lower_bound().include_bound)
      {
        backend->call("addPrevGreaterEqual", 2, "vpvlp", "", &var, &value);               
      }
      else
      {
        backend->call("addPrevGreater", 2, "vpvlp", "", &var, &value);       
      }
    }
  }
}

void ConsistencyChecker::send_init_equation(Variable &var, string fmt)
{
  fmt += "vp";
  backend->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
}


void ConsistencyChecker::add_continuity(const VariableFinder& finder, const PhaseType &phase){
  assert(prev_map != nullptr);
  std::string fmt = "v";

  map<string, int> vm;
  variable_set_t variable_set;

  if(phase == PointPhase)
  {
    variable_set = finder.get_variable_set();
    fmt += "n";
    for(auto prev_variable : finder.get_prev_variable_set())
    {
      if(!variable_set.count(prev_variable)) send_prev_constraint(prev_variable);
    }    
  }
  else
  {
    fmt += "z";
    variable_set = finder.get_all_variable_set();
  }

  for(auto dm_entry : get_differential_map(variable_set))
  {
    for(int i = 0; i < dm_entry.second;i++){
      variable_t var(dm_entry.first, i);
      send_prev_constraint(var);
      send_init_equation(var, fmt);
    }
    variable_t var(dm_entry.first, dm_entry.second);
    send_prev_constraint(var);
  }

}

int ConsistencyChecker::get_backend_check_consistency_count()
{
  return backend_check_consistency_count;
}


void ConsistencyChecker::reset_count()
{
  backend_check_consistency_count = 0;
}

CheckConsistencyResult ConsistencyChecker::call_backend_check_consistency(const PhaseType &phase)
{
  backend_check_consistency_count++;
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


map<string, int> ConsistencyChecker::get_differential_map(variable_set_t &vs)
{
  map<string, int> dm;

  for(auto variable: vs)
  {
    string name = variable.get_name();
    int d_cnt = variable.get_differential_count();
    if(!dm.count(name) || dm[name] < d_cnt)
    {
      dm[name] = d_cnt;
    }
  }
  return dm;
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
  VariableFinder finder;
  ConstraintStore constraint_store;
  module_set_t module_set;
  // get constraints related with the guard 
  relation_graph.get_related_constraints(guard->get_guard(), constraint_store, module_set);


  backend->call("resetConstraintForVariable", 0, "", "");

  for(auto constraint : constraint_store)
  {
    finder.visit_node(constraint);
  }
  finder.visit_node(guard->get_child());
  add_continuity(finder, phase);

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
      add_continuity(finder, phase);
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


CheckConsistencyResult ConsistencyChecker::check_consistency(RelationGraph &relation_graph, const PhaseType& phase, const bool reuse)
{
  CheckConsistencyResult result;
  inconsistent_module_sets.clear();
  HYDLA_LOGGER_DEBUG("");
  for(int i = 0; i < relation_graph.get_connected_count(); i++)
  {

    ConstraintStore tmp_constraint_store = relation_graph.get_constraints(i);

    if(reuse && !relation_graph.is_changing(tmp_constraint_store)) continue;

    VariableFinder finder;
    variable_set_t related_variables = relation_graph.get_variables(i);
        
    for(auto constraint : tmp_constraint_store)
    {
      finder.visit_node(constraint);
    }
    CheckConsistencyResult tmp_result;

    tmp_result = check_consistency(tmp_constraint_store, finder, phase);


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

void ConsistencyChecker::set_prev_map(const variable_map_t* vm)
{
  prev_map = vm;
}

vector<module_set_t> ConsistencyChecker::get_inconsistent_module_sets()
{
  return inconsistent_module_sets;
}


CheckConsistencyResult ConsistencyChecker::check_consistency(const ConstraintStore& constraint_store, const VariableFinder &finder, const PhaseType& phase)
{
  backend->call("resetConstraintForVariable", 0, "", "");
  add_continuity(finder, phase);

  const char* fmt = (phase == PointPhase)?"csn":"cst";
  backend->call("addConstraint", 1, fmt, "", &constraint_store);
  return call_backend_check_consistency(phase);
}

}
}
