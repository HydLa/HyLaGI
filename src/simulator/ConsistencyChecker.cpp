#include "ConsistencyChecker.h"
#include "Backend.h"

#include <iostream>

#include "Logger.h"

#include "VariableFinder.h"
#include "VariableReplacer.h"
#include "SimulateError.h"
#include "Timer.h"

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


ConsistencyChecker::ConsistencyChecker(backend_sptr_t back) : backend(back), prev_map(nullptr), backend_check_consistency_count(0), backend_check_consistency_time(0){}
ConsistencyChecker::~ConsistencyChecker(){}

void ConsistencyChecker::send_prev_constraint(Variable &var)
{
  if(!prev_map->count(var))return;
  auto range = prev_map->find(var)->second;
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

  if(phase == POINT_PHASE)
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

int ConsistencyChecker::get_backend_check_consistency_time()
{
  return backend_check_consistency_time;
}


void ConsistencyChecker::reset_count()
{
  backend_check_consistency_count = 0;
  backend_check_consistency_time = 0;
}

CheckConsistencyResult ConsistencyChecker::call_backend_check_consistency(const PhaseType &phase)
{
  timer::Timer timer;
  backend_check_consistency_count++;
  CheckConsistencyResult ret;
  if(phase == POINT_PHASE)
  {
    backend->call("checkConsistencyPoint", 0, "", "cc", &ret);
  }
  else
  {
    backend->call("checkConsistencyInterval", 0, "", "cc", &ret);
  }
  backend_check_consistency_time += timer.get_elapsed_us();
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

CheckEntailmentResult ConsistencyChecker::check_entailment(
  RelationGraph &relation_graph,
  CheckConsistencyResult &cc_result,
  const ask_t &guard,
  const PhaseType &phase,
  profile_t &profile
  )
{
  
  CheckEntailmentResult ce_result;
  HYDLA_LOGGER_DEBUG(get_infix_string(guard) );
  VariableFinder finder;
  ConstraintStore constraint_store;
  module_set_t module_set;
  relation_graph.get_related_constraints(guard->get_guard(), constraint_store, module_set);


  backend->call("resetConstraintForVariable", 0, "", "");

  for(auto constraint : constraint_store)
  {
    finder.visit_node(constraint);
  }
  finder.visit_node(guard->get_child());
  add_continuity(finder, phase);

  backend->call("addConstraint", 1, (phase == POINT_PHASE)?"en":"et", "", &guard->get_guard());
  backend->call("addConstraint", 1, (phase == POINT_PHASE)?"csn":"cst", "", &constraint_store);
  
  cc_result = call_backend_check_consistency(phase);


  if(cc_result.consistent_store.consistent()){
    HYDLA_LOGGER_DEBUG("%% entailable");
    if(cc_result.inconsistent_store.consistent()){
      HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }
    else
    {
      timer::Timer timer;
      backend->call("resetConstraintForVariable", 0, "", "");
      add_continuity(finder, phase);
      backend->call("addConstraint", 1, (phase == POINT_PHASE)?"csn":"cst", "", &constraint_store);
      symbolic_expression::node_sptr not_node = symbolic_expression::node_sptr(new Not(guard->get_guard()));
      const char* fmt = (phase == POINT_PHASE)?"en":"et";
      backend->call("addConstraint", 1, fmt, "", &not_node);
      profile["PrepareForSecondCC"] += timer.get_elapsed_us();
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

void ConsistencyChecker::check_consistency(const ConstraintStore &constraints, 
                                           RelationGraph &relation_graph,
                                           module_set_t &module_set,
                                           CheckConsistencyResult &result,
                                           const PhaseType& phase,
                                           profile_t &profile)
{
  VariableFinder finder;
  for(auto constraint : constraints)
  {
    timer::Timer timer;
    finder.visit_node(constraint);
    profile["VisitNode"] += timer.get_elapsed_us();
  }
  CheckConsistencyResult tmp_result;
  tmp_result = check_consistency(constraints, finder, phase, profile);

  // TODO: consistent_storeしかまともに管理していないので、inconsistent_storeも正しく管理する（閉包計算内での分岐が発生しない限りは問題ない）
  if(!tmp_result.consistent_store.consistent())
  {
    result.consistent_store = tmp_result.consistent_store;
    inconsistent_module_sets.push_back(module_set);
  }
  else
  {
    if(tmp_result.inconsistent_store.consistent())
    {
      // There may be some branches.
      // TODO: 本来はここで論理和を取らないといけない．
      result.inconsistent_store.add_constraint_store(tmp_result.inconsistent_store);
    }
    // TODO: ここで変数表が必ずしもできるとは限らない．underconstraintの場合があるので対処する．
    // TODO: 最終的なunderconstraintは通すべきではないのでどうにかする．
    result.consistent_store.add_constraint_store(tmp_result.consistent_store);
    result.inconsistent_store.add_constraint_store(tmp_result.inconsistent_store);
    vector<variable_map_t> create_result;
    timer::Timer timer;
    if(phase == POINT_PHASE)
    {
      backend->call("createVariableMap", 0, "", "cv", &create_result);
    }
    else
    {
      backend->call("createVariableMapInterval", 0, "", "cv", &create_result);
    }
    profile["CreateVMInCC"] += timer.get_elapsed_us();
    // TODO: deal with multiple variable map
    if(create_result.size() == 1)
    {
      for(auto var_entry : create_result[0])
      {
        result_maps[0][var_entry.first] = var_entry.second;
      }
    }
    else if(create_result.size() > 1)
    {
      throw SimulateError("result variable map is not single.");
    }
  }
}

CheckConsistencyResult ConsistencyChecker::check_consistency(RelationGraph &relation_graph, ConstraintStore &difference_constraints, const variable_set_t &discrete_variables, const PhaseType& phase, profile_t &profile)
{
  CheckConsistencyResult result;
  inconsistent_module_sets.clear();
  result_maps.clear();
  result_maps.push_back(variable_map_t());
  HYDLA_LOGGER_DEBUG("");

  timer::Timer timer;
  vector<ConstraintStore> related_constraints_list;
  vector<module_set_t> related_modules_list;
  relation_graph.get_related_constraints_vector(difference_constraints, discrete_variables, related_constraints_list, related_modules_list);
  profile["PreparationInCC"] += timer.get_elapsed_us();
  for(int i = 0; i < related_constraints_list.size(); i++)
  {
    HYDLA_LOGGER_DEBUG_VAR(related_constraints_list[i]);
    check_consistency(related_constraints_list[i], relation_graph, related_modules_list[i], result, phase, profile);
  }

  
  if(result.inconsistent_store.empty())
  {
    result.inconsistent_store.set_consistency(false);
  }

  return result;
}

void ConsistencyChecker::set_prev_map(const variable_map_t* vm)
{
  prev_map = vm;
}

vector<variable_map_t> ConsistencyChecker::get_result_maps()
{
  return result_maps;
}

vector<module_set_t> ConsistencyChecker::get_inconsistent_module_sets()
{
  return inconsistent_module_sets;
}


CheckConsistencyResult ConsistencyChecker::check_consistency(const ConstraintStore& constraint_store, const VariableFinder &finder, const PhaseType& phase, profile_t &profile)
{
  timer::Timer timer;
  backend->call("resetConstraintForVariable", 0, "", "");
  add_continuity(finder, phase);
  profile["AddContinuity"] += timer.get_elapsed_us();

  const char* fmt = (phase == POINT_PHASE)?"csn":"cst";
  backend->call("addConstraint", 1, fmt, "", &constraint_store);
  return call_backend_check_consistency(phase);
}

}
}
