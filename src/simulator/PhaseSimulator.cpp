#include <iostream>

#include "PhaseSimulator.h"

#include "Logger.h"
#include "Timer.h"

#include "PrevReplacer.h"
#include "VariableFinder.h"
#include "PrevSearcher.h"
#include "HydLaError.h"
#include "AlwaysFinder.h"
#include "EpsilonMode.h"
#include "ValueModifier.h"

#include "MinTimeCalculator.h"

#include "Backend.h"

namespace hydla
{
namespace simulator
{

using namespace std;
using namespace boost;

using namespace backend;
using namespace hierarchy;
using namespace symbolic_expression;
using namespace timer;

PhaseSimulator::PhaseSimulator(Simulator* simulator,const Opts& opts): simulator_(simulator), opts_(&opts) {
}

PhaseSimulator::~PhaseSimulator(){}

void PhaseSimulator::process_todo(phase_result_sptr_t &todo)
{
  timer::Timer phase_timer;
  module_set_container->reset();
  todo->inconsistent_module_sets.clear();
  todo->inconsistent_constraints.clear();
  aborting = false;

  if(todo->parent == result_root.get())
  {
    for(auto module : module_set_container->get_max_module_set())
    {
      relation_graph_->set_expanded_recursive(module.second, true);
    }
    todo->diff_sum.add_constraint_store(relation_graph_->get_constraints());
  }

  list<phase_result_sptr_t> phase_list = make_results_from_todo(todo);

  if(phase_list.empty())
  {
    todo->simulation_state = INCONSISTENCY;
    todo->parent->children.push_back(todo);
  }
  else
  {
    int phase_num = 0;
    for(auto phase : phase_list)
    {
      // TOOD: move this block into upper level
      if(phase->parent == result_root.get())
      {
        AlwaysFinder always_finder;
        ConstraintStore non_always;
        always_set_t always_set;
        for(auto module : module_set_container->get_max_module_set())
        {
          always_finder.find_always(module.second, &always_set, &non_always);
        }
        for(auto constraint : non_always)
        {
          relation_graph_->set_expanded_atomic(constraint, false);
        }
      }
      make_next_todo(phase);
      phase->id += phase_num;
      phase_num++;
      todo->parent->todo_list.push_back(phase);
      if(aborting)break;
    }
  }

  todo->profile["PhaseResult"] += phase_timer.get_elapsed_us();  
}


std::list<phase_result_sptr_t> PhaseSimulator::make_results_from_todo(phase_result_sptr_t& todo)
{
  std::list<phase_result_sptr_t> result_list;  
  timer::Timer preprocess_timer;

  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &todo->parameter_map);
  backend_->call("addParameterConstraint", 1, "csn", "", &todo->additional_constraint_store);
  consistency_checker->set_prev_map(&todo->prev_map);
  relation_graph_->set_ignore_prev(todo->phase_type == POINT_PHASE);

  todo->profile["Preprocess"] += preprocess_timer.get_elapsed_us();

  asks_t nonprev_trigger_asks;
  if(todo->phase_type == POINT_PHASE)
  {
    for(auto trigger : todo->discrete_asks)
    {
      if(trigger.second)
      {
        ask_t ask = trigger.first;
        bool entailed = relation_graph_->get_entailed(ask);
        
        if(relation_graph_->entail_if_prev(ask,
                                           !entailed))
        {
          todo->always_list.add_constraint_store(relation_graph_->get_always_list(ask));
          todo->diff_sum.add_constraint(ask->get_child());
          
          if(entailed)
          {
            todo->diff_negative_asks.insert(ask);
          }else
          {
            todo->diff_positive_asks.insert(ask);
          }
        }
        
        else
        {
          nonprev_trigger_asks.insert(ask);
        }
      }
    }
  }
  else
  {
    for(auto trigger : todo->discrete_asks)
    {
      nonprev_trigger_asks.insert(trigger.first);
    }
  }
  
  while(module_set_container->has_next())
  {
    // TODO: unadopted_ms も差分をとるようにして使いたい
    // 大体の例題ではトップレベルからでも効率が悪化しないので，そこまで重要ではなさそう
    module_set_t unadopted_ms = module_set_container->unadopted_module_set();
    string module_sim_string = "\"ModuleSet" + unadopted_ms.get_name() + "\"";
    timer::Timer ms_timer;

    result_list.merge(simulate_ms(unadopted_ms, todo, nonprev_trigger_asks));

    todo->profile[module_sim_string] += ms_timer.get_elapsed_us();
  }

  if(todo->profile["# of CheckConsistency"]) todo->profile["Average of CheckConsistency"] = todo->profile["CheckConsistency"] / todo->profile["# of CheckConsistency"];
  if(todo->profile["# of CheckEntailment"]) todo->profile["Average of CheckEntailment"] =  todo->profile["CheckEntailment"] / todo->profile["# of CheckEntailment"];
  return result_list;
}

module_diff_t PhaseSimulator::get_module_diff(module_set_t unadopted_ms, module_set_t parent_unadopted)
{
  module_diff_t module_diff;
  for(auto p_it = parent_unadopted.begin(); p_it != parent_unadopted.end();)
  {
    auto parent_module = *p_it;
    bool duplicate = false;
    for(auto it = unadopted_ms.begin(); it != unadopted_ms.end(); it++)
    {
      auto module = *it;
      if(module.second == parent_module.second)
      {
        duplicate = true;
        unadopted_ms.erase(it);
        break;
      }
    }
    if(!duplicate)
    {
      module_diff.insert(make_pair(parent_module, true));
      p_it = parent_unadopted.erase(p_it);
    }
    else
    {
      p_it++;
    }
  }
  for(auto it = unadopted_ms.begin(); it != unadopted_ms.end();it++)
  {
    auto module = *it;
    bool duplicate = false;
    for(auto p_it = parent_unadopted.begin(); p_it != parent_unadopted.end(); p_it++)
    {
      auto parent_module = *p_it;
      if(module.second == parent_module.second)
      {
        duplicate = true;
        parent_unadopted.erase(p_it);
        break;
      }
    }
    if(!duplicate)
    {
      module_diff.insert(make_pair(module, false));
    } 
  }
  return module_diff;
}


list<phase_result_sptr_t> PhaseSimulator::simulate_ms(const module_set_t& unadopted_ms, phase_result_sptr_t& phase, asks_t trigger_asks)
{
  HYDLA_LOGGER_DEBUG("\n--- next unadopted module set ---\n", unadopted_ms.get_infix_string());

  module_diff_t module_diff = get_module_diff(unadopted_ms, phase->parent->unadopted_ms);
  
  ConstraintStore local_diff_sum = phase->diff_sum;
  for(auto diff : module_diff)
  {
    relation_graph_->set_adopted(diff.first, diff.second);
    local_diff_sum.add_constraint(diff.first.second);
  }

  list<phase_result_sptr_t> result_list;  
  timer::Timer cc_timer;

  asks_t cc_local_positives, cc_local_negatives;
  ConstraintStore cc_local_always;

  consistency_checker->clear_inconsistent_constraints();
  bool consistent = calculate_closure(phase, trigger_asks, local_diff_sum, cc_local_positives, cc_local_negatives, cc_local_always);
  phase->profile["CalculateClosure"] += cc_timer.get_elapsed_us();
  phase->profile["# of CalculateClosure"]++;

  for(auto module_set : consistency_checker->get_inconsistent_module_sets()) phase->inconsistent_module_sets.push_back(module_set);
  for(auto constraint_store : consistency_checker->get_inconsistent_constraints()) phase->inconsistent_constraints.push_back(constraint_store);
      

  if(!consistent)
  {
    HYDLA_LOGGER_DEBUG("INCONSISTENT: ", unadopted_ms);
    for(auto module_set : consistency_checker->get_inconsistent_module_sets())
    {
      timer::Timer gen_timer;
      module_set_container->generate_new_ms(phase->unadopted_mss, module_set);
      phase->profile["GenerateNewMS"] += gen_timer.get_elapsed_us();
    }
  }
  else
  {
    HYDLA_LOGGER_DEBUG("CONSISTENT: ", unadopted_ms);
    timer::Timer postprocess_timer;
    module_set_container->remove_included_ms_by_current_ms();
    phase->profile["RemoveIncludedMS"] += postprocess_timer.get_elapsed_us();
    phase->unadopted_mss.insert(unadopted_ms);

    if(phase->simulation_state == SIMULATED)
    {
      phase.reset(new PhaseResult(*phase));
      phase->profile.clear();
      phase->parent->todo_list.push_back(phase);
    }
    phase->parent->children.push_back(phase);

    vector<variable_map_t> create_result = consistency_checker->get_result_maps();
    if(create_result.size() == 0)
    {
      throw HYDLA_ERROR("some error occured in creation of variable maps.");
    }
    else if(create_result.size() > 1)
    {
      throw HYDLA_ERROR("result variable map is not single.");
    }
    phase->variable_map = create_result[0];
    
    if(opts_->epsilon_mode >= 0){
      cut_high_order_epsilon(backend_.get(),phase, opts_->epsilon_mode);
    }
    

    phase->profile["# of CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_count();
    phase->profile["CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_time();
    consistency_checker->reset_count();

    list<parameter_map_t> parameter_maps;
    backend_->call("createParameterMaps", 0, "", "mps", &parameter_maps);
    assert(parameter_maps.size() == 1);
    phase->parameter_map = parameter_maps.front();
    
    phase->diff_positive_asks.insert(cc_local_positives.begin(), cc_local_positives.end());
    phase->diff_negative_asks.insert(cc_local_negatives.begin(), cc_local_negatives.end());

    phase->always_list.add_constraint_store(cc_local_always);
    phase->simulation_state = SIMULATED;
    phase->diff_sum = local_diff_sum;
    phase->unadopted_ms = unadopted_ms;
    phase->module_diff = module_diff;
    result_list.push_back(phase);

    if(phase->phase_type == POINT_PHASE && phase->in_following_step()){
      VariableFinder finder;
      for(auto constraint : local_diff_sum){
        finder.visit_node(constraint);
      }
      variable_set_t tmp_vs = finder.get_variable_set(), discrete_vs;
      std::map<std::string, int> tmp_vm = consistency_checker->get_differential_map(tmp_vs);
      for(auto pair : tmp_vm){
        int max_d_count = -1;
        for(int i = pair.second; i>=0; i--){
          Variable var(pair.first, i);
          if(phase->variable_map.count(var)){
            max_d_count = i;
            break;
          }
        }
        assert(max_d_count >= 0);
        Variable var(pair.first, max_d_count);
        if(!consistency_checker->check_continuity(var, phase->variable_map))
          discrete_vs.insert(var);
      }
      phase->discrete_differential_set = discrete_vs;
    }
    phase->profile["Postprocess"] += postprocess_timer.get_elapsed_us();
  }

  revert_diff(cc_local_positives, cc_local_negatives, cc_local_always, module_diff);
  return result_list;
}

void PhaseSimulator::push_branch_states(phase_result_sptr_t &original, CheckConsistencyResult &result){
  phase_result_sptr_t branch_state_false(new PhaseResult(*original));
  branch_state_false->id = ++phase_sum_;
  branch_state_false->additional_constraint_store.add_constraint_store(result.inconsistent_store);
  original->parent->todo_list.push_back(branch_state_false);
  original->additional_constraint_store.add_constraint_store(result.consistent_store);
    backend_->call("addParameterConstraint", 1, "csn", "", &original->additional_constraint_store);
}


void PhaseSimulator::check_break_points(phase_result_sptr_t &phase, variable_map_t &variable_map)
{
  if(!break_point_list.empty())
  {
    timer::Timer entailment_timer;
    backend_->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);
    for(auto entry : break_point_list)
    {
      auto break_point = entry.first;
      CheckConsistencyResult cc_result;
      HYDLA_LOGGER_DEBUG_VAR(get_infix_string(break_point.condition));
      variable_map_t related_vm = get_related_vm(break_point.condition, variable_map);
      switch(consistency_checker->check_entailment(related_vm, cc_result, break_point.condition, phase->phase_type, phase->profile)){
      case BRANCH_PAR:
        push_branch_states(phase, cc_result);
      case ENTAILED:
        if(!break_point.call_back(break_point, phase))aborting = true;
        break;
      case CONFLICTING:
      case BRANCH_VAR:
        break;
      }
      phase->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    }
  }
}

void PhaseSimulator::initialize(variable_set_t &v,
                                parameter_map_t &p,
                                variable_map_t &m,
                                module_set_container_sptr &msc,
                                phase_result_sptr_t root)
{
  variable_set_ = &v;
  parameter_map_ = &p;
  variable_map_ = &m;
  phase_sum_ = 1;
  module_set_container = msc;
  result_root = root;

  simulator::module_set_t ms = module_set_container->get_max_module_set();

  
  relation_graph_.reset(new RelationGraph(ms)); 

  if(opts_->dump_relation){
    relation_graph_->dump_graph(cout);
    exit(EXIT_SUCCESS);
  }

  for(auto variable : *variable_set_)
  {
    variable_names.insert(variable.get_name());
  }

  FullInformation root_information;
  root_information.negative_asks = relation_graph_->get_all_asks();
  result_root->set_full_information(root_information);

  if(opts_->max_time != ""){
    max_time = symbolic_expression::node_sptr(new symbolic_expression::Number(opts_->max_time));
  }else{
    max_time = symbolic_expression::node_sptr(new symbolic_expression::Infinity());
  }

  aborting = false;

  backend_->set_variable_set(*variable_set_);
  value_modifier.reset(new ValueModifier(*backend_));
}

void PhaseSimulator::replace_prev2parameter(PhaseResult &phase,
                                            variable_map_t &vm,
                                            parameter_map_t &parameter_map)
{
  assert(phase.parent != nullptr);
  PrevReplacer replacer(parameter_map, phase, *simulator_);
  for(auto var_entry : vm)
  {
    ValueRange range = var_entry.second;
    value_t val;
    if(range.unique())
    {
      val = range.get_unique_value();
      if(replacer.replace_value(val))
      {
        range.set_unique_value(val);
        vm[var_entry.first] = range;
      }
    }
    else
    {
      bool replaced = false;
      if(range.get_upper_cnt()>0)
      {
        val = range.get_upper_bound().value;
        if(replacer.replace_value(val))
        {
          range.set_upper_bound(val, range.get_upper_bound().include_bound);
          replaced = true;
        }
      }
      if(range.get_lower_cnt() > 0)
      {
        val = range.get_lower_bound().value;
        if(replacer.replace_value(val))
        {
          range.set_lower_bound(val, range.get_lower_bound().include_bound);
          replaced = true;
        }
      }
      if(replaced)vm[var_entry.first] = range;
    }
  }
}

void PhaseSimulator::set_backend(backend_sptr_t back)
{
  backend_ = back;
  consistency_checker.reset(new ConsistencyChecker(backend_));
}

bool PhaseSimulator::calculate_closure(phase_result_sptr_t& phase, asks_t &trigger_asks, ConstraintStore &diff_sum, asks_t &positive_asks, asks_t &negative_asks, ConstraintStore expanded_always)
{
  asks_t unknown_asks = trigger_asks;
  bool expanded, first_loop = true;
  PhaseType phase_type = phase->phase_type;
  do{
    HYDLA_LOGGER_DEBUG_VAR(diff_sum);
    expanded = false;
    timer::Timer entailment_timer;
    
    variable_set_t discrete_variables;
    for(auto constraint: diff_sum)
    {
      VariableFinder finder;
      finder.visit_node(constraint);
      variable_set_t variables = phase_type==POINT_PHASE?finder.get_variable_set()
        :finder.get_all_variable_set();
      discrete_variables.insert(variables.begin(), variables.end());
    }

    set<ask_t> adjacents;
    if(phase->phase_type == POINT_PHASE || !phase->in_following_step()){
      for(auto variable : discrete_variables) adjacents = relation_graph_->get_adjacent_asks(variable.get_name(), phase_type == POINT_PHASE);
    }else{
      for(auto variable : discrete_variables)
      {
        if(phase->parent->discrete_differential_set.count(variable))
          adjacents = relation_graph_->get_adjacent_asks2var_and_derivatives(variable, phase_type == POINT_PHASE);
      }
    }
    for(auto adjacent : adjacents)
    {
      unknown_asks.insert(adjacent);
    }
    for(auto ask_it = unknown_asks.begin(); ask_it != unknown_asks.end();)
    {
      const auto ask = *ask_it;

      if(phase_type == POINT_PHASE
         // in initial phase, conditions about left-hand limits are considered to be invalid
         && phase->parent == result_root.get()
         && PrevSearcher().search_prev(ask))
      {
        ask_it = unknown_asks.erase(ask_it);
        continue;
      }

      CheckConsistencyResult check_consistency_result;
      switch(consistency_checker->check_entailment(*relation_graph_, check_consistency_result, ask->get_guard(), ask->get_child(), phase_type, phase->profile)){

      case BRANCH_PAR:
        HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
        push_branch_states(phase, check_consistency_result);
        // Since we choose entailed case in push_branch_states, we go down without break.
      case ENTAILED:
        HYDLA_LOGGER_DEBUG("\n--- entailed ask ---\n", get_infix_string(ask));
        if(!relation_graph_->get_entailed(ask))
        {
          if(negative_asks.erase(ask))
          {
            diff_sum.erase(ask->get_child());
          }
          else
          {
expanded_always.add_constraint_store(relation_graph_->get_always_list(ask));
            positive_asks.insert(ask);
            diff_sum.add_constraint(ask->get_child());
          }
          relation_graph_->set_entailed(ask, true);
          expanded = true;
        }
        ask_it = unknown_asks.erase(ask_it);
        break;
      case CONFLICTING:
        HYDLA_LOGGER_DEBUG("\n--- conflicted ask ---\n", get_infix_string(ask));
        if(relation_graph_->get_entailed(ask))
        {
          if(positive_asks.erase(ask))
          {
            diff_sum.erase(ask->get_child());
          }
          else
          {
            negative_asks.insert(ask);
            diff_sum.add_constraint(ask->get_child());
          }
          relation_graph_->set_entailed(ask, false);
          expanded = true;
        }
        ask_it = unknown_asks.erase(ask_it);
        break;
      case BRANCH_VAR:
        HYDLA_LOGGER_DEBUG("\n--- branched ask ---\n", get_infix_string(ask));
        ask_it++;
        break;
      }
      phase->profile["# of CheckEntailment"]+= 1;
    }
    phase->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    if(expanded || first_loop)
    {
      // loop until no branching occurs
      while(true)
      {
        timer::Timer consistency_timer;
        CheckConsistencyResult cc_result;
        cc_result = consistency_checker->check_consistency(*relation_graph_, diff_sum, phase_type, phase->profile, phase->in_following_step());
        phase->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        phase->profile["# of CheckConsistency"]++;
        if(!cc_result.consistent_store.consistent()){
          return false;
        }else if (cc_result.inconsistent_store.consistent()){
          push_branch_states(phase, cc_result);
        }
        else
        {
          break;
        }
      }
      first_loop = false;
    }
  }while(expanded);

  if(!unknown_asks.empty()){
    //TODO: implement branching here
  }
  return true;
}

variable_map_t PhaseSimulator::get_related_vm(const node_sptr &node, const variable_map_t &vm)
{
  VariableFinder var_finder;
  var_finder.visit_node(node);
  variable_set_t related_variables = var_finder.get_all_variable_set();
  variable_map_t related_vm;
  for(auto related_variable : related_variables)
  {
    auto vm_it = vm.find(related_variable);
    if(vm_it != vm.end())
      related_vm[related_variable] = vm_it->second;
  }
  return related_vm;
}

find_min_time_result_t PhaseSimulator::find_min_time(const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed)
{
  std::list<AtomicConstraint *> guards = relation_graph_->get_atomic_guards(guard);
  for(auto atomic_guard : guards)
  {
    constraint_t guard = atomic_guard->constraint;
    constraint_t constraint_for_this_guard;
    if(!guard_time_map.count(guard))
    {
      variable_map_t related_vm = get_related_vm(guard, original_vm);
      backend_->call("calculateConsistentTime", 3, "etmvtvlt", "e", &guard, &related_vm, &time_limit, &constraint_for_this_guard);
      guard_time_map[guard] = constraint_for_this_guard;
    }
  }

  find_min_time_result_t min_time_for_this_ask;
  return min_time_calculator.calculate_min_time(&guard_time_map, guard, entailed);
}

void
PhaseSimulator::make_next_todo(phase_result_sptr_t& phase)
{
  timer::Timer next_todo_timer;

  apply_diff(*phase);

  phase_result_sptr_t next_todo(new PhaseResult());
  next_todo->id = ++phase_sum_;
  next_todo->step = phase->step + 1;
  next_todo->parameter_map = phase->parameter_map;
  if(phase->phase_type == POINT_PHASE)
  {
    if(phase->in_following_step()){
      variable_map_t &vm_to_take_over = phase->prev_map;
      for(auto var_entry : vm_to_take_over)
      {
        auto var = var_entry.first;
        if(!phase->variable_map.count(var) && relation_graph_->referring(var) )
        {
          phase->variable_map[var] = vm_to_take_over[var];
        }
      }
    }
    check_break_points(phase, phase->variable_map);
    if(!aborting)
    {
      next_todo->phase_type = INTERVAL_PHASE;
      next_todo->parent = phase.get();
      next_todo->diff_sum = phase->diff_sum;
      next_todo->current_time = phase->end_time = phase->current_time;
      next_todo->discrete_asks = phase->discrete_asks;
      next_todo->prev_map = phase->variable_map;
      phase->todo_list.push_back(next_todo);
    }
  }
  else
  {
    PhaseSimulator::replace_prev2parameter(*phase->parent, phase->variable_map, phase->parameter_map);
    next_todo->phase_type = POINT_PHASE;

    backend_->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);

    value_t time_limit(max_time);
    time_limit -= phase->current_time;

    
    variable_map_t original_vm = phase->variable_map;
    phase->variable_map = value_modifier->shift_time(phase->current_time, phase->variable_map);

    if(phase->in_following_step()){
      variable_map_t &vm_to_take_over = phase->parent->parent->variable_map;
      for(auto var_entry : vm_to_take_over)
      {
        auto var = var_entry.first;
        if(!phase->variable_map.count(var) && relation_graph_->referring(var) )
        {
          phase->variable_map[var] = vm_to_take_over[var];
          // create a variable map whose start point(t=0) is the start of this phase
          original_vm[var] = value_modifier->shift_time(-phase->current_time, vm_to_take_over[var]);
        }
      }
    }
    check_break_points(phase, original_vm);
    if(!aborting)
    {
      if(phase->in_following_step())
      {
        phase->next_pp_candidate_map = phase->parent->next_pp_candidate_map;
        for(auto &entry : phase->next_pp_candidate_map)
        {
          for(auto cand_it = entry.second.begin(); cand_it != entry.second.end();)
          {
            auto &candidate = *cand_it;
            candidate.time -= (phase->current_time - phase->parent->parent->current_time);
            list<parameter_map_t> parameter_maps;
            backend_->call("productWithGlobalParameterConstraint", 1, "mp", "mps", &candidate.parameter_map, &parameter_maps);
            assert(parameter_maps.size() <= 1);
            if(parameter_maps.empty())entry.second.erase(cand_it++);
            else
            {
              candidate.parameter_map = parameter_maps.front();
              cand_it++;
            }
          }
        }
      }
      next_pp_candidate_map_t &candidate_map = phase->next_pp_candidate_map;

      set<constraint_t> calculated_ask_set;
      set<string> checked_variables;

      variable_set_t diff_variables;
      {
        for(auto diff: phase->diff_sum)
        {
          variable_set_t var_set;
          var_set = relation_graph_->get_related_variables(diff);
          diff_variables.insert(var_set.begin(), var_set.end());
        }
      }

      guard_time_map_t guard_time_map;

      // 各変数に関する最小時刻をask単位で更新する．
      MinTimeCalculator min_time_calculator(relation_graph_.get(), backend_.get());
      for(auto variable : diff_variables)
      {
        string var_name = variable.get_name();
        // 既にチェック済みの変数なら省略（x'とxはどちらもxとして扱うため，二回呼ばれないようにする）
        if(checked_variables.count(var_name))continue;
        checked_variables.insert(var_name);
        asks_t asks = relation_graph_->get_adjacent_asks(var_name);
        
        for(auto ask : asks)
        {
          if(calculated_ask_set.count(ask))continue;
          calculated_ask_set.insert(ask);
                  
          if(opts_->epsilon_mode >= 0)
          {
            candidate_map[ask] = find_min_time_epsilon(ask->get_guard(), original_vm,
                                                 time_limit, phase, backend_.get());
          }
          
          candidate_map[ask] = find_min_time(ask->get_guard(), min_time_calculator, guard_time_map, original_vm, time_limit, relation_graph_->get_entailed(ask));
        }
      }

      for(auto entry : break_point_list)
      {
        auto break_point = entry.first;
        HYDLA_LOGGER_DEBUG_VAR(get_infix_string(entry.first.condition));

        entry.second = find_min_time(break_point.condition, min_time_calculator, guard_time_map, original_vm, time_limit, false);
      }

      pp_time_result_t time_result;
      // 各askに関する最小時刻を比較して最小のものを選ぶ．
      for(auto entry : candidate_map)
      {
        time_result = compare_min_time(time_result, entry.second, entry.first);
      }
      for(auto entry : break_point_list)
      {
        ask_t null_ask;
        time_result = compare_min_time(time_result, entry.second, null_ask);
      }
      /*
      if(opts_->epsilon_mode >= 0){
        time_result = reduce_unsuitable_case(time_result, backend_.get(), phase);
      }*/


      if(time_result.empty())
      {
        phase->simulation_state = simulator::TIME_LIMIT;
        phase->end_time = max_time;
      }
      else
      {
        phase_result_sptr_t pr = phase;

        auto time_it = time_result.begin();
        while(true)
        {
          DCCandidate &candidate = *time_it;
          // 全体を置き換えると，値の上限も下限もない記号定数が消えるので，追加のみを行う
          for(auto par_entry : candidate.parameter_map ){
            pr->parameter_map[par_entry.first] = par_entry.second;
          }
          pr->end_time = pr->current_time + candidate.time;
          backend_->call("simplify", 1, "vln", "vl", &pr->end_time, &pr->end_time);
          if(candidate.time.undefined() || candidate.time.infinite() )
          {
            pr->simulation_state = simulator::TIME_LIMIT;
            pr->end_time = max_time;
          }
          else
          {
            next_todo->discrete_asks = candidate.discrete_asks;
            next_todo->next_pp_candidate_map = pr->next_pp_candidate_map;
            for(auto ask : next_todo->discrete_asks)
            {
              next_todo->next_pp_candidate_map.erase(ask.first);
            }
            next_todo->parameter_map = pr->parameter_map;
            next_todo->parent = pr.get();
            next_todo->prev_map = value_modifier->substitute_time(candidate.time, original_vm);
            next_todo->current_time = pr->end_time;
            pr->todo_list.push_back(next_todo);
          }

          if(++time_it == time_result.end())break;
      
          pr.reset(new PhaseResult(*pr));
          pr->id = ++phase_sum_;
          pr->parent->children.push_back(pr);
          pr->parent->todo_list.push_back(pr);
          pr->todo_list.clear();
          next_todo.reset(new PhaseResult(*next_todo));
          next_todo->id = ++phase_sum_;
        }
      }
    }
  }
  revert_diff(*phase);
}

pp_time_result_t PhaseSimulator::compare_min_time(const pp_time_result_t &existings, const find_min_time_result_t &newcomers, const ask_t& ask)
{
  pp_time_result_t result;
  if(existings.empty())
  {
    for(auto newcomer :newcomers)
    {
      map<ask_t, bool> discrete_asks;
      if(ask.get())discrete_asks[ask] = newcomer.on_time;
      DCCandidate candidate(newcomer.time, discrete_asks,  newcomer.parameter_map);
      result.push_back(candidate);      
    }
  }
  else if(newcomers.empty())
  {
    result = existings;
    return result;
  }
  else
  {
    for(auto newcomer : newcomers)
    {
      for(auto existing: existings)
      {
        compare_min_time_result_t compare_result;
        backend_->call("compareMinTime", 4, "vltvltmpmp", "cp", &existing.time, &newcomer.time, &existing.parameter_map, &newcomer.parameter_map, &compare_result);
        for(auto less_map : compare_result.less_maps)
        {
          DCCandidate candidate(existing.time, existing.discrete_asks, less_map);
          result.push_back(candidate);
        }
        for(auto equal_map : compare_result.equal_maps)
        {
          map<ask_t, bool> discrete_asks = existing.discrete_asks;
          if(ask.get())discrete_asks[ask] = newcomer.on_time;
          DCCandidate candidate(existing.time, discrete_asks, equal_map);
          result.push_back(candidate);
        }
        for(auto greater_map : compare_result.greater_maps)
        {
          map<ask_t, bool> discrete_asks;
          if(ask.get())discrete_asks[ask] = newcomer.on_time;
          DCCandidate candidate(newcomer.time, discrete_asks, greater_map);
          result.push_back(candidate);
        }
      }
    }
  }
  return result;
}


void PhaseSimulator::apply_diff(const PhaseResult &phase)
{
  for(auto diff : phase.module_diff)
  {
    relation_graph_->set_adopted(diff.first, diff.second);
  }
  for(auto positive : phase.diff_positive_asks)
  {
    relation_graph_->set_entailed(positive, true);
  }
  for(auto negative : phase.diff_negative_asks)
  {
    relation_graph_->set_entailed(negative, false);
  }
}

void PhaseSimulator::add_break_point(BreakPoint b)
{
  break_point_list.push_back(make_pair(b, find_min_time_result_t()));
  relation_graph_->add_guard(b.condition);
}

void PhaseSimulator::revert_diff(const PhaseResult &phase)
{
  revert_diff(phase.diff_positive_asks, phase.diff_negative_asks, phase.always_list, phase.module_diff);
}

void PhaseSimulator::revert_diff(const asks_t &positive_asks, const asks_t &negative_asks, const ConstraintStore &always_list, const module_diff_t &module_diff)
{
  for(auto diff : module_diff)
  {
    relation_graph_->set_adopted(diff.first, !diff.second);
  }
  for(auto positive : positive_asks)
  {
    relation_graph_->set_entailed(positive, false);
  }
  for(auto negative : negative_asks)
  {
    relation_graph_->set_entailed(negative, true);
  }
  for(auto always : always_list)
  {
    relation_graph_->set_expanded_atomic(always, false);
  }
}

}
}
