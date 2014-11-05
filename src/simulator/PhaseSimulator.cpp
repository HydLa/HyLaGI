#include <iostream>

#include "PhaseSimulator.h"

#include "Logger.h"
#include "Timer.h"

#include "PrevReplacer.h"
#include "VariableFinder.h"
#include "PrevSearcher.h"
#include "Exceptions.h"
#include "AlwaysFinder.h"
#include "EpsilonMode.h"
#include "ValueModifier.h"

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
    todo->children.push_back(todo);
  }
  else
  {
    for(auto phase : phase_list)
    {
      make_next_todo(phase);
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
  backend_->call("addParameterConstraint", 1, "csn", "", &todo->initial_constraint_store);
  consistency_checker->set_prev_map(&todo->prev_map);
  relation_graph_->set_ignore_prev(todo->phase_type == POINT_PHASE);

  todo->profile["Preprocess"] += preprocess_timer.get_elapsed_us();

  ask_set_t prev_positives, prev_negatives, nonprev_trigger_asks;
  if(!relation_graph_is_taken_over)
  {
    // TODO: relation_graph_の状態が親フェーズから直接引き継いだものでない場合は，差分を用いることができないので全制約に関して設定する必要がある．
    // TODO: 並列実行とかしなければ現状でも問題はない
    assert(0);
  }
  else if(todo->phase_type == POINT_PHASE)
  {
    for(auto trigger : todo->discrete_asks)
    {
      if(trigger.second)
      {
        ConstraintStore always_list;
        bool entailed = relation_graph_->get_entailed(trigger.first);
        if(relation_graph_->entail_if_prev(trigger.first,
                                           !entailed,
                                           always_list))
        {
          todo->always_list.add_constraint_store(always_list);
          todo->diff_sum.add_constraint(trigger.first);
          if(entailed)
          {
            todo->diff_negative_asks.insert(trigger.first);
          }else
          {
            todo->diff_positive_asks.insert(trigger.first);
          }
        }
        
        else
        {
          nonprev_trigger_asks.insert(trigger.first);
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

    result_list.merge(simulate_ms(unadopted_ms, todo, nonprev_trigger_asks, prev_positives, prev_negatives));

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


list<phase_result_sptr_t> PhaseSimulator::simulate_ms(const module_set_t& unadopted_ms, phase_result_sptr_t& phase, ask_set_t trigger_asks, ask_set_t positive_asks, ask_set_t negative_asks)
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

  ask_set_t cc_local_positives, cc_local_negatives;
  ConstraintStore cc_local_always;

  consistency_checker->clear_inconsistent_module_sets();
  bool consistent = calculate_closure(phase, trigger_asks,
                                      local_diff_sum, cc_local_positives, cc_local_negatives, cc_local_always);
  phase->profile["CalculateClosure"] += cc_timer.get_elapsed_us();
  phase->profile["# of CalculateClosure"]++;

  if(!consistent)
  {
    HYDLA_LOGGER_DEBUG("INCONSISTENT: ", unadopted_ms);
    for(auto module_set : consistency_checker->get_inconsistent_module_sets())
    {
      module_set_container->generate_new_ms(phase->unadopted_mss, module_set);
    }
  }
  else
  {
    HYDLA_LOGGER_DEBUG("CONSISTENT: ", unadopted_ms);
    timer::Timer postprocess_timer;
    module_set_container->remove_included_ms_by_current_ms();
    phase->unadopted_mss.insert(unadopted_ms);

    phase->parent->children.push_back(phase);

    vector<variable_map_t> create_result = consistency_checker->get_result_maps();
    if(create_result.size() != 1)
    {
      throw SimulateError("result variable map is not single.");
    }
    phase->variable_map = create_result[0];


    phase->profile["# of CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_count();
    phase->profile["CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_time();
    consistency_checker->reset_count();

    backend_->call("createParameterMap", 0, "", "mp", &phase->parameter_map);
  
/*  TODO: implement
    if(opts_->assertion || break_condition_.get() != NULL){
    timer::Timer entailment_timer;

    backend_->call("resetConstraintForVariable", 0, "","");
    string fmt = "mv0";
    fmt += (phase->phase_type==POINT_PHASE)?"n":"t";
    backend_->call("addConstraint", 1, fmt.c_str(), "", &phase->variable_map);
    backend_->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);

    if(opts_->assertion)
    {
    HYDLA_LOGGER_DEBUG("%% check_assertion");
    CheckConsistencyResult cc_result;
    switch(consistency_checker->check_entailment(*relation_graph_, cc_result, symbolic_expression::node_sptr(new symbolic_expression::Not(opts_->assertion)), phase->phase_type)){
    case CONFLICTING:
    case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
    cout << "Assertion Failed!" << endl;
    HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
    phase->simulation_state = ASSERTION;
    break;
    case ENTAILED:
    break;
    case BRANCH_PAR:
    HYDLA_LOGGER_DEBUG("%% failure of assertion depends on conditions of parameters");
    push_branch_states(phase, cc_result);
    cout << "Assertion Failed!" << endl;
    phase->parameter_map = phase->parameter_map;
    HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
    phase->simulation_state = ASSERTION;
    break;
    }
    phase->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    }
    if(break_condition_.get() != NULL)
    {
    HYDLA_LOGGER_DEBUG("%% check_break_condition");
    CheckConsistencyResult cc_result;
    switch(consistency_checker->check_entailment(*relation_graph_, cc_result, break_condition_, phase->phase_type)){
    case ENTAILED:
    case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
    case BRANCH_PAR: //TODO: 分岐すべき？要検討
    breaking = true;
    break;
    case CONFLICTING:
    break;
    }
    }
    }
*/

    if(opts_->epsilon_mode){
      phase->variable_map = cut_high_order_epsilon(backend_.get(),phase);
    }
    
    phase->diff_positive_asks.insert(positive_asks.begin(), positive_asks.end());
    phase->diff_negative_asks.insert(negative_asks.begin(), negative_asks.end());
    phase->diff_positive_asks.insert(cc_local_positives.begin(), cc_local_positives.end());
    phase->diff_negative_asks.insert(cc_local_negatives.begin(), cc_local_negatives.end());
    phase->always_list.add_constraint_store(cc_local_always);
    phase->simulation_state = SIMULATED;
    phase->diff_sum = local_diff_sum;
    phase->unadopted_ms = unadopted_ms;
    phase->module_diff = module_diff;
    result_list.push_back(phase);
    phase->profile["Postprocess"] += postprocess_timer.get_elapsed_us();
  }
  revert_diff(cc_local_positives, cc_local_negatives, cc_local_always, module_diff);

  return result_list;
}

void PhaseSimulator::push_branch_states(phase_result_sptr_t &original, CheckConsistencyResult &result){
  phase_result_sptr_t branch_state_false(new PhaseResult(*original));
  branch_state_false->id = ++todo_id;
  branch_state_false->initial_constraint_store.add_constraint_store(result.inconsistent_store);
  original->parent->todo_list.push_back(branch_state_false);
  original->initial_constraint_store.add_constraint_store(result.consistent_store);
  backend_->call("resetConstraintForParameter", 1, "csn", "", &original->initial_constraint_store);
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


  // TODO: manage relation_graph_is_taken_over appropriately for parallel processing and resuming simulation
  relation_graph_is_taken_over = true;

  if(opts_->max_time != ""){
    max_time = symbolic_expression::node_sptr(new symbolic_expression::Number(opts_->max_time));
  }else{
    max_time = symbolic_expression::node_sptr(new symbolic_expression::Infinity());
  }

  backend_->set_variable_set(*variable_set_);
  value_modifier.reset(new ValueModifier(*backend_));
}

void PhaseSimulator::replace_prev2parameter(PhaseResult &phase,
                                            variable_map_t &vm,
                                            parameter_map_t &parameter_map)
{
  assert(phase.parent != nullptr);
  PrevReplacer replacer(parameter_map, phase, *simulator_, opts_->approx);
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

bool PhaseSimulator::calculate_closure(phase_result_sptr_t& state, ask_set_t &trigger_asks, ConstraintStore &diff_sum, ask_set_t &positive_asks, ask_set_t &negative_asks, ConstraintStore expanded_always)
{
  ask_set_t unknown_asks = trigger_asks;
  bool expanded, first_loop = true;
  PhaseType phase_type = state->phase_type;
  ConstraintStore diff = diff_sum;
  ConstraintStore next_diff;
  do{
    HYDLA_LOGGER_DEBUG_VAR(diff);
    expanded = false;
    timer::Timer entailment_timer;
    
    variable_set_t discrete_variables;
    for(auto constraint: diff)
    {
      VariableFinder finder;
      finder.visit_node(constraint);
      variable_set_t variables = phase_type==POINT_PHASE?finder.get_variable_set()
        :finder.get_all_variable_set();
      discrete_variables.insert(variables.begin(), variables.end());
    }
    for(auto variable : discrete_variables)
    {
      set<ask_t> adjacents = relation_graph_->get_adjacent_asks(variable.get_name(), phase_type == POINT_PHASE);
      unknown_asks.insert(adjacents.begin(), adjacents.end());
    }
    for(auto ask_it = unknown_asks.begin(); ask_it != unknown_asks.end();)
    {
      auto& ask = *ask_it;

      if(
        (phase_type == POINT_PHASE
         // in initial state, conditions about left-hand limits are considered to be invalid
         && state->parent == result_root.get()
         && PrevSearcher().search_prev(ask->get_guard()))
        ||
        // omit judgments of continous guards
        (state->in_following_step() && judge_continuity(state, ask, discrete_variables))){
        ask_it = unknown_asks.erase(ask_it);
        continue;
      }

      CheckConsistencyResult check_consistency_result;
      switch(consistency_checker->check_entailment(*relation_graph_, check_consistency_result, ask, phase_type, state->profile)){

      case BRANCH_PAR:
        HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
        push_branch_states(state, check_consistency_result);
        // Since we choose entailed case in push_branch_states, we go down without break.
      case ENTAILED:
        HYDLA_LOGGER_DEBUG("\n--- entailed ask ---\n", get_infix_string(ask->get_guard()));
        if(!relation_graph_->get_entailed(ask))
        {
          ConstraintStore always_list;
          always_list = relation_graph_->set_entailed(ask, true);
          expanded_always.add_constraint_store(always_list);
          positive_asks.insert(ask);
          diff.add_constraint(ask->get_child());
          next_diff.add_constraint(ask->get_child());
          diff_sum.add_constraint(ask->get_child());
          expanded = true;
        }
        ask_it = unknown_asks.erase(ask_it);
        break;
      case CONFLICTING:
        HYDLA_LOGGER_DEBUG("\n--- conflicted ask ---\n", *(ask->get_guard()));
        if(relation_graph_->get_entailed(ask))
        {
          next_diff.add_constraint(ask->get_child());
          diff_sum.add_constraint(ask->get_child());
          diff.add_constraint(ask->get_child());
          relation_graph_->set_entailed(ask, false);
          negative_asks.insert(ask);
          expanded = true;
        }
        ask_it = unknown_asks.erase(ask_it);
        break;
      case BRANCH_VAR:
        HYDLA_LOGGER_DEBUG("\n--- branched ask ---\n", *(ask->get_guard()));
        ask_it++;
        break;
      }
      state->profile["# of CheckEntailment"]+= 1;
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    if(expanded || first_loop)
    {
      timer::Timer consistency_timer;
      CheckConsistencyResult cc_result;
      cc_result = consistency_checker->check_consistency(*relation_graph_, diff, phase_type, state->profile);
      state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
      state->profile["# of CheckConsistency"]++;
      if(!cc_result.consistent_store.consistent()){
        return false;
      }else if (cc_result.inconsistent_store.consistent()){
        push_branch_states(state, cc_result);
      }
      first_loop = false;
    }
    diff = next_diff;
    next_diff.clear();
  }while(expanded);

  // TODO: implement branching here
  if(!unknown_asks.empty()){
    throw SimulateError("unknown asks");
  }
  return true;
}

bool PhaseSimulator::judge_continuity(const phase_result_sptr_t &todo, const ask_t &ask, const variable_set_t &changing_variables)
{
  // TODO: 何かうまく行かないコーナーケースがある気がするので考える．
  for(auto discrete_ask : todo->discrete_asks)
  {
    if(discrete_ask.first == ask) return false;
  }
  
  VariableFinder finder;
  finder.visit_node(ask->get_guard());
  variable_set_t variables(finder.get_all_variable_set());

  for(auto variable : variables){
    auto differential_pair = todo->parent->variable_map.find(Variable(variable.get_name(), variable.get_differential_count() + 1));
    if(differential_pair == todo->parent->variable_map.end() || differential_pair->second.undefined()) return false;
    for(auto cv : changing_variables){
      if(variable.get_name() == cv.get_name()) return false;
    }
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

void
PhaseSimulator::make_next_todo(phase_result_sptr_t& phase)
{
  timer::Timer next_todo_timer;

  apply_diff(*phase);

  phase_result_sptr_t next_todo(new PhaseResult());
  next_todo->id = ++phase_sum_;
  next_todo->step = phase->step + 1;
  AlwaysFinder always_finder;
  ConstraintStore non_always;
  always_set_t always_set;
  // TOOD: move this block into upper level
  if(phase->parent == result_root.get())
  {
    for(auto module : module_set_container->get_max_module_set())
    {
      always_finder.find_always(module.second, &always_set, &non_always);
    }
    for(auto constraint : non_always)
    {
      relation_graph_->set_expanded_atomic(constraint, false);
    }
  }
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
    next_todo->phase_type = INTERVAL_PHASE;
    next_todo->parent = phase.get();
    next_todo->diff_sum = phase->diff_sum;
    next_todo->current_time = phase->end_time = phase->current_time;
    next_todo->discrete_asks = phase->discrete_asks;
    next_todo->prev_map = phase->variable_map;
    phase->todo_list.push_back(next_todo);
  }
  else
  {
    PhaseSimulator::replace_prev2parameter(*phase->parent, phase->variable_map, phase->parameter_map);
    next_todo->phase_type = POINT_PHASE;

    backend_->call("resetConstraint", 0, "", "");
    backend_->call("addParameterConstraint", 1, "mp", "", &phase->parameter_map);

    value_t time_limit(max_time);
    time_limit -= phase->current_time;

/* TODO: implement
//assertionの否定を追加
if(opts_->assertion){
symbolic_expression::node_sptr assert_node(new Not(opts_->assertion));
dc_causes.push_back(dc_cause_t(assert_node, -2));
}
if(break_condition_.get() != NULL)
{
dc_causes.push_back(dc_cause_t(break_condition_, -3));
}
*/

/* TODO:implement
   if(opts_->epsilon_mode){
   time_result = reduce_unsuitable_case(time_result, backend_.get(), phase);
   }
*/

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

    pp_time_result_t time_result;
    VariableFinder v_finder;
    for(auto diff : phase->diff_sum)v_finder.visit_node(diff);
    for(auto trigger : phase->discrete_asks)v_finder.visit_node(trigger.first->get_guard());

    variable_set_t diff_variables = v_finder.get_all_variable_set();
    if(phase->in_following_step())
    {
      phase->next_pp_candidate_map = phase->parent->parent->next_pp_candidate_map;
      for(auto &entry : phase->next_pp_candidate_map)
      {
        for(auto &candidate : entry.second)
        {
          candidate.time -= phase->current_time;
        }
      }
    }
    next_pp_candidate_map_t &candidate_map = phase->next_pp_candidate_map;
    HYDLA_LOGGER_DEBUG_VAR(candidate_map);

    map<ask_t, find_min_time_result_t> calculated_pp_time_map;
    set<string> checked_variables;


    // 各変数に関する最小時刻を更新する．
    for(auto variable : diff_variables)
    {
      string var_name = variable.get_name();
      // 既にチェック済みの変数なら省略（x'とxはどちらもxとして扱うため，二回呼ばれないようにする）
      if(checked_variables.count(var_name))continue;
      checked_variables.insert(var_name);
      pp_time_result_t result_for_this_var;

      ask_set_t asks = relation_graph_->get_adjacent_asks(var_name);
      for(auto ask : asks)
      {
        HYDLA_LOGGER_DEBUG_VAR(get_infix_string(ask));
        find_min_time_result_t min_time_for_this_ask;
        if(calculated_pp_time_map.count(ask))
        {
          min_time_for_this_ask = calculated_pp_time_map[ask];
        }
        else
        {
          variable_map_t related_vm = get_related_vm(ask, original_vm);

          symbolic_expression::node_sptr trigger(ask->get_guard());
          if(relation_graph_->get_entailed(ask))trigger.reset(new Not(trigger)); // negate the condition
          backend_->call("findMinTime", 3, "etmvtvlt", "f", &trigger, &related_vm, &time_limit, &min_time_for_this_ask);
          calculated_pp_time_map[ask] = min_time_for_this_ask;
          set<string> adjacent_var_names = relation_graph_->get_adjacent_variables(ask);
          for(auto adjacent_var_name : adjacent_var_names)
          {
            if(adjacent_var_name == var_name)continue;
            HYDLA_LOGGER_DEBUG_VAR(adjacent_var_name);
            candidate_map[adjacent_var_name] = compare_min_time(candidate_map[adjacent_var_name], min_time_for_this_ask, ask);
          }
        }
        result_for_this_var = compare_min_time(result_for_this_var, min_time_for_this_ask, ask);
      }
      HYDLA_LOGGER_DEBUG_VAR(result_for_this_var);
      candidate_map[var_name] = result_for_this_var;
    }
    set<ask_t> checked_asks;
    set<string> min_time_variables;
    // 各変数に関する最小時刻を比較して最小のものを選ぶ．
    for(auto entry : candidate_map)
    {
      time_result = compare_min_time(time_result, entry.second);
    }
    
    if(time_result.empty())
    {
      DCCandidate candidate;
      candidate.time = max_time;
      time_result.push_back(candidate);
    }
    
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
      }
      else
      {
        next_todo->discrete_asks = candidate.discrete_asks;
        next_todo->parameter_map = pr->parameter_map;
        next_todo->parent = pr.get();
        next_todo->prev_map = value_modifier->substitute_time(candidate.time, original_vm);
        next_todo->current_time = pr->end_time;
        pr->todo_list.push_back(next_todo);
      }
      // HAConverter, HASimulator用にTIME_LIMITのtodoも返す
/*
  TODO: implement
  if((opts_->ha_convert_mode || opts_->ha_simulator_mode) && pr->simulation_state == TIME_LIMIT)
  {
  next_todo->current_time = pr->end_time;
  next_todo->parameter_map = pr->parameter_map;
  next_todo->parent = pr;
  ret.push_back(next_todo);
  }
*/
      if(++time_it == time_result.end())break;
      
      // TODO: 全部コピーしなくていい気がするので何をコピーすべきか考える
      pr.reset(new PhaseResult(*pr));
      pr->id = ++phase_sum_;
      pr->parent->children.push_back(pr);
      pr->parent->todo_list.push_back(pr);
      pr->todo_list.clear();
      next_todo.reset(new PhaseResult(*next_todo));
      next_todo->id = ++phase_sum_;
    }
  }
  revert_diff(*phase);
}

pp_time_result_t PhaseSimulator::compare_min_time(const pp_time_result_t &existing, const find_min_time_result_t &newcomers, const ask_t &ask)
{
  pp_time_result_t result;
  if(existing.empty())
  {
    for(auto newcomer :newcomers)
    {
      map<ask_t, bool> discrete_asks;
      discrete_asks[ask] = newcomer.on_time;
      DCCandidate candidate(newcomer.time, discrete_asks, newcomer.parameter_map);
      result.push_back(candidate);      
    }
  }
  else if(newcomers.empty())
  {
    result = existing;
    return result;
  }
  else
  {
    for(auto newcomer : newcomers)
    {
      for(auto existing_it = existing.begin(); existing_it != existing.end(); existing_it++)
      {
        compare_min_time_result_t compare_result;
        backend_->call("compareMinTime", 4, "vltvltmpmp", "cp", &existing_it->time, &newcomer.time, &existing_it->parameter_map, &newcomer.parameter_map, &compare_result);
        for(auto less_map : compare_result.less_maps)
        {
          DCCandidate candidate(existing_it->time, existing_it->discrete_asks, less_map);
          result.push_back(candidate);
        }
        for(auto equal_map : compare_result.equal_maps)
        {
          map<ask_t, bool> discrete_asks = existing_it->discrete_asks;
          discrete_asks[ask] = newcomer.on_time;
          DCCandidate candidate(existing_it->time, discrete_asks, equal_map);
          result.push_back(candidate);
        }
        for(auto greater_map : compare_result.greater_maps)
        {
          map<ask_t, bool> discrete_asks;
          discrete_asks[ask] = newcomer.on_time;
          DCCandidate candidate(newcomer.time, discrete_asks, greater_map);
          result.push_back(candidate);
        }
      }
    }
  }
  return result;
}


pp_time_result_t PhaseSimulator::compare_min_time(const pp_time_result_t &existings, const pp_time_result_t &newcomers)
{

  if(existings.empty())
  {
    return newcomers;
  }
  else if(newcomers.empty())
  {
    return existings;
  }
  pp_time_result_t result;
  HYDLA_LOGGER_DEBUG_VAR(existings);
  HYDLA_LOGGER_DEBUG_VAR(newcomers);
  
  for(auto newcomer : newcomers)
  {
    for(auto existing : existings)
    {
      if(newcomer.discrete_asks == existing.discrete_asks)
      {
        result.push_back(existing);
        continue;
      }
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
        discrete_asks.insert(newcomer.discrete_asks.begin(), newcomer.discrete_asks.end());
        for(auto ask : discrete_asks)
        {
          HYDLA_LOGGER_DEBUG_VAR(get_infix_string(ask.first));
        }
        DCCandidate candidate(existing.time, discrete_asks, equal_map);
        result.push_back(candidate);
      }
      for(auto greater_map : compare_result.greater_maps)
      {
        DCCandidate candidate(newcomer.time, newcomer.discrete_asks, greater_map);
        result.push_back(candidate);
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


void PhaseSimulator::revert_diff(const PhaseResult &phase)
{
  revert_diff(phase.diff_positive_asks, phase.diff_negative_asks, phase.always_list, phase.module_diff);
}

void PhaseSimulator::revert_diff(const ask_set_t &positive_asks, const ask_set_t &negative_asks, const ConstraintStore &always_list, const module_diff_t &module_diff)
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
    relation_graph_->set_expanded_atomic(always, true);
  }
}



}
}
