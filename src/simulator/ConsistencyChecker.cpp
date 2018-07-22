#include "ConsistencyChecker.h"
#include "Backend.h"

#include <iostream>

#include "Logger.h"

#include "VariableFinder.h"
#include "VariableReplacer.h"
#include "HydLaError.h"
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

void ConsistencyChecker::send_range_constraint(const Variable &var, const variable_map_t &vm, bool prev_mode)
{
  if(!vm.count(var))return;
  std::string fmt = prev_mode ? "vpvlp" : "vnvln";
  auto range = vm.find(var)->second;
  if(range.unique())
  {
    value_t value = range.get_unique_value();
    std::string name = prev_mode ? "addPrevEqual" : "addEquation";
    backend->call(name.c_str(), false, 2, fmt.c_str(), "", &var, &value);
  }
  else
  {
    // replace variables in the range with their values
    VariableReplacer v_replacer(vm, false);
    v_replacer.replace_range(range);
    if(range.get_upper_cnt())
    {
      value_t value = range.get_upper_bound().value;
      if(range.get_upper_bound().include_bound)
      {
        std::string name = prev_mode ? "addPrevLessEqual" : "addLessEqual";
        backend->call(name.c_str(), false, 2, fmt.c_str(), "", &var, &value);
      }
      else
      {
        std::string name = prev_mode ? "addPrevLess" : "addLess";
        backend->call(name.c_str(), false, 2, fmt.c_str(), "", &var, &value);
      }
    }
    if(range.get_lower_cnt())
    {
      value_t value = range.get_lower_bound().value;
      if(range.get_lower_bound().include_bound)
      {
        std::string name = prev_mode ? "addPrevGreaterEqual" : "addGreaterEqual";
        backend->call(name.c_str(), false, 2, fmt.c_str(), "", &var, &value);
      }
      else
      {
        std::string name = prev_mode ? "addPrevGreater" : "addGreater";
        backend->call(name.c_str(), false, 2, fmt.c_str(), "", &var, &value);
      }
    }
  }
}

void ConsistencyChecker::send_prev_constraint(const Variable &var){
  send_range_constraint(var, *prev_map, true);
}

void ConsistencyChecker::send_init_equation(Variable &var, string fmt)
{
  fmt += "vp";
  backend->call("addInitEquation", true, 2, fmt.c_str(), "", &var, &var);
}

void ConsistencyChecker::add_continuity(VariableFinder& finder, const PhaseType &phase, const constraint_t &constraint_for_default_continuity){
  assert(prev_map != nullptr);
  std::string fmt = "v";

  map<string, int> vm;
  variable_set_t variable_set;

  // get variables assumed to be continuous
  if(constraint_for_default_continuity.get())finder.visit_node(constraint_for_default_continuity);
  if(phase == POINT_PHASE)
  {
    variable_set = finder.get_variable_set();
    fmt += "n";
  }
  else
  {
    variable_set = finder.get_all_variable_set();
    fmt += "z";
  }
  // create a map that projects variables to orders of those derivatives
  auto dm = get_differential_map(variable_set);
  for(auto dm_entry : dm)
  {
    for(int i = 0; i < dm_entry.second;i++){
      variable_t var(dm_entry.first, i);
      send_init_equation(var, fmt);
    }
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

CheckConsistencyResult ConsistencyChecker::call_backend_check_consistency(const PhaseType &phase, ConstraintStore tmp_cons)
{
  timer::Timer timer;
  backend_check_consistency_count++;
  CheckConsistencyResult ret;
  if(phase == POINT_PHASE)
  {
    backend->call("checkConsistencyPoint", true, 1, "csn", "cc", &tmp_cons, &ret);
    for(auto consistent : ret.consistent_store) HYDLA_LOGGER_DEBUG_VAR(get_infix_string(consistent));
    for(auto inconsistent : ret.inconsistent_store) HYDLA_LOGGER_DEBUG_VAR(get_infix_string(inconsistent));
  }
  else
  {
    backend->call("checkConsistencyInterval", true, 1, "cst", "cc", &tmp_cons, &ret);
  }
  backend_check_consistency_time += timer.get_elapsed_us();
  return ret;
}


map<string, int> ConsistencyChecker::get_differential_map(const variable_set_t &vs)
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
  const constraint_t &guard,
  const constraint_t &constraint_for_default_continuity,
  const asks_t &unknown_asks,
  const PhaseType &phase,
  profile_t &profile,
  bool following_step
  )
{
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard) );

  backend->call("resetConstraintForVariable", false, 0, "", "");

  VariableFinder finder;
  ConstraintStore constraint_store;

  asks_t temporarily_invalidated_asks;
  // remove constraints which are children of unknown_asks temporarily
  for(auto ask : unknown_asks)
  {
    if(relation_graph.get_entailed(ask))
    {
      relation_graph.set_entailed(ask, false);
      temporarily_invalidated_asks.insert(ask);
    }
  }
  relation_graph.get_related_constraints(guard, constraint_store);
  for(auto ask : temporarily_invalidated_asks)
  {
    relation_graph.set_entailed(ask, true);
  }    

  for(auto ask : unknown_asks)
  {
    constraint_store.erase(ask->get_child());
  }
  for(auto constraint : constraint_store)
  {
    finder.visit_node(constraint);
  }
  if(following_step) add_continuity(finder, phase, constraint_for_default_continuity);
  backend->call("addConstraint", true, 1, (phase == POINT_PHASE)?"csn":"cst", "", &constraint_store);
  return check_entailment_essential(cc_result, guard, phase, profile);
}


CheckEntailmentResult ConsistencyChecker::check_entailment(
  variable_map_t &vm,
  CheckConsistencyResult &cc_result,
  const constraint_t &guard,
  const PhaseType &phase,
  profile_t &profile
  )
{
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard) );
  
  backend->call("resetConstraintForVariable", false, 0, "", "");

  string fmt = (phase==POINT_PHASE)?"mv0n":"mv0t";
  backend->call("addConstraint", true, 1, fmt.c_str(), "", &vm);
  return check_entailment_essential(cc_result, guard, phase, profile);
}


CheckEntailmentResult ConsistencyChecker::check_entailment_essential(
  CheckConsistencyResult &cc_result,
  const constraint_t &guard,
  const PhaseType &phase,
  profile_t &profile
  )
{
  CheckEntailmentResult ce_result;

  cc_result = call_backend_check_consistency(phase, ConstraintStore(guard));

  if(cc_result.consistent_store.consistent()){
    HYDLA_LOGGER_DEBUG("%% entailable");
    if(cc_result.inconsistent_store.consistent()){
      HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }
    else
    {
      node_sptr not_node = node_sptr(new Not(guard));
      cc_result = call_backend_check_consistency(phase, ConstraintStore(not_node));
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

void ConsistencyChecker::clear_inconsistent_constraints()
{
  inconsistent_module_sets.clear();
  inconsistent_constraints.clear();
}

void ConsistencyChecker::check_consistency_foreach(const ConstraintStore &constraints,
                                                   RelationGraph &relation_graph,
CheckConsistencyResult &result,
                                           const PhaseType& phase,
                                           profile_t &profile,
                                           bool following_step)
{
  VariableFinder finder;
  for(auto constraint : constraints)
  {
    timer::Timer timer;
    finder.visit_node(constraint);
    profile["VisitNode"] += timer.get_elapsed_us();
  }
  CheckConsistencyResult tmp_result;

  tmp_result = check_consistency_essential(constraints, finder, phase, profile, following_step);

  if(!tmp_result.consistent_store.consistent())
  {
    result.consistent_store.set_consistency(false);
    HYDLA_LOGGER_DEBUG(""); inconsistent_module_sets.push_back(relation_graph.get_related_modules(constraints));
    inconsistent_constraints.push_back(constraints);
  }
  else
  {
    if(tmp_result.inconsistent_store.consistent())
    {
      // There may be some branches.
      result.consistent_store.add_constraint_store(tmp_result.consistent_store);
      result.inconsistent_store.add_constraint_store(tmp_result.inconsistent_store);
      return;
    }
    result.consistent_store.add_constraint_store(tmp_result.consistent_store);

    if(result_maps.size() == 0)return; // if result_maps has already been invalidated, skip creation
    vector<variable_map_t> create_result;
    timer::Timer timer;
    if(phase == POINT_PHASE)
    {
      backend->call("createVariableMap", true, 0, "", "cv", &create_result);
    }
    else if(following_step)
    {
      variable_set_t vars = finder.get_all_variable_set();
      std::map<std::string, int> dm = get_differential_map(vars);
      variable_set_t send_vars;
      for(auto pair : dm){
        for(int i=0; i<=pair.second; i++){
          send_vars.insert(Variable(pair.first, i));
        }
      }
      backend->call("createVariableMapInterval", true, 1, "vst", "cv", &send_vars, &create_result);
    }else backend->call("createVariableMapInterval", true, 0, "", "cv", &create_result);

    profile["CreateVMInCC"] += timer.get_elapsed_us();
    int size_of_constraint;
    backend->call("getSizeOfConstraint", false, 0, "", "i", &size_of_constraint);
    profile["SizeOfConstraint"] += size_of_constraint;

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
      //For Epsilon mode
      // for(auto var_entry : create_result[0])
      // {
      //   result_maps[0][var_entry.first] = var_entry.second;
      // }
      // int i;
      // for(i=0;i < create_result.size();i++){
      //   for(auto var : create_result[i]){

      //     HYDLA_LOGGER_DEBUG("#epsilon create result varialbe map ",i," : ",var.first," : ",var.second);
      //   }
      // }
      //if !epsilon_mode
      throw HYDLA_ERROR("result variable map is not single.");
    }
    else
    {
      result_maps.clear(); // invalidate result_maps (caused by over-constrained or under-constrained)
    }
  }
}

CheckConsistencyResult ConsistencyChecker::check_consistency(RelationGraph &relation_graph, ConstraintStore &difference_constraints, const PhaseType& phase, profile_t &profile, const asks_t &unknown_asks, bool following_step)
{
  CheckConsistencyResult result;
  result_maps.clear();
  result_maps.push_back(variable_map_t());

  timer::Timer timer;
  vector<ConstraintStore> related_constraints_list;

  // remove constraints which are children of unknown_asks temporarily
  asks_t temporarily_invalidated_asks;
  for(auto ask : unknown_asks)
  {
    HYDLA_LOGGER_DEBUG_VAR(get_infix_string(ask));
    if(relation_graph.get_entailed(ask))
    {
      relation_graph.set_entailed(ask, false);
      temporarily_invalidated_asks.insert(ask);
    }
  }

vector<module_set_t> module_set_vector;  relation_graph.get_related_constraints_vector(difference_constraints, related_constraints_list, module_set_vector);
  profile["PreparationInCC"] += timer.get_elapsed_us();
  for(auto ask : relation_graph.get_active_asks())HYDLA_LOGGER_DEBUG_VAR(get_infix_string(ask));
  for(int i = 0; i < related_constraints_list.size(); i++)
  {
    HYDLA_LOGGER_DEBUG("related[", i + 1, "/", related_constraints_list.size(),
                       "]: ", related_constraints_list[i]);
    check_consistency_foreach(related_constraints_list[i], relation_graph, result, phase, profile, following_step);
    if(result.consistent_store.consistent() && !result.inconsistent_store.empty())break;
  }

  if(result.inconsistent_store.empty()) result.inconsistent_store.set_consistency(false);

  for(auto ask : temporarily_invalidated_asks)
  {
    relation_graph.set_entailed(ask, true);
  }    


  return result;
}

void ConsistencyChecker::set_prev_map(const variable_map_t* vm)
{
  prev_map = vm;
  backend->call("clearPrevConstraint", false, 0, "", "");
  for(auto var_entry : *vm)
  {
    send_prev_constraint(var_entry.first);
  }
}

vector<variable_map_t> ConsistencyChecker::get_result_maps()
{
  return result_maps;
}

vector<module_set_t> ConsistencyChecker::get_inconsistent_module_sets()
{
  return inconsistent_module_sets;
}

list<ConstraintStore> ConsistencyChecker::get_inconsistent_constraints()
{
  return inconsistent_constraints;
}


CheckConsistencyResult ConsistencyChecker::check_consistency_essential(const ConstraintStore& constraint_store, VariableFinder &finder, const PhaseType& phase, profile_t &profile, bool following_step)
{
  timer::Timer timer;
  // PPでは変数表を全変数について作成する必要があるので，特定変数だけ解くようにするのは難しい．
  // variable_set_t vars = finder.get_all_variable_set();
  // std::map<std::string, int> dm = get_differential_map(vars);
  // variable_set_t send_vars;
  // for(auto pair : dm){
  //   for(int i=0; i<=pair.second; i++){
  //     send_vars.insert(Variable(pair.first, i));
  //   }
  // }
  // backend->set_variable_set(send_vars);
  backend->call("resetConstraintForVariable", false, 0, "", "");
  if(following_step)add_continuity(finder, phase);
  profile["AddContinuity"] += timer.get_elapsed_us();

  const char* fmt = (phase == POINT_PHASE)?"csn":"cst";
  backend->call("addConstraint", true, 1, fmt, "", &constraint_store);
  return call_backend_check_consistency(phase);
}


}

}
