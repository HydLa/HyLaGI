#include <algorithm>
#include <iostream>
#include <sstream>

#include "PhaseSimulator.h"

#include "Logger.h"
#include "Timer.h"

#include "PrevReplacer.h"
#include "VariableReplacer.h"
#include "VariableFinder.h"
#include "PrevSearcher.h"
#include "HydLaError.h"
#include "AlwaysFinder.h"
#include "EpsilonMode.h"
#include "ValueModifier.h"
#include "GuardMapApplier.h"

#include "MinTimeCalculator.h"

#include "Backend.h"

#include "TreeInfixPrinter.h"

#include "IntervalNewton.h"
#include "IntervalTreeVisitor.h"
#include "kv/interval.hpp"
#include "AffineApproximator.h"

#pragma clang diagnostic ignored "-Wpotentially-evaluated-expression"

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

namespace se = symbolic_expression;

typedef interval::itvd itvd;

PhaseSimulator::PhaseSimulator(Simulator* simulator,const Opts& opts): simulator_(simulator), opts_(&opts) {
}

PhaseSimulator::~PhaseSimulator(){}

phase_list_t PhaseSimulator::process_todo(phase_result_sptr_t &todo)
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
    // // for auto abstraction
    // AlwaysFinder always_finder;
    // ConstraintStore non_always;
    // always_set_t always_set;
    // asks_t diff_positives = todo->parent->get_diff_positive_asks();
    // asks_t nonalways_asks;
    // for(auto module : module_set_container->get_max_module_set())
    //   {
    //     always_finder.find_always(module.second, &always_set, &non_always, &diff_positives, &nonalways_asks);
    //   }
    // for(auto constraint : non_always)relation_graph_->set_expanded_atomic(constraint, false);
    // for(auto ask : nonalways_asks)relation_graph_->set_expanded_atomic(ask, false);
    // //
  }
  else
  {
    if(todo->phase_type == INTERVAL_PHASE)
      todo->set_parameter_constraint(todo->parent->get_parameter_constraint());

    if(todo->parent->parent == result_root.get())
    {
      // remove constraints without always at first interval phase
      AlwaysFinder always_finder;
      ConstraintStore non_always;
      always_set_t always_set;
      asks_t diff_positives = todo->parent->get_diff_positive_asks();
      asks_t nonalways_asks;
      for(auto module : module_set_container->get_max_module_set())
      {
        always_finder.find_always(module.second, &always_set, &non_always, &diff_positives, &nonalways_asks);
      }
      for(auto constraint : non_always)relation_graph_->set_expanded_atomic(constraint, false);
      for(auto ask : nonalways_asks)relation_graph_->set_expanded_atomic(ask, false);
    }

    if(todo->phase_type == INTERVAL_PHASE)
    {
      todo->prev_map = todo->parent->variable_map;
    }
  }

  if(todo->phase_type == POINT_PHASE)backend_->call("setCurrentTime", true, 1, "vln", "", &todo->current_time);

  list<phase_result_sptr_t> phase_list = make_results_from_todo(todo);
  if(phase_list.empty())
  {
    todo->simulation_state = INCONSISTENCY;
    todo->set_parameter_constraint(get_current_parameter_constraint());
    todo->parent->children.push_back(todo);
  }
  else
  {
    for(auto phase : phase_list)
    {
      make_next_todo(phase);

      // warn against unreferenced variables
      std::string warning_var_str = "";
      for(auto var: *variable_set_)
      {

        if(var.get_differential_count() == 0 &&
           !phase->variable_map.count(var))warning_var_str += var.get_string() + " ";
      }
      if(warning_var_str.length() > 0)HYDLA_LOGGER_WARN(warning_var_str, " is completely unbound at phase... \n", *phase);

      if(aborting)break;
    }
  }

  todo->profile["PhaseResult"] += phase_timer.get_elapsed_us();
  return phase_list;
}

value_t PhaseSimulator::calculate_middle_value(const phase_result_sptr_t &phase, ValueRange range)
{
  itvd itv = evaluate_interval(phase, range, false);
  double mid_double = (itv.lower() + itv.upper())/2;
  value_t mid_val(mid_double);
  backend_->call("transformToRational", false, 1, "vln", "vl", &mid_val, &mid_val);
  return mid_val;
}

std::list<phase_result_sptr_t> PhaseSimulator::make_results_from_todo(phase_result_sptr_t& todo)
{
  list<phase_result_sptr_t> result_list;
  timer::Timer preprocess_timer;

  backend_->call("resetConstraint", false, 0, "", "");
  HYDLA_LOGGER_DEBUG_VAR(*todo);
  consistency_checker->set_prev_map(&todo->prev_map);
  // add assumptions
  // TODO: If the discrete_guard is not approximated, this process may be redundant
  if(todo->phase_type == INTERVAL_PHASE)
  {
    for(auto it = todo->discrete_guards.begin();it != todo->discrete_guards.end();)
    {
      constraint_t guard = *it;
      VariableFinder finder;
      finder.visit_node(guard);
      bool discrete_changed = false;
      variable_set_t vars_in_guard = finder.get_all_variable_set();
      for(auto var: todo->parent->discrete_differential_set)
      {
        if(vars_in_guard.count(var) > 0)
        {
          discrete_changed = true;
          break;
        }
      }
      if(discrete_changed)
      {
        todo->discrete_guards.erase(it++);
      }
      else ++it;
    }
  }
  

  
  for(auto guard : todo->discrete_guards)
  {
    constraint_t cons = guard;
    backend_->call("makeEquation", false, 1, "en", "e", &cons, &cons);
    backend_->call("addAssumption", true, 1, "ep", "", &cons);
  }

  ConstraintStore parameter_cons = todo->get_parameter_constraint();
  HYDLA_LOGGER_DEBUG_VAR(parameter_cons);
  backend_->call("addParameterConstraint", true, 1, "csn", "", &parameter_cons);
  backend_->call("addParameterConstraint", true, 1, "csn", "", &todo->additional_parameter_constraint);
  backend_->call("addAssumption", true, 1, todo->phase_type == INTERVAL_PHASE?"cst":"csn", "", &todo->additional_constraint_store);

  relation_graph_->set_ignore_prev(todo->phase_type == POINT_PHASE && todo->parent != result_root.get());

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
            todo->add_diff_negative_ask(ask);
          }else
          {
            todo->add_diff_positive_ask(ask);
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
    auto tmp_result_list = simulate_ms(unadopted_ms, todo, nonprev_trigger_asks);
    result_list.merge(tmp_result_list);

    todo->profile[module_sim_string] += ms_timer.get_elapsed_us();

    if(!tmp_result_list.empty() && !opts_->nd_mode)break;
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

// phase = todo
list<phase_result_sptr_t> PhaseSimulator::simulate_ms(const module_set_t& unadopted_ms, phase_result_sptr_t phase, asks_t trigger_asks)
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

  asks_t ms_local_positives, ms_local_negatives;
  ConstraintStore ms_local_always;

  consistency_checker->clear_inconsistent_constraints();
  bool consistent = calculate_closure(phase, trigger_asks, local_diff_sum, ms_local_positives, ms_local_negatives, ms_local_always);
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
    module_set_container->remove_included_ms_by_current_ms();

    if(phase->simulation_state == SIMULATED)
    {
      // branching by multiple maximal consistent modulesets
      // clone the existing phase
      phase.reset(new PhaseResult(*phase));
      phase->id = ++phase_sum_;
      phase->profile.clear();
      phase->parent->todo_list.push_back(phase);
    }

    phase->unadopted_mss.insert(unadopted_ms);
    phase->parent->children.push_back(phase);

    vector<variable_map_t> create_result = consistency_checker->get_result_maps();
    if(create_result.size() == 0)
    {
      throw HYDLA_ERROR("some error occured in creation of variable maps at phase...\n" + phase->get_string());
    }
    else if(create_result.size() > 1)
    {
      throw HYDLA_ERROR("result variable map is not single at phase...\n" + phase->get_string());
    }
    phase->variable_map = create_result[0];
    if(opts_->epsilon_mode > 0){
      //cut_high_order : approx count n > 0.(n=1,2,3...)
      variable_map_t before_map = phase->variable_map;
      value_t before_time = phase->current_time;
      cut_high_order_epsilon(backend_.get(),phase, opts_->epsilon_mode );
      HYDLA_LOGGER_DEBUG("#epsilon before : time : ",before_time);
      for(auto var_entry : before_map)HYDLA_LOGGER_DEBUG("#epsilon before : ",var_entry.first," : ",var_entry.second);
      HYDLA_LOGGER_DEBUG("#epsilon after : time : ",phase->current_time);
      for(auto var_entry : phase->variable_map)HYDLA_LOGGER_DEBUG("#epsilon after : ",var_entry.first," : ",var_entry.second);
    }

    phase->profile["# of CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_count();
    phase->profile["CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_time();
    consistency_checker->reset_count();

    phase->set_parameter_constraint(get_current_parameter_constraint());
    phase->add_diff_positive_asks(ms_local_positives);
    phase->add_diff_negative_asks(ms_local_negatives);

    phase->always_list.add_constraint_store(ms_local_always);
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

        if(max_d_count < 0)
        {
          // This variable is completely unbound
          for(int i = 0; i < max_d_count; i++)
          {
            //insert all entrys
            Variable var(pair.first, i);
            discrete_vs.insert(var);
          }
        }
        else
        {
          Variable var(pair.first, max_d_count);
          if(phase->variable_map.count(var) == 0 || phase->prev_map.count(var) == 0
             || !phase->variable_map[var].unique() || !phase->prev_map[var].unique()
             || !check_equality(phase->variable_map[var].get_unique_value(), phase->prev_map[var].get_unique_value()))
            discrete_vs.insert(var);
        }
      }
      phase->discrete_differential_set = discrete_vs;
    }
  }
  revert_diff(ms_local_positives, ms_local_negatives, ms_local_always, module_diff);
  return result_list;
}

ConstraintStore PhaseSimulator::get_current_parameter_constraint()
{
  ConstraintStore parameter_cons;
  backend_->call("getParameterConstraint", true, 0, "", "cs", &parameter_cons);
  return parameter_cons;
}


bool PhaseSimulator::check_equality(const value_t &lhs, const value_t &rhs){
  // TODO: resume this function with more efficient way
  bool equal;
  backend_->call("exactlyEqual", true, 2, "vlnvln", "b", &lhs, &rhs, &equal);
  return equal;
}


void PhaseSimulator::push_branch_states(phase_result_sptr_t original, CheckConsistencyResult &result){
  phase_result_sptr_t branch_state_false = clone_branch_state(original);
  // copy necesarry information for branching

  branch_state_false->additional_parameter_constraint.add_constraint_store(replace_prev_store(original->parent, result.inconsistent_store));

  original->parent->todo_list.push_back(branch_state_false);
  original->additional_parameter_constraint.add_constraint_store(replace_prev_store(original->parent, result.consistent_store));
  if(original->phase_type == INTERVAL_PHASE)
  {
    original->prev_map = original->parent->variable_map;
    consistency_checker->set_prev_map(&original->prev_map);
  }
  HYDLA_LOGGER_DEBUG_VAR(original->additional_parameter_constraint);
  backend_->call("addParameterConstraint", true, 1, "csn", "", &original->additional_parameter_constraint);
  HYDLA_LOGGER_DEBUG_VAR(*branch_state_false);
  HYDLA_LOGGER_DEBUG_VAR(*original);
}

phase_result_sptr_t PhaseSimulator::clone_branch_state(phase_result_sptr_t original)
{
  phase_result_sptr_t branch_state_false(new PhaseResult());
  branch_state_false->phase_type = original->phase_type;
  branch_state_false->id = ++phase_sum_;
  branch_state_false->step = original->step;
  branch_state_false->current_time = original->current_time;
  branch_state_false->prev_map = original->prev_map;
  branch_state_false->set_parameter_constraint(original->get_parameter_constraint());
  branch_state_false->additional_parameter_constraint = original->additional_parameter_constraint;

  branch_state_false->discrete_differential_set = original->discrete_differential_set;
  branch_state_false->parent = original->parent;
  branch_state_false->discrete_asks = original->discrete_asks;
  branch_state_false->always_list = original->always_list;
  branch_state_false->diff_sum = original->diff_sum;
  branch_state_false->next_pp_candidate_map = original->next_pp_candidate_map;
  return branch_state_false;
}

ConstraintStore PhaseSimulator::replace_prev_store(PhaseResult *parent, ConstraintStore orig)
{
  PrevReplacer replacer(*parent, *simulator_, backend_.get(), opts_->affine);
  ConstraintStore replaced_store;
  for(auto cons : orig)
  {
    replacer.replace_node(cons);
    replaced_store.add_constraint(cons);
  }
  return replaced_store;
}

void PhaseSimulator::check_break_points(phase_result_sptr_t &phase, variable_map_t &variable_map)
{
  if(!break_point_list.empty())
  {
    timer::Timer entailment_timer;
    reset_parameter_constraint(phase->get_parameter_constraint());
    for(auto entry : break_point_list)
    {
      auto break_point = entry.first;
      CheckConsistencyResult cc_result;
      HYDLA_LOGGER_DEBUG_VAR(get_infix_string(break_point.condition));
      variable_map_t related_vm = get_related_vm(break_point.condition, variable_map);
      switch(consistency_checker->check_entailment(related_vm, cc_result, break_point.condition, phase->phase_type, phase->profile)){
      case BRANCH_PAR:
        push_branch_states(phase, cc_result);
        phase->set_parameter_constraint(get_current_parameter_constraint());
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
  time_id = 0;
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
    if(opts_->vars_to_approximate.count(variable.get_string()))
    {
      vars_to_approximate.insert(variable);
    }
  }

  FullInformation root_information;
  root_information.negative_asks = relation_graph_->get_all_asks();
  result_root->set_full_information(root_information);

  interval::AffineApproximator::get_instance()->set_extra_dummy_num(opts_->extra_dummy_num);

  max_time = opts_->max_time;

  aborting = false;

  backend_->set_variable_set(*variable_set_);
  value_modifier.reset(new ValueModifier(*backend_));
}

void PhaseSimulator::replace_prev2parameter(PhaseResult &phase,
                                            variable_map_t &vm)
{
  PrevReplacer replacer(*phase.parent, *simulator_, backend_.get(), opts_->affine);
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
  phase.add_parameter_constraint(replacer.get_parameter_constraint());
  phase.set_parameter_constraint(replace_prev_store(phase.parent, phase.get_parameter_constraint()));
  HYDLA_LOGGER_DEBUG_VAR(phase.get_parameter_constraint());
}



void PhaseSimulator::set_backend(backend_sptr_t back)
{
  backend_ = back;
  consistency_checker.reset(new ConsistencyChecker(backend_));
}

variable_set_t get_discrete_variables(ConstraintStore &diff_sum, PhaseType phase_type)
{
  variable_set_t result;
  for(auto constraint: diff_sum)
  {
    VariableFinder finder;
    finder.visit_node(constraint);
    variable_set_t variables = phase_type==POINT_PHASE?finder.get_variable_set()
      :finder.get_all_variable_set();
    result.insert(variables.begin(), variables.end());
  }
  return result;
}

bool PhaseSimulator::calculate_closure(phase_result_sptr_t& phase, asks_t &trigger_asks, ConstraintStore &diff_sum, asks_t &positive_asks, asks_t &negative_asks, ConstraintStore& expanded_always)
{
  asks_t unknown_asks = trigger_asks;
  bool expanded;
  PhaseType phase_type = phase->phase_type;
  variable_set_t discrete_variables = get_discrete_variables(diff_sum, phase_type);
  bool first = true;

  do{
    HYDLA_LOGGER_DEBUG_VAR(diff_sum);
    expanded = false;
    timer::Timer entailment_timer;

    set<ask_t> adjacents;
    if(phase->parent == result_root.get() && first)
    {
      adjacents = relation_graph_->get_all_asks();
      first = false;
    }
    else if(phase->phase_type == POINT_PHASE || !phase->in_following_step()){
      for(auto variable : discrete_variables)
      {
        auto each_adjacents = relation_graph_->get_adjacent_asks(variable.get_name(), phase_type == POINT_PHASE);
        adjacents.insert(each_adjacents.begin(), each_adjacents.end());
      }
    }else{
      for(auto variable : discrete_variables)
      {
        if(phase->parent->discrete_differential_set.count(variable))
        {
          auto each_adjacents = relation_graph_->get_adjacent_asks2var_and_derivatives(variable, phase_type == POINT_PHASE);
          adjacents.insert(each_adjacents.begin(), each_adjacents.end());
        }
      }
    }
    discrete_variables.clear();
    ConstraintStore local_diff_sum;
    for(auto adjacent : adjacents)
    {
      unknown_asks.insert(adjacent);
    }
    for(auto ask_it = unknown_asks.begin(); ask_it != unknown_asks.end() && !expanded;)
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
      switch(consistency_checker->check_entailment(*relation_graph_, check_consistency_result, ask->get_guard(), ask->get_child(), unknown_asks, phase_type, phase->profile)){

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
          local_diff_sum.add_constraint(ask->get_child());
          relation_graph_->set_entailed(ask, true);
        }
        expanded = true;
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
          local_diff_sum.add_constraint(ask->get_child());
          relation_graph_->set_entailed(ask, false);
        }
        expanded = true;
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
    // loop until no branching occurs
    discrete_variables = get_discrete_variables(local_diff_sum, phase_type);
  }while(expanded);
  
  while(true)
  {
    timer::Timer consistency_timer;
    CheckConsistencyResult cc_result;
    cc_result = consistency_checker->check_consistency(*relation_graph_, diff_sum, phase_type, phase->profile, unknown_asks, phase->in_following_step());
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

  if(!unknown_asks.empty()){
    phase_result_sptr_t branch_state_false = clone_branch_state(phase);
    constraint_t unknown_guard = (*unknown_asks.begin())->get_guard();
    branch_state_false->additional_constraint_store.add_constraint(constraint_t(new Not(unknown_guard)));
    phase->parent->todo_list.push_back(branch_state_false);
    phase->additional_constraint_store.add_constraint(unknown_guard);
    HYDLA_LOGGER_DEBUG_VAR(branch_state_false->additional_constraint_store);
    backend_->call("addAssumption", true, 1, phase->phase_type==INTERVAL_PHASE?"et":"en", "", &unknown_guard);
    return calculate_closure(phase, trigger_asks, diff_sum, positive_asks, negative_asks, expanded_always);
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

ValueRange PhaseSimulator::create_range_from_interval(itvd itv)
{
  value_t lower(itv.lower()), upper(itv.upper());
  backend_->call("transformToRational", false, 1, "vln", "vl", &lower, &lower);
  backend_->call("transformToRational", false, 1, "vln", "vl", &upper, &upper);
  return ValueRange(lower, upper);
}

find_min_time_result_t PhaseSimulator::find_min_time(const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed, phase_result_sptr_t &phase, std::map<std::string, HistoryData>& atomic_guard_min_time_interval_map)
{
  if(opts_->step_by_step)return find_min_time_step_by_step(guard, original_vm, time_limit, phase, entailed, atomic_guard_min_time_interval_map);
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
  std::list<AtomicConstraint *> guards = relation_graph_->get_atomic_guards(guard);
  list<Parameter> parameters;
  constraint_t guard_by_newton;
  constraints_t other_guards;

  HYDLA_LOGGER_DEBUG_VAR(phase->discrete_guards.size());

  for(auto guard : phase->discrete_guards)
  {
    HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
  }

  for(auto atomic_guard : guards)
  {
    constraint_t g = atomic_guard->constraint;

    if(!guard_time_map.count(g))
    {
      variable_map_t related_vm = get_related_vm(g, original_vm);
      
      bool by_newton = false;
      if(opts_->interval && guard_by_newton.get() == nullptr)
      {
        if(opts_->guards_to_interval_newton.empty())
        {
          cout << "apply Interval Newton method to " << get_infix_string(g) << "?('y' or 'n')" << endl;
          char c;
          cin >> c;
          if(c == 'y')
          {
            guard_by_newton = g;
            by_newton = true;
          }
        }
        else
        {
          // TODO: avoid string comparison
          if(opts_->guards_to_interval_newton.count(get_infix_string(g)))
          {
            guard_by_newton = g;
            by_newton = true;
          }
        }
      }
      if(!by_newton)
      {
        constraint_t cons = g;
        constraint_t constraint_for_this_guard;
        variable_map_t related_vm = get_related_vm(g, original_vm);
        backend_->call("substituteVM", true, 3, "etmvtvlt", "e", &cons, &related_vm, &phase->current_time, &cons);

        value_t lower("0");
        backend_->call("calculateConsistentTime", true, 2, "etvlt", "e", &cons, &lower, &constraint_for_this_guard);
        other_guards.insert(g);
        guard_time_map[g] = constraint_for_this_guard;
      }
    }
  }

  find_min_time_result_t min_time_for_this_ask;
  if(guard_by_newton.get() != nullptr)
  {
    HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard_by_newton));

    constraint_t time_guard_by_newton;
    variable_map_t related_vm = get_related_vm(guard_by_newton, original_vm);
    backend_->call("substituteVM", true, 3, "etmvtvlt", "e", &guard_by_newton, &related_vm, &phase->current_time, &time_guard_by_newton);


    // TODO: deal with multiple parameter maps
    vector<parameter_map_t> parameter_map_vector = phase->get_parameter_maps();
    HYDLA_ASSERT(parameter_map_vector.size() <= 1);
    parameter_map_t pm = parameter_map_vector.size()==1?parameter_map_vector.front():parameter_map_t();
    list<itvd> result_interval_list = calculate_interval_newton_nd(time_guard_by_newton, pm);
    const type_info &guard_type = typeid(*guard_by_newton);

    // TODO: integrate process for both equalities and inequalities
    if(guard_type == typeid(Equal) || guard_type == typeid(UnEqual))
    {
      Parameter parameter_prev("t", -1, ++time_id);
      Parameter parameter_current("t", -1, ++time_id);
      itvd prev_interval = itvd(0, 0);
      bool empty = false;
      while(true)
      {
        itvd current_interval;
        if(result_interval_list.empty())
        {
          empty = true;
          current_interval = upper_bound_of_itv_newton;
        }
        else
        {
          current_interval = result_interval_list.front();
          result_interval_list.pop_front();
        }

        // add temporary parameters for parameter_prev and parameter_current
        parameter_map_t pm_for_each = pm;
        ValueRange prev_range = create_range_from_interval(prev_interval);
        ValueRange current_range = create_range_from_interval(current_interval);
        pm_for_each[parameter_prev] = prev_range;
        pm_for_each[parameter_current] = current_range;

        backend_->call("resetConstraintForParameter", false, 1, "mp", "", &pm_for_each);

        if(empty)
        {
          if(guard_type == typeid(UnEqual)) guard_time_map[guard_by_newton] = constraint_t(new True());
          else guard_time_map[guard_by_newton] = constraint_t(new False());
        }
        else if(guard_type == typeid(UnEqual))
        {
          guard_time_map[guard_by_newton] = constraint_t(new UnEqual(new se::Parameter(parameter_current), new SymbolicT()));
        }
        else
        {
          guard_time_map[guard_by_newton] = constraint_t(new Equal(new se::Parameter(parameter_current), new SymbolicT()));
        }
        constraint_t lb, ub;
        lb.reset(new LessEqual(new se::Parameter(parameter_prev), new SymbolicT()));
        ub.reset(new LessEqual(new SymbolicT(), new se::Parameter(parameter_current)));
        constraint_t time_bound;
        // prev_t < t /\ t < current_t
        time_bound.reset(new LogicalAnd(lb, ub));

        min_time_for_this_ask = min_time_calculator.calculate_min_time(&guard_time_map, guard, entailed, time_limit, time_bound);
        // TODO: deal with  branching of cases
        if(!min_time_for_this_ask.empty())
        {
          HYDLA_ASSERT(min_time_for_this_ask.size() == 1);
          add_parameter_constraint(phase, parameter_current, current_range);
          min_time_for_this_ask.front().range_by_newton = current_range;
          break;
        }
        else if(empty)break;
        prev_interval = current_interval;
      }
    }
    else
    {
      HYDLA_ASSERT(guard_type == typeid(se::LessEqual) ||
             guard_type == typeid(se::Less) ||
             guard_type == typeid(se::Greater) ||
             guard_type == typeid(se::GreaterEqual));

      Parameter parameter_prev("t", -1, ++time_id);
      Parameter parameter_lower("t", -1, ++time_id);
      Parameter parameter_upper("t", -1, ++time_id);
      itvd prev_interval = itvd(0, 0);
      bool include_border = (guard_type == typeid(se::LessEqual) || guard_type == typeid(se::GreaterEqual));

      itvd lower_interval, upper_interval;

      bool last_trial = false;
      bool negated = false;
      if(entailed)
      {
        lower_interval = itvd(0,0);
      }
      else
      {
        if(result_interval_list.empty()) negated = true;
        else
        {
          lower_interval = result_interval_list.front();
          result_interval_list.pop_front();
        }
      }
      while(true)
      {
        if(!result_interval_list.empty())
        {
          upper_interval = result_interval_list.front();
          result_interval_list.pop_front();
        }
        else
        {
          upper_interval = upper_bound_of_itv_newton;
          last_trial = true;
        }

        // add temporary parameters for parameter_prev and parameter_current
        parameter_map_t pm_for_each = pm;
        ValueRange prev_range = create_range_from_interval(prev_interval);
        ValueRange lower_range = create_range_from_interval(lower_interval);
        ValueRange upper_range = create_range_from_interval(upper_interval);
        pm_for_each[parameter_prev] = prev_range;
        pm_for_each[parameter_lower] = lower_range;
        pm_for_each[parameter_upper] = upper_range;
        backend_->call("resetConstraintForParameter", false, 1, "mp", "", &pm_for_each);

        {
          constraint_t lb, ub;
          if(include_border)
          {
            lb.reset(new LessEqual(new se::Parameter(parameter_lower), new SymbolicT()));
            ub.reset(new LessEqual(new SymbolicT(), new se::Parameter(parameter_upper)));
          }
          else
          {
            lb.reset(new Less(new se::Parameter(parameter_lower), new SymbolicT()));
            ub.reset(new Less(new SymbolicT(), new se::Parameter(parameter_upper)));
          }
          if(!negated)  guard_time_map[guard_by_newton] = constraint_t(new LogicalAnd(lb, ub));
          else  guard_time_map[guard_by_newton] = constraint_t(new Not(new LogicalAnd(lb, ub)));
        }

        constraint_t time_bound;
        {
          constraint_t lb, ub;
          if(negated) lb.reset(new LessEqual(new se::Parameter(parameter_lower), new SymbolicT()));
          else lb.reset(new LessEqual(new se::Parameter(parameter_prev), new SymbolicT()));
          ub.reset(new Less(new SymbolicT(), new se::Parameter(parameter_upper)));

          // prev_t < t /\ t < current_t
          time_bound.reset(new LogicalAnd(lb, ub));
        }

        min_time_for_this_ask = min_time_calculator.calculate_min_time(&guard_time_map, guard, entailed, time_limit, time_bound);
        // TODO: deal with  branching of cases
        if(!min_time_for_this_ask.empty() && !min_time_for_this_ask.front().time.infinite())
        {
          HYDLA_ASSERT(min_time_for_this_ask.size() == 1);
          simulator_->introduce_parameter(parameter_upper, upper_range);
          simulator_->introduce_parameter(parameter_lower, lower_range);
          ConstraintStore new_store = phase->get_parameter_constraint();
          new_store.add_constraint_store(lower_range.create_range_constraint(node_sptr(new se::Parameter(parameter_lower))));
          new_store.add_constraint_store(upper_range.create_range_constraint(node_sptr(new se::Parameter(parameter_upper))));
          phase->set_parameter_constraint(new_store);
          backend_->call("resetConstraintForParameter", false, 1, "csn", "", &new_store);

          min_time_for_this_ask.front().range_by_newton = ValueRange(value_t(parameter_lower), value_t(parameter_upper));
          break;
        }
        prev_interval = upper_interval;
        if(last_trial)break;

        if(!result_interval_list.empty())
        {
          lower_interval = result_interval_list.front();
          result_interval_list.pop_front();
        }
        else
        {
          // deal with remaining interval ( ub of upper_interval -> upper_bound_of_itv_newton)
          negated = true;
          lower_interval = upper_interval;
        }
      }
    }
    HYDLA_ASSERT(min_time_for_this_ask.size() <= 1);
    if(min_time_for_this_ask.size() == 1)min_time_for_this_ask.front().guard_by_newton = guard_by_newton;
  }
  else
  {
    min_time_for_this_ask = min_time_calculator.calculate_min_time(&guard_time_map, guard, entailed, time_limit);
  }

  if(opts_->numerize_mode || opts_->epsilon_mode || opts_->interval > 0)
  {
    for(auto &each_case: min_time_for_this_ask)
    {
      for(auto guard : other_guards)
      {
        each_case.other_guards_to_time_condition[guard] = guard_time_map[guard];
      }
    }
  }

  return min_time_for_this_ask;
}

  
struct PhaseSimulator::StateOfIntervalNewton{
  interval::itv_stack_t  stack;
  boost::optional<itvd>  min_interval = boost::none;
  node_sptr guard;
  node_sptr exp;
  node_sptr dexp;
};


PhaseSimulator::StateOfIntervalNewton PhaseSimulator::initialize_newton_state(const constraint_t& time_guard, parameter_map_t &pm)
{
  StateOfIntervalNewton state;
  backend_->call("relationToFunction", true, 1, "et", "e", &time_guard, &state.exp);
  backend_->call("differentiateWithTime", true, 1, "et", "e", &state.exp, &state.dexp);
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(state.exp));
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(state.dexp));

  itvd init = itvd(0,upper_bound_of_itv_newton);
  HYDLA_LOGGER_DEBUG_VAR(init);
  state.stack.push(init);
  return state;
}


find_min_time_result_t PhaseSimulator::find_min_time_step_by_step(const constraint_t &guard, variable_map_t &original_vm, Value &time_limit, phase_result_sptr_t &phase, bool entailed, std::map<std::string, HistoryData>& atomic_guard_min_time_interval_map)
{
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
  std::list<AtomicConstraint *> guards = relation_graph_->get_atomic_guards(guard);
  list<Parameter> parameters;
  HYDLA_LOGGER_DEBUG_VAR(phase->discrete_guards.size());

  for(auto guard : phase->discrete_guards)
  {
    HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
  }


  map<constraint_t, value_t> atomic_guard_time_map;
  map<constraint_t, std::list<value_t> > symbolic_guard_times_map;
  map<constraint_t, StateOfIntervalNewton> newton_guard_state_map;
  map<constraint_t, bool> atomic_guard_map;

  vector<parameter_map_t> parameter_map_vector = phase->get_parameter_maps();
  HYDLA_ASSERT(parameter_map_vector.size() <= 1);
  parameter_map_t pm = parameter_map_vector.size()==1?parameter_map_vector.front():parameter_map_t();
  for(auto atomic_guard : guards)
  {
    constraint_t g = atomic_guard->constraint;
    variable_map_t related_vm = get_related_vm(g, original_vm);
    bool by_newton = false;
    if(opts_->interval)
    {
      if(opts_->guards_to_interval_newton.empty())
      {
        cout << "apply Interval Newton method to " << get_infix_string(g) << "?('y' or 'n')" << endl;
        char c;
        cin >> c;
        if(c == 'y')
        {
          by_newton = true;
        }
      }
      else
      {
        // TODO: avoid string comparison
        if(opts_->guards_to_interval_newton.count(get_infix_string(g)))
        {
          by_newton = true;
        }
      }
    }
    value_t lower("0");
    constraint_t time_guard;
    backend_->call("substituteVM", true, 3, "etmvtvlt", "e", &g, &related_vm, &phase->current_time, &time_guard);
    if(typeid(*(time_guard)) == typeid(True))
    {
      //TODO: consider to improve efficiency by exploiting the information that this guard will be forever true
      atomic_guard_map[g] = true;
    }
    else if(typeid(*(time_guard)) == typeid(False))
    {
      atomic_guard_map[g] = false;
    }
    else
    {
      if(by_newton)
      {
        StateOfIntervalNewton state = initialize_newton_state(time_guard, pm);
        lower = value_t(state.stack.top().lower());
        newton_guard_state_map[g] = state;
      }
      else
      {
        find_min_time_result_t time_list_for_this_guard;
        backend_->call("solveTimeEquation", true, 2, "etvlt", "f", &time_guard, &lower, &time_list_for_this_guard);
        std::list<value_t> symbolic_time_list;
        for(auto candidate : time_list_for_this_guard)
        {
          symbolic_time_list.push_back(candidate.time);
        }
        symbolic_guard_times_map[g] = symbolic_time_list;
      }

      const type_info &guard_type = typeid(*(atomic_guard->constraint));
      if(guard_type == typeid(Equal) || guard_type == typeid(UnEqual))
      {
        atomic_guard_map[atomic_guard->constraint] = (guard_type == typeid(Equal));
      }
      else
      {
        bool true_at_initial_time;
        backend_->call("trueAtInitialTime", true, 2, "etvlt", "b", &time_guard, &lower, &true_at_initial_time);
        atomic_guard_map[atomic_guard->constraint] = true_at_initial_time;
      }
    }
  }

  ConstraintStore current_parameter_constraint  = phase->get_parameter_constraint();
  find_min_time_result_t min_time_for_this_ask;
  int loopCount = 0;

  //std::map<std::string, HistoryData> current_atomic_guard_history_data = atomic_guard_min_time_interval_map;

  //Atomic guard index -> Interval newton initial value index
  std::map<int, int> current_atomic_guard_index_history_stack_index;
  for (auto it = newton_guard_state_map.begin(); it != newton_guard_state_map.end(); ++it)
  {
    const int index = std::distance(newton_guard_state_map.begin(), it);
    const auto& entry = *it;

    const std::string atomic_guard_str = get_infix_string(entry.first);
    if (atomic_guard_min_time_interval_map.find(atomic_guard_str) != atomic_guard_min_time_interval_map.end())
    {
      current_atomic_guard_index_history_stack_index[index] = 0;
    }
  }

  while(true)
  {
    HYDLA_LOGGER_DEBUG("BREAK1====================================================");
    HYDLA_LOGGER_DEBUG_VAR(++loopCount);
    HYDLA_LOGGER_DEBUG("BREAK1 loopCount:", std::to_string(loopCount));
    vector<TimeListElement > time_list;
    parameter_map_t pm_for_each = pm;
    map<constraint_t, Parameter> guard_parameter_map;
    for(auto entry : symbolic_guard_times_map)
    {
      if(entry.second.empty())continue;
      TimeListElement elem(entry.second.front(), entry.first);
      time_list.push_back(elem);
    }
    //for(auto &entry : newton_guard_state_map)
    for (auto it = newton_guard_state_map.begin(); it != newton_guard_state_map.end(); ++it)
    {
      const int atomic_guard_index = std::distance(newton_guard_state_map.begin(), it);
      auto& entry = *it;
      bool needUpdate = true;
      StateOfIntervalNewton &state = entry.second;
      const std::string atomic_guard_str = get_infix_string(entry.first);
      HYDLA_LOGGER_DEBUG("BREAK Guard Adress:", std::to_string(reinterpret_cast<unsigned long long>(entry.first.get())), "  Guard Value:", atomic_guard_str);

      optional<IntervalNewtonResult> new_data_opt;

      if(!state.min_interval)
      {
        const bool generate_new_data = current_atomic_guard_index_history_stack_index.find(atomic_guard_index) == current_atomic_guard_index_history_stack_index.end();
        if (!generate_new_data)
        {
          const int stack_index = current_atomic_guard_index_history_stack_index[atomic_guard_index];

          auto& stack_states = atomic_guard_min_time_interval_map[atomic_guard_str].results;
          if (stack_states.size() == stack_index)
          {
            HYDLA_LOGGER_DEBUG("BREAK1 interval newton PROGRESS");
            IntervalNewtonResult new_data;
            new_data.current_stack_top = std::make_shared<kv::interval<double>>(state.stack.top());

            {
              HYDLA_LOGGER_DEBUG("BREAK2 Before Interval Newton");
              interval::itv_stack_t printState = state.stack;
              int stackCount = static_cast<int>(state.stack.size());
              while (!printState.empty())
              {
                --stackCount;
                const itvd v = printState.top();
                printState.pop();

                HYDLA_LOGGER_DEBUG("BREAK2", v);
              }
            }

            state.min_interval =
              interval::calculate_interval_newton(state.stack, state.exp, state.dexp, pm, !phase->in_following_step() || phase->discrete_guards.count(entry.first));

            {
              HYDLA_LOGGER_DEBUG("BREAK2 After Interval Newton");
              interval::itv_stack_t printState = state.stack;
              int stackCount = static_cast<int>(state.stack.size());
              while (!printState.empty())
              {
                --stackCount;
                const itvd v = printState.top();
                printState.pop();

                HYDLA_LOGGER_DEBUG("BREAK2", v);
              }
            }

            new_data.min_interval = std::make_shared<kv::interval<double>>(state.min_interval.value());
            new_data.next_stack = state.stack;

            HYDLA_LOGGER_DEBUG("BREAK2 Calculated interval : ", state.min_interval.value());

            new_data_opt = new_data;
          }
          else
          {
            HYDLA_LOGGER_DEBUG("BREAK1 interval newton USE PRECOMPUTED VALUE");

            state.stack = stack_states[stack_index].next_stack;

            //state.min_interval = optional<kv::interval<double>>(*stack_states[stack_index].min_interval);
            state.min_interval = *stack_states[stack_index].min_interval;

            HYDLA_LOGGER_DEBUG("BREAK2 Got interval : ", state.min_interval.value());
          }
        }
        else
        {
          HYDLA_LOGGER_DEBUG("BREAK1 interval newton CALUCULATE NEW VALUE");

          for (auto guard : phase->discrete_guards)
          {
            HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
          }
          HYDLA_LOGGER_DEBUG("1234567890");
          HYDLA_LOGGER_DEBUG_VAR(atomic_guard_str);
          HYDLA_LOGGER_DEBUG_VAR(phase->discrete_guards.count(entry.first));
          if (atomic_guard_min_time_interval_map.find(atomic_guard_str) == atomic_guard_min_time_interval_map.end(), true)
          {
            {
              HYDLA_LOGGER_DEBUG("BREAK2 Before Interval Newton");
              interval::itv_stack_t printState = state.stack;
              int stackCount = static_cast<int>(state.stack.size());
              while (!printState.empty())
              {
                --stackCount;
                const itvd v = printState.top();
                printState.pop();

                HYDLA_LOGGER_DEBUG("BREAK2", v);
              }
            }

            IntervalNewtonResult new_data;
            new_data.current_stack_top = std::make_shared<kv::interval<double>>(state.stack.top());

            state.min_interval =
              interval::calculate_interval_newton(state.stack, state.exp, state.dexp, pm, !phase->in_following_step() || phase->discrete_guards.count(entry.first));
            //atomic_guard_min_time_interval_map[atomic_guard_str].min_interval = std::make_shared<kv::interval<double>>(state.min_interval.value());
            new_data.min_interval = std::make_shared<kv::interval<double>>(state.min_interval.value());
            new_data.next_stack = state.stack;

            HYDLA_LOGGER_DEBUG("BREAK2 Calculated interval : ", state.min_interval.value());

            new_data_opt = new_data;

            {
              HYDLA_LOGGER_DEBUG("BREAK2 After Interval Newton");
              interval::itv_stack_t printState = state.stack;
              int stackCount = static_cast<int>(state.stack.size());
              while (!printState.empty())
              {
                --stackCount;
                const itvd v = printState.top();
                printState.pop();

                HYDLA_LOGGER_DEBUG("BREAK2", v);
              }
            }
          }
          /*else
          {
            HYDLA_LOGGER_DEBUG("using precomputed min time");
            state.min_interval = *atomic_guard_min_time_interval_map[atomic_guard_str].min_interval;
            //needUpdate = false;
            //continue;
          }*/
        }
      }
      if(*state.min_interval == interval::INVALID_ITV)continue;
      if(opts_->numerize_mode)
      {
        *state.min_interval = mid(*state.min_interval);
      }

      if (new_data_opt)
      {
        Parameter parameter("t", -1, ++time_id);
        TimeListElement elem;

        value_t new_value_for_save = value_t(parameter);
        bool isAffine = false;

        if (opts_->affine)
        {
          hydla::backend::CalculateTLinearResult ct;
          value_t lb = state.min_interval->lower();
          value_t ub = state.min_interval->upper();

          backend_->call("calculateTLinear", true, 4, "etmpvltvlt", "ct", &state.exp, &pm, &lb, &ub, &ct);
          ValueRange range;
          range.set_upper_bound(value_t("1"), true);
          range.set_lower_bound(value_t("-1"), true);
          value_t new_value = ct.exp;
          new_value += value_t(ct.mid_rad.midpoint + ct.mid_rad.radius * value_t(parameter));
          pm_for_each[parameter] = range;
          HYDLA_LOGGER_DEBUG_VAR(new_value);
          elem = TimeListElement(new_value, entry.first);
          new_value_for_save = new_value;
          isAffine = true;
        }
        else
        {
          elem = TimeListElement(value_t(parameter), entry.first);
          ValueRange range = create_range_from_interval(*state.min_interval);
          pm_for_each[parameter] = range;
        }
        guard_parameter_map.insert(make_pair(entry.first, parameter));
        time_list.push_back(elem);

        //atomic_guard_min_time_interval_map[atomic_guard_str].time_id = time_id;
        //atomic_guard_min_time_interval_map[atomic_guard_str].elem = elem;

        auto& new_data = new_data_opt.value();

        new_data.time_id = time_id;
        //new_data.time_list_element_time = elem.time;
        new_data.time_list_element_time = new_value_for_save;
        new_data.isAffine = isAffine;

        atomic_guard_min_time_interval_map[atomic_guard_str].results.push_back(new_data);
        if (current_atomic_guard_index_history_stack_index.find(atomic_guard_index) == current_atomic_guard_index_history_stack_index.end())
        {
          HYDLA_LOGGER_DEBUG("BREAK set new history stack index:");
          current_atomic_guard_index_history_stack_index[atomic_guard_index] = 0;
        }

        std::string cons_str;
        for (auto& cons : elem.parameter_constraint)
        {
          cons_str += get_infix_string(cons) + ", ";
        }

        HYDLA_LOGGER_DEBUG("BREAK time_id:", std::to_string(time_id));
        HYDLA_LOGGER_DEBUG("BREAK elem.time:", elem.time.get_string(), "  elem.guard:", get_infix_string(elem.guard), "  elem.constraint:", cons_str);
      }
      else
      {
        const int stack_index = current_atomic_guard_index_history_stack_index[atomic_guard_index];
        auto& stack_states = atomic_guard_min_time_interval_map[atomic_guard_str].results;

        Parameter parameter("t", -1, stack_states[stack_index].time_id);
        TimeListElement elem;

        //elem = TimeListElement(value_t(parameter), entry.first);
        elem = TimeListElement(stack_states[stack_index].time_list_element_time, entry.first);

        if (stack_states[stack_index].isAffine)
        {
          ValueRange range;
          range.set_upper_bound(value_t("1"), true);
          range.set_lower_bound(value_t("-1"), true);
          pm_for_each[parameter] = range;
        }
        else
        {
          ValueRange range = create_range_from_interval(*state.min_interval);
          pm_for_each[parameter] = range;
        }
        /*if (pm_for_each.find(parameter) == pm_for_each.end())
        {
          
        }*/

        guard_parameter_map.insert(make_pair(entry.first, parameter));
        time_list.push_back(elem);

        //Parameter parameter("t", -1, atomic_guard_min_time_interval_map[atomic_guard_str].time_id);
        //guard_parameter_map.insert(make_pair(entry.first, parameter));
        //time_list.push_back(atomic_guard_min_time_interval_map[atomic_guard_str].elem);
      }
    }

    for (const auto& parameter_range : pm_for_each)
    {
      HYDLA_LOGGER_DEBUG("BREAK parameter:", parameter_range.first.to_string(), " -> ", parameter_range.second.get_string());
    }
    if (time_list.empty()) { HYDLA_LOGGER_DEBUG("BREAK"); break; }
    find_min_time_result_t minimum_candidates;
    backend_->call("resetConstraintForParameter", false, 1, "mp", "", &pm_for_each);
    backend_->call("getMinimum", true, 1, "tl", "f", &time_list, &minimum_candidates);
    //TODO: deal with case branching
    HYDLA_ASSERT(minimum_candidates.size() == 1);
    for(auto &candidate : minimum_candidates)
    {
      std::list<constraint_t> guard_list;
      candidate.guard_indices.sort(greater<int>());
      for(int index : candidate.guard_indices)
      {
        HYDLA_LOGGER_DEBUG("BREAK1 candidate.guard:", get_infix_string(time_list[index].guard));
        guard_list.push_back(time_list[index].guard);
        constraint_t guard = (time_list.begin() + index)->guard;
        auto newton_it = newton_guard_state_map.find(guard);
        if (newton_it != newton_guard_state_map.end())
        {
          const auto current_min_interval = newton_it->second.min_interval.value();
          HYDLA_LOGGER_DEBUG("BREAK1 min interval:", current_min_interval);
          newton_it->second.min_interval = boost::none;

          const int atomic_guard_index = std::distance(newton_guard_state_map.begin(), newton_it);
          //auto& entry = *newton_it;
          //StateOfIntervalNewton &state = entry.second;
          //const std::string atomic_guard_str = get_infix_string(entry.first);
          ++current_atomic_guard_index_history_stack_index[atomic_guard_index];

          /*const std::string atomic_guard_str = get_infix_string(time_list[index].guard);
          if (atomic_guard_min_time_interval_map.find(atomic_guard_str) != atomic_guard_min_time_interval_map.end())
          {
            atomic_guard_min_time_interval_map.erase(atomic_guard_str);
          }*/
        }
        else symbolic_guard_times_map[guard].pop_front();
      }
      bool on_time;
      bool satisfied = checkAndUpdateGuards(atomic_guard_map, guard, guard_list, on_time, entailed);
      ;
      HYDLA_LOGGER_DEBUG("BREAK1 satisfied? :", satisfied ? "true" : "false");
      if(satisfied)
      {
        FindMinTimeCandidate new_candidate;
        new_candidate.time = candidate.time;
        new_candidate.discrete_guards = guard_list;
        new_candidate.parameter_constraint = candidate.parameter_constraint;
        new_candidate.on_time = on_time;
        min_time_for_this_ask.push_back(new_candidate);
        HYDLA_ASSERT(guard_list.size() == 1);
        auto guard_it = guard_parameter_map.find(guard_list.front());
        if(guard_it != guard_parameter_map.end())
        {
          Parameter new_param = guard_it->second;

          const auto& simulator_parameter_map = simulator_->get_parameter_map();
          if (simulator_parameter_map.find(new_param) == simulator_parameter_map.end())
          {
            simulator_->introduce_parameter(new_param, pm_for_each[new_param]);
          }
        }
        backend_->call("productWithNegatedConstraint", true, 2, "csncsn", "cs", &current_parameter_constraint, &candidate.parameter_constraint, &current_parameter_constraint);
        HYDLA_LOGGER_DEBUG_VAR(current_parameter_constraint.consistent());

        if (!current_parameter_constraint.consistent())
        {
          //HYDLA_LOGGER_DEBUG("BREAK Finish loopCount:", std::to_string(loopCount));
          HYDLA_LOGGER_DEBUG("BREAK Finish:");

          HYDLA_LOGGER_DEBUG("BREAK newton_guard_state_map:");

          for (auto &entry : newton_guard_state_map)
          {
            StateOfIntervalNewton &state = entry.second;
            const std::string atomic_guard_str = get_infix_string(entry.first);

            HYDLA_LOGGER_DEBUG("BREAK atomic_guard_str: ", atomic_guard_str);

            if (state.min_interval)
            {
              HYDLA_LOGGER_DEBUG("BREAK state.min_interval: ", state.min_interval.value());
            }
            else
            {
              HYDLA_LOGGER_DEBUG("BREAK state.min_interval is not exist");
            }
          }

          for (auto &current_param : current_parameter_constraint)
          {
            //HYDLA_LOGGER_DEBUG("BREAK current_parameter_constraint: ", get_infix_string(current_param));
          }

          for (auto &cand_param : candidate.parameter_constraint)
          {
            //HYDLA_LOGGER_DEBUG("BREAK candidate.parameter_constraint: ", get_infix_string(cand_param));
          }

          HYDLA_LOGGER_DEBUG("BREAK");
          break;
        }
      }
      else
      {
        //HYDLA_LOGGER_DEBUG("BREAK loopCount:", std::to_string(loopCount));

        HYDLA_LOGGER_DEBUG("BREAK newton_guard_state_map:");

        for (auto &entry : newton_guard_state_map)
        {
          StateOfIntervalNewton &state = entry.second;
          const std::string atomic_guard_str = get_infix_string(entry.first);

          HYDLA_LOGGER_DEBUG("BREAK atomic_guard_str: ", atomic_guard_str);

          if (state.min_interval)
          {
            HYDLA_LOGGER_DEBUG("BREAK state.min_interval: ", state.min_interval.value());
          }
          else
          {
            HYDLA_LOGGER_DEBUG("BREAK state.min_interval is not exist");
          }
        }

        for (auto &current_param : current_parameter_constraint)
        {
          //HYDLA_LOGGER_DEBUG("BREAK current_parameter_constraint: ", get_infix_string(current_param));
        }

        for (auto &cand_param : candidate.parameter_constraint)
        {
          //HYDLA_LOGGER_DEBUG("BREAK candidate.parameter_constraint: ", get_infix_string(cand_param));
        }
      }
    }
    if (!current_parameter_constraint.consistent())
    {
      HYDLA_LOGGER_DEBUG("BREAK");
      break;
    }
  }
  //HYDLA_LOGGER_DEBUG_VAR(candidate.time);
  HYDLA_LOGGER_DEBUG("BREAK Print min_time_for_this_ask");
  for (const auto& element : min_time_for_this_ask)
  {
    std::string guards_str;
    for (const auto& guard : element.discrete_guards)
    {
      guards_str += get_infix_string(guard) + ", ";
    }
    HYDLA_LOGGER_DEBUG("BREAK Guards:", guards_str);
    HYDLA_LOGGER_DEBUG("BREAK Time:", element.time.get_string());
  }
  return min_time_for_this_ask;
}

bool PhaseSimulator::checkAndUpdateGuards(map<constraint_t, bool> &guard_map, constraint_t guard, list<constraint_t> guard_list, bool &on_time, bool entailed)
{
  map<constraint_t, bool> guard_map_prev = guard_map;
  GuardMapApplier applier;
  bool satisfied = false;
  for(auto &atomic_guard : guard_list)
  {
    const type_info &guard_type = typeid(*(atomic_guard));
    if(guard_type == typeid(Equal) || guard_type == typeid(LessEqual) || guard_type == typeid(GreaterEqual))
    {
      guard_map[atomic_guard] = true;
    }
    else guard_map[atomic_guard] = false;
  }
  
  constraint_t applied_guard = applier.apply(guard, &guard_map);
  backend_->call("isGuardSatisfied", true, 1, "en", "b", &applied_guard, &satisfied);
  if((!entailed && satisfied) || (entailed && !satisfied))
  {
    on_time = true;
    return true;
  }
  for(auto &atomic_guard : guard_list)
  {
    const type_info &guard_type = typeid(*(atomic_guard));
    if(guard_type == typeid(Equal))
    {
      guard_map[atomic_guard] = false;
    }
    else if(guard_type == typeid(UnEqual))
    {
      guard_map[atomic_guard] = true;
    }
    else
    {
      guard_map[atomic_guard] = !guard_map_prev[atomic_guard];
    }
  }
  applied_guard = applier.apply(guard, &guard_map);
  backend_->call("isGuardSatisfied", true, 1, "en", "b", &applied_guard, &satisfied);
  if((!entailed && satisfied) || (entailed && !satisfied) )
  {
    on_time = false;
    return true;
  }
  return false;
}
    

void PhaseSimulator::remove_redundant_parameters(phase_result_sptr_t phase)
{
  ConstraintStore par_cons = phase->get_parameter_constraint();
  Value end_time = phase->end_time.undefined()?Value("0"):phase->end_time;
  backend_->call("removeRedundantParameters", true, 4, "vltvltmvncsn", "cs", &phase->current_time, &end_time, &phase->variable_map, &par_cons, &par_cons);
  phase->set_parameter_constraint(par_cons);
}


void
PhaseSimulator::make_next_todo(phase_result_sptr_t& phase)
{
  timer::Timer next_todo_timer;

  HYDLA_LOGGER_DEBUG_VAR(*phase);

  apply_diff(*phase);

  phase_result_sptr_t next_todo(new PhaseResult());

  next_todo->step = phase->step + 1;

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
      approximate_phase(phase, phase->variable_map);
    }
    check_break_points(phase, phase->variable_map);
    if(!aborting)
    {
      remove_redundant_parameters(phase);
      next_todo->set_parameter_constraint(phase->get_parameter_constraint());
      next_todo->id = ++phase_sum_;
      next_todo->phase_type = INTERVAL_PHASE;
      next_todo->parent = phase.get();
      next_todo->diff_sum = phase->diff_sum;
      next_todo->current_time = phase->end_time = phase->current_time;
      next_todo->discrete_asks = phase->discrete_asks;
      next_todo->discrete_guards = phase->discrete_guards;
      phase->todo_list.push_back(next_todo);
    }
  }
  else
  {
    next_todo->phase_type = POINT_PHASE;
    replace_prev2parameter(*phase, phase->variable_map);

    reset_parameter_constraint(phase->get_parameter_constraint());

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
            backend_->call("productWithGlobalParameterConstraint", true, 1, "csn", "cs", &candidate.parameter_constraint, &candidate.parameter_constraint);
            if(!candidate.parameter_constraint.consistent())entry.second.erase(cand_it++);
            else
            {
              for(auto &entry : candidate.other_guards_to_time_condition)
              {
                entry.second = value_modifier->shift_time(-(phase->current_time - phase->parent->parent->current_time), value_t(entry.second) ).get_node();
              }
              cand_it++;

            }
          }
        }
      }

      next_pp_candidate_map_t &candidate_map = phase->next_pp_candidate_map;

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
      set<ask_t> asks;
      if(phase->parent->parent == result_root.get())
      {
        asks = relation_graph_->get_all_asks();
      }
      else
      {
        set<string> checked_variables;
        for(auto variable : diff_variables)
        {
          string var_name = variable.get_name();
          // 既にチェック済みの変数なら省略（x'とxはどちらもxとして扱うため，二回呼ばれないようにする）
          if(checked_variables.count(var_name))continue;
          checked_variables.insert(var_name);
          asks_t tmp_asks = relation_graph_->get_adjacent_asks(var_name);
          asks.insert(tmp_asks.begin(), tmp_asks.end());
        }
        for(auto entry : phase->discrete_asks)
        {
          asks.insert(entry.first);
        }
      }
      HYDLA_LOGGER_DEBUG("BREAK current phase:", phase_sum_);
      std::map<std::string, HistoryData> atomic_guard_min_time_interval_map;
      timer::Timer find_min_time_timer;
      if(opts_->interval && !max_time.infinite())upper_bound_of_itv_newton = evaluate_interval(phase, max_time - phase->current_time, false).upper();

      HYDLA_LOGGER_DEBUG("BREAK3 asks.size: ", asks.size());
      for(auto ask : asks)
      {
        HYDLA_LOGGER_DEBUG("BREAK3 call find_min_time for: ", get_infix_string(ask->get_guard()));
        candidate_map[ask] = find_min_time(ask->get_guard(), min_time_calculator, guard_time_map, original_vm, time_limit, relation_graph_->get_entailed(ask), phase, atomic_guard_min_time_interval_map);
      }

      HYDLA_LOGGER_DEBUG("BREAK3 break_point_list.size: ", break_point_list.size());
      for(auto &entry : break_point_list)
      {
        auto break_point = entry.first;
        HYDLA_LOGGER_DEBUG("BREAK3 call find_min_time for: ", get_infix_string(break_point.condition));
        entry.second = find_min_time(break_point.condition, min_time_calculator, guard_time_map, original_vm, time_limit, false, phase, atomic_guard_min_time_interval_map);
      }
      phase->profile["FindMinTime"] += find_min_time_timer.get_elapsed_us();
      pp_time_result_t time_result;
      // 各askに関する最小時刻を比較して最小のものを選ぶ．
      timer::Timer compare_min_time_timer;
      for(auto entry : candidate_map)
      {
        time_result = compare_min_time(time_result, entry.second, entry.first);
        for(auto candidate : entry.second)
        {
          HYDLA_LOGGER_DEBUG_VAR(candidate.time);
        }
      }
      for(auto entry : break_point_list)
      {
        ask_t null_ask;
        time_result = compare_min_time(time_result, entry.second, null_ask);
      }
      phase->profile["CompareMinTime"] += compare_min_time_timer.get_elapsed_us();
/*
      if(opts_->epsilon_mode >= 0){
        for(auto entry : time_result){
          HYDLA_LOGGER_DEBUG("#epsilon DC before : ", entry.parameter_constraint);
        }
        time_result = reduce_unsuitable_case(time_result, backend_.get(), phase);

        for(auto entry : time_result){
          HYDLA_LOGGER_DEBUG("#epsilon DC after : ", entry.parameter_constraint);
        }
      }
*/

      if(time_result.empty())
      {
        phase->simulation_state = simulator::TIME_LIMIT;
        phase->end_time = max_time;
      }
      else
      {
        auto time_it = time_result.begin();
        while(true)
        {
          DCCandidate &candidate = *time_it;
          phase->set_parameter_constraint(candidate.parameter_constraint);
          phase->end_time = phase->current_time + candidate.time;
          backend_->call("simplify", false, 1, "vln", "vl", &phase->end_time, &phase->end_time);
          if(candidate.time.undefined() || candidate.time.infinite() )
          {
            phase->simulation_state = TIME_LIMIT;
            phase->end_time = max_time;
          }
          else
          {
            next_todo->id = ++phase_sum_;
            next_todo->discrete_asks = candidate.discrete_asks;
            if(opts_->interval) 
            {
              // verify the time of the next discrete change
              if(evaluate_interval(phase, value_t(upper_bound_of_itv_newton) - candidate.time, false).lower() < 0)
              {
                string asks_str = "";
                for(auto discrete_ask : next_todo->discrete_asks)asks_str += get_infix_string(discrete_ask.first);
                throw HYDLA_ERROR("failed to calculate valid time of the discrete change for " + asks_str);
              }
            }
            if(opts_->step_by_step)
            {
              for(auto discrete_ask : next_todo->discrete_asks)
              {
                find_min_time_result_t &f_result = candidate_map[discrete_ask.first];
                if(opts_->epsilon_mode >= 0)f_result = reduce_unsuitable_case(f_result, backend_.get(), phase);
                HYDLA_ASSERT(f_result.size() == 1);
                HYDLA_LOGGER_DEBUG_VAR(f_result.front().discrete_guards.size());
                for(auto guard : f_result.front().discrete_guards)
                {
                  next_todo->discrete_guards.insert(guard); 
                }
              }
            }
            else if(opts_->interval || opts_->numerize_mode || opts_->epsilon_mode > 0)
            {
              // calculate discrete_guards
              for(auto discrete_ask : next_todo->discrete_asks)
              {
                find_min_time_result_t &f_result = candidate_map[discrete_ask.first];
                bool included_by_newton_guard = false;

                if(opts_->epsilon_mode >= 0){
                  for(auto entry : time_result){
                    HYDLA_LOGGER_DEBUG("#epsilon DC before : ", entry.parameter_constraint);
                  }
                  f_result = reduce_unsuitable_case(f_result, backend_.get(), phase);
                  for(auto entry : time_result){
                    HYDLA_LOGGER_DEBUG("#epsilon DC after : ", entry.parameter_constraint);
                  }
                }

                HYDLA_ASSERT(f_result.size() == 1);
                FindMinTimeCandidate candidate = f_result.front();
                if(candidate.guard_by_newton.get() != nullptr)
                {
                  backend_->call("borderIsIncluded", true, 3, "vlnvlnvln", "b", &candidate.time, &candidate.range_by_newton.get_lower_bound().value, &candidate.range_by_newton.get_upper_bound().value, &included_by_newton_guard);
                  next_todo->discrete_guards.insert(candidate.guard_by_newton);
                }
                for(auto guard : candidate.other_guards_to_time_condition)
                {
                  bool on_border;
                  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard.first));
                  backend_->call("onBorder", true, 2, "etvlt", "b", &guard.second, &candidate.time, &on_border);
                  if(on_border)
                  {
                    //if(included_by_newton_guard)throw HYDLA_ERROR("Both of 2 guards are on the border: " + get_infix_string(guard.first) +", " + get_infix_string(candidate.guard_by_newton));
                    next_todo->discrete_guards.insert(guard.first);
                  }
                }
              }
            }
            next_todo->next_pp_candidate_map = phase->next_pp_candidate_map;
            for(auto ask : next_todo->discrete_asks)
            {
              next_todo->next_pp_candidate_map.erase(ask.first);
            }
            next_todo->set_parameter_constraint(phase->get_parameter_constraint());
            next_todo->parent = phase.get();
            timer::Timer apply_time_timer;
            next_todo->prev_map = value_modifier->substitute_time(candidate.time, original_vm);
            phase->profile["ApplyTime2Expr"] += apply_time_timer.get_elapsed_us();
            next_todo->current_time = phase->end_time;
            if(opts_->eager_approximation) approximate_phase(next_todo, phase->prev_map);
            phase->simulation_state = SIMULATED;
            phase->todo_list.push_back(next_todo);
          }

          if(++time_it == time_result.end())break;
          //prepare new PhaseResult
          phase.reset(new PhaseResult(*phase));
          phase->id = ++phase_sum_;
          HYDLA_LOGGER_DEBUG_VAR(phase->id);
          HYDLA_LOGGER_DEBUG_VAR(phase->get_parameter_constraint());
          phase->parent->children.push_back(phase);
          phase->parent->todo_list.push_back(phase);
          phase->todo_list.clear();
          if(!(candidate.time.undefined() || candidate.time.infinite()) )
          {
            // prepare new todo
            next_todo.reset(new PhaseResult(*next_todo));
            next_todo->id = ++phase_sum_;
          }
        }
      }
    }
  }
  revert_diff(*phase);

  phase->profile["MakeNextTodo"] = next_todo_timer.get_elapsed_us();
}

void PhaseSimulator::approximate_phase(phase_result_sptr_t& phase, variable_map_t &vm_to_approximate)
{
  for(auto entry: vm_to_approximate)
  {
    if(!entry.second.unique())return;
  }
  // approximation
  if(opts_->numerize_mode)
  {
    phase->current_time = calculate_middle_value(phase, ValueRange(phase->current_time));
    for(auto &entry: vm_to_approximate)
    {
      entry.second.set_unique_value(calculate_middle_value(phase, entry.second));
    }
  }
  else if(opts_->interval && opts_->approximation_step > 0 && (phase->step/2) % opts_->approximation_step == 0)
  {
    if(opts_->affine)
    {
      interval::AffineApproximator* approximator = interval::AffineApproximator::get_instance();
      vector<parameter_map_t> parameter_maps = phase->get_parameter_maps();
      assert(parameter_maps.size() == 1);
      approximator->approximate(vars_to_approximate, vm_to_approximate, parameter_maps[0], phase->current_time);
      backend_->call("resetConstraintForParameter", false, 1, "mp", "", &parameter_maps[0]);
      phase->set_parameter_constraint(get_current_parameter_constraint());
    }
    else
    {
      for(auto &entry: vm_to_approximate)
      {
        if(vars_to_approximate.count(entry.first) == 0 )
          continue;
        assert(entry.second.unique());
        itvd interval = evaluate_interval(phase, entry.second, false);
        if(width(interval) > 0)
        {
          ValueRange range = create_range_from_interval(interval);
          Parameter param(entry.first, *phase);
          add_parameter_constraint(phase, param, range);
          entry.second = param.as_value();
        }
      }
      itvd interval = evaluate_interval(phase, phase->current_time, false);
      if(width(interval) > 0)
      {
        ValueRange range = create_range_from_interval(interval);
        Parameter param("t", -1, ++time_id);
        add_parameter_constraint(phase, param, range);
        phase->current_time = param.as_value();
      }
    }
  }
  if(opts_->interval)
  {
    double sum_width = 0;
    for(auto &entry: vm_to_approximate)
    {
      if(vars_to_approximate.count(entry.first) == 0 )
        continue;
      auto var = entry.first;
      itvd interval = evaluate_interval(phase, entry.second, false);
      phase->profile["width(" + var.get_string() + ")"] = width(interval);
      sum_width += width(interval);
    }
    phase->profile["Sum of width"] = sum_width;
  }
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
      DCCandidate candidate(newcomer.time, discrete_asks,  newcomer.parameter_constraint);
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
        backend_->call("compareMinTime", true, 4, "vltvltcsncsn", "cp", &existing.time, &newcomer.time, &existing.parameter_constraint, &newcomer.parameter_constraint, &compare_result);

        if(compare_result.less_cons.consistent())
        {
          DCCandidate candidate(existing.time, existing.discrete_asks, compare_result.less_cons);
          result.push_back(candidate);
        }
        if(compare_result.equal_cons.consistent())
        {
          map<ask_t, bool> discrete_asks = existing.discrete_asks;
          if(ask.get())discrete_asks[ask] = newcomer.on_time;
          DCCandidate candidate(existing.time, discrete_asks, compare_result.equal_cons);
          result.push_back(candidate);
        }
        if(compare_result.greater_cons.consistent())
        {
          map<ask_t, bool> discrete_asks;
          if(ask.get())discrete_asks[ask] = newcomer.on_time;
          DCCandidate candidate(newcomer.time, discrete_asks, compare_result.greater_cons);
          result.push_back(candidate);
        }
      }
    }
  }
  return result;
}


void PhaseSimulator::reset_parameter_constraint(ConstraintStore par_cons)
{
  backend_->call("resetConstraintForParameter", false, 1, "csn", "", &par_cons);
}


void PhaseSimulator::apply_diff(const PhaseResult &phase)
{
  for(auto diff : phase.module_diff)
  {
    relation_graph_->set_adopted(diff.first, diff.second);
  }
  for(auto positive : phase.get_diff_positive_asks())
  {
    relation_graph_->set_entailed(positive, true);
  }
  for(auto negative : phase.get_diff_negative_asks())
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
  revert_diff(phase.get_diff_positive_asks(), phase.get_diff_negative_asks(), phase.always_list, phase.module_diff);
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

list<itvd> PhaseSimulator::calculate_interval_newton_nd(const constraint_t& time_guard, parameter_map_t &pm)
{
  node_sptr exp;
  node_sptr dexp;
  backend_->call("relationToFunction", true, 1, "et", "e", &time_guard, &exp);
  backend_->call("differentiateWithTime", true, 1, "et", "e", &exp, &dexp);
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(exp));
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(dexp));


  itvd init = itvd(0,upper_bound_of_itv_newton);
  HYDLA_LOGGER_DEBUG_VAR(init);
  for(auto entry : pm)
  {
    HYDLA_LOGGER_DEBUG_VAR(entry.first);
    HYDLA_LOGGER_DEBUG_VAR(entry.second);
  }
  list<itvd> result_intervals =
    interval::calculate_interval_newton_nd(init, exp, dexp, pm);
  return result_intervals;
}

itvd PhaseSimulator::evaluate_interval(const phase_result_sptr_t phase, ValueRange range, bool use_affine)
{
  VariableReplacer v_replacer(phase->variable_map);
  v_replacer.replace_range(range);
  range = value_modifier->apply_function("simplify", range);
  vector<parameter_map_t> parameter_map_vector = phase->get_parameter_maps();
  assert(parameter_map_vector.size() <= 1);
  parameter_map_t pm = parameter_map_vector.size()==1?parameter_map_vector.front():parameter_map_t();
  HYDLA_ASSERT(range.unique());
  if(!use_affine)
  {
    interval::IntervalTreeVisitor interval_visitor;
    return interval_visitor.get_interval_value(range.get_unique_value().get_node(), nullptr, &pm);
  }else
  {
    interval::AffineTreeVisitor affine_visitor(phase->variable_map);
    return affine_visitor.approximate(range.get_unique_value().get_node()).to_interval();
  }
}


void PhaseSimulator::add_parameter_constraint(const phase_result_sptr_t phase, const Parameter &parameter, ValueRange range)
{
  simulator_->introduce_parameter(parameter, range);
  ConstraintStore new_store = phase->get_parameter_constraint();
  new_store.add_constraint_store(range.create_range_constraint(node_sptr(new se::Parameter(parameter))));
  phase->set_parameter_constraint(new_store);


  backend_->call("resetConstraintForParameter", false, 1, "csn", "", &new_store);
}

}
}

