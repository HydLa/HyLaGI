#include <iostream>

#include "PhaseSimulator.h"

#include "Logger.h"
#include "Timer.h"

#include "PrevReplacer.h"
#include "AskCollector.h"
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

void PhaseSimulator::process_todo(simulation_job_sptr_t &todo)
{
  timer::Timer phase_timer;

  module_set_container->reset();
  // apply diff
  for(auto diff : todo->expanded_diff)
  {
    HYDLA_LOGGER_DEBUG_VAR(get_infix_string(diff.first));
    HYDLA_LOGGER_DEBUG_VAR(diff.second);
    relation_graph_->set_expanded(diff.first, diff.second);
  }
  
  list<phase_result_sptr_t> phase_list = make_results_from_todo(todo);

  if(phase_list.empty())
  {
    phase_result_sptr_t phase(new PhaseResult(*todo, INCONSISTENCY));
    todo->owner->children.push_back(phase);
  }
  else
  {
    for(auto phase : phase_list)
    {
      make_next_todo(phase, todo);
    }
  }
  // revert diff
  // caution: If there are duplications here (for example, positive asks in the previous phase are again included in positive diff in this phase),
  // the state of the relation_graph_ won't be reverted!
  for(auto diff : todo->expanded_diff)
  {
    relation_graph_->set_expanded(diff.first, !diff.second);
  }
  todo->profile["PhaseResult"] += phase_timer.get_elapsed_us();  
}


std::list<phase_result_sptr_t> PhaseSimulator::make_results_from_todo(simulation_job_sptr_t& todo)
{
  std::list<phase_result_sptr_t> result_list;  
  timer::Timer preprocess_timer;

  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &todo->parameter_map);
  backend_->call("addParameterConstraint", 1, "csn", "", &todo->initial_constraint_store);
  consistency_checker->set_prev_map(&todo->prev_map);
  relation_graph_->set_ignore_prev(todo->phase_type == POINT_PHASE);

  if(todo->owner == result_root)
  {
    // in the initial state, set all modules expanded
    for(auto module : module_set_container->get_max_module_set())
    {
      relation_graph_->set_expanded(module.second, true);
      todo->expanded_constraints.add_constraint(module.second);
      todo->expanded_diff[module.second] = true;
    }
  }
  todo->profile["Preprocess"] += preprocess_timer.get_elapsed_us();

  map<ask_t, bool> discrete_nonprev_positives, discrete_nonprev_negatives;
  ask_set_t prev_positives, prev_negatives;
  if(!relation_graph_is_taken_over)
  {
    // TODO: relation_graph_の状態が親フェーズから直接引き継いだものでない場合は，差分を用いることができないので全制約に関して設定する必要がある．
    // TODO: 並列実行とかしなければ現状でも問題はない
    assert(0);
  }
  else if(todo->phase_type == POINT_PHASE)
  {
    // set entailedness for prev_asks
    for(auto positive : todo->discrete_positive_asks)
    {
      if(positive.second)
      {
        if(guard_relation_graph_->set_entailed_if_prev(positive.first, true))
        {
          relation_graph_->set_expanded(positive.first->get_child(), true);
          todo->expanded_diff[positive.first->get_child()] = true;
          prev_positives.insert(positive.first);
        }
        else
        {
          discrete_nonprev_positives.insert(positive);
        }
      }
    }
    for(auto negative : todo->discrete_negative_asks)
    {
      if(negative.second)
      {
        if(guard_relation_graph_->set_entailed_if_prev(negative.first, false))
        {
          relation_graph_->set_expanded(negative.first->get_child(), false);
          todo->expanded_diff[negative.first->get_child()] = false;
          prev_negatives.insert(negative.first);
        }
        else
        {
          // set ask "not entailed" to preserve monotonicity in calculate closure
          discrete_nonprev_negatives.insert(negative);
          relation_graph_->set_expanded(negative.first->get_child(), false);
        }
      }
    }
  }
  
  while(module_set_container->has_next())
  {
    // TODO: unadopted_ms も差分をとるようにして使いたい
    // 大体の例題ではトップレベルからでも効率が悪化しないので，expanded_diffほど重要ではなさそう
    module_set_t unadopted_ms = module_set_container->unadopted_module_set();
    string module_sim_string = "\"ModuleSet" + unadopted_ms.get_name() + "\"";
    timer::Timer ms_timer;

    result_list.merge(simulate_ms(unadopted_ms, todo, discrete_nonprev_positives, discrete_nonprev_negatives, prev_positives, prev_negatives));

    todo->profile[module_sim_string] += ms_timer.get_elapsed_us();

  }

  if(todo->profile["# of CheckConsistency"]) todo->profile["Average of CheckConsistency"] = todo->profile["CheckConsistency"] / todo->profile["# of CheckConsistency"];
  if(todo->profile["# of CheckEntailment"]) todo->profile["Average of CheckEntailment"] =  todo->profile["CheckEntailment"] / todo->profile["# of CheckEntailment"];

  return result_list;
}


list<phase_result_sptr_t> PhaseSimulator::simulate_ms(const module_set_t& unadopted_ms, simulation_job_sptr_t& todo,  map<ask_t, bool> &discrete_nonprev_positives, map<ask_t, bool> &discrete_nonprev_negatives, ask_set_t positive_asks, ask_set_t negative_asks)
{
  HYDLA_LOGGER_DEBUG("\n--- next unadopted module set ---\n", unadopted_ms.get_infix_string());

  constraint_diff_t ms_local_diff;

  relation_graph_->set_adopted(unadopted_ms, false);
  guard_relation_graph_->set_adopted(unadopted_ms, false);


  list<phase_result_sptr_t> result_list;  
  timer::Timer cc_timer;

  bool consistent = calculate_closure(todo, ms_local_diff, discrete_nonprev_positives, discrete_nonprev_negatives, positive_asks, negative_asks);
  todo->profile["CalculateClosure"] += cc_timer.get_elapsed_us();
  todo->profile["# of CalculateClosure"]++;

  if(!consistent)
  {
    HYDLA_LOGGER_DEBUG("INCONSISTENT: ", unadopted_ms);
    for(auto module_set : consistency_checker->get_inconsistent_module_sets())
    {
      module_set_container->generate_new_ms(todo->unadopted_mss, module_set);
    }
  }
  else
  {
    HYDLA_LOGGER_DEBUG("CONSISTENT: ", unadopted_ms);

    timer::Timer postprocess_timer;

    module_set_container->remove_included_ms_by_current_ms();
    todo->unadopted_mss.insert(unadopted_ms);

    phase_result_sptr_t phase = make_new_phase(todo);

    vector<variable_map_t> create_result = consistency_checker->get_result_maps();
    if(create_result.size() != 1)
    {
      throw SimulateError("result variable map is not single.");
    }
    phase->variable_map = create_result[0];


    if(todo->in_following_step()){
      timer::Timer timer;
      if(phase->phase_type == INTERVAL_PHASE && phase->parent && phase->parent->parent){
        variable_map_t &vm_to_take_over = phase->parent->parent->variable_map;
        for(auto var_entry : vm_to_take_over)
        {
          auto var = var_entry.first;
          if(!phase->variable_map.count(var) && relation_graph_->referring(var) )
          {
            auto var = var_entry.first;
            if(!phase->variable_map.count(var) && relation_graph_->referring(var) )
            {
              // TODO : ここでずらした時刻をmake_next_todoの中で戻すことになっているので何とかする
              phase->variable_map[var] = value_modifier->shift_time(-phase->current_time, vm_to_take_over[var]);
            }
          }
        }
      }
      else if(phase->phase_type == POINT_PHASE && phase->parent){
        variable_map_t &vm_to_take_over = todo->prev_map;
        for(auto var_entry : vm_to_take_over)
        {
          auto var = var_entry.first;
          if(!phase->variable_map.count(var) && relation_graph_->referring(var) )
          {
            auto var = var_entry.first;
            if(!phase->variable_map.count(var) && relation_graph_->referring(var) )
            {
              phase->variable_map[var] = vm_to_take_over[var];
            }
          }
        }
        todo->profile["TakeOverVM"] += timer.get_elapsed_us();
      }
    }

    todo->profile["# of CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_count();
    todo->profile["CheckConsistency(Backend)"] += consistency_checker->get_backend_check_consistency_time();
    consistency_checker->reset_count();

    for(auto positive_ask : positive_asks)
    {
      todo->expanded_constraints.add_constraint(positive_ask->get_child());
    }


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
    switch(consistency_checker->check_entailment(*relation_graph_, cc_result, symbolic_expression::node_sptr(new symbolic_expression::Not(opts_->assertion)), todo->phase_type)){
    case CONFLICTING:
    case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
    cout << "Assertion Failed!" << endl;
    HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
    phase->cause_for_termination = ASSERTION;
    break;
    case ENTAILED:
    break;
    case BRANCH_PAR:
    HYDLA_LOGGER_DEBUG("%% failure of assertion depends on conditions of parameters");
    push_branch_states(todo, cc_result);
    cout << "Assertion Failed!" << endl;
    phase->parameter_map = todo->parameter_map;
    HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
    phase->cause_for_termination = ASSERTION;
    break;
    }
    todo->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    }
    if(break_condition_.get() != NULL)
    {
    HYDLA_LOGGER_DEBUG("%% check_break_condition");
    CheckConsistencyResult cc_result;
    switch(consistency_checker->check_entailment(*relation_graph_, cc_result, break_condition_, todo->phase_type)){
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
    
    phase->diff_positive_asks = positive_asks;
    phase->diff_negative_asks = negative_asks;
    phase->module_set = unadopted_ms;
    phase->expanded_diff = ms_local_diff;
    result_list.push_back(phase);
    todo->profile["Postprocess"] += postprocess_timer.get_elapsed_us();
  }

  
  // revert diff
  for(auto diff : ms_local_diff)
  {
    relation_graph_->set_expanded(diff.first, !diff.second);
  }
  guard_relation_graph_->set_adopted(unadopted_ms, true);
  relation_graph_->set_adopted(unadopted_ms, true);

  return result_list;
}

void PhaseSimulator::push_branch_states(simulation_job_sptr_t &original, CheckConsistencyResult &result){
  simulation_job_sptr_t branch_state_false(new SimulationJob(*original));
  branch_state_false->id = todo_id++;
  branch_state_false->initial_constraint_store.add_constraint_store(result.inconsistent_store);
  original->owner->todo_list.push_back(branch_state_false);
  original->initial_constraint_store.add_constraint_store(result.consistent_store);
  backend_->call("resetConstraintForParameter", 1, "csn", "", &original->initial_constraint_store);
}

phase_result_sptr_t PhaseSimulator::make_new_phase(simulation_job_sptr_t& todo)
{
  phase_result_sptr_t phase(new PhaseResult(*todo));
  phase->id = ++phase_sum_;
  todo->owner->children.push_back(phase);
  return phase;
}


phase_result_sptr_t PhaseSimulator::make_new_phase(const phase_result_sptr_t& original)
{
  phase_result_sptr_t phase(new PhaseResult(*original));
  // TODO: 全部コピーしなくていい気がするので何をコピーすべきか考える
  phase->id = ++phase_sum_;
  phase->parent->children.push_back(phase);
  phase->cause_for_termination = NOT_SIMULATED;
  return phase;
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
  phase_sum_ = 0;
  todo_id = 0;
  module_set_container = msc;
  result_root = root;

  simulator::module_set_t ms = module_set_container->get_max_module_set();

  relation_graph_.reset(new RelationGraph(ms)); 

  guard_relation_graph_.reset(new AskRelationGraph(ms));


  if(opts_->dump_relation){
    relation_graph_->dump_graph(cout);
    exit(EXIT_SUCCESS);
  }

  for(auto variable : *variable_set_)
  {
    variable_names.insert(variable.get_name());
  }

  AskCollector ac;

  ConstraintStore constraints;
  for(auto module : ms)
  {
    constraints.add_constraint(module.second);
  }
  ask_set_t pat;
  ask_set_t nat;

  ac.collect_ask(constraints, &pat, &nat, &all_asks);
  FullInformation root_information;
  root_information.negative_asks = all_asks;
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

bool PhaseSimulator::calculate_closure(simulation_job_sptr_t& state, constraint_diff_t &local_diff, map<ask_t, bool> &discrete_nonprev_positives, map<ask_t, bool> &discrete_nonprev_negatives, ask_set_t &positive_asks, ask_set_t &negative_asks)
{
  ask_set_t judged_asks;

  ask_set_t unknown_asks;

  variable_set_t discrete_variables;

  PhaseType &phase_type = state->phase_type;

  bool expanded;
  ConstraintStore all_diff;
  for(auto diff : state->expanded_diff)all_diff.add_constraint(diff.first);
  for(auto diff : state->adopted_module_diff)all_diff.add_constraint(diff.first.second);
  for(auto diff : local_diff)all_diff.add_constraint(diff.first);
  // IPでは親となるPPでの離散変化部分についても計算する必要がある
  if(phase_type == INTERVAL_PHASE)
  {
    for(auto diff : state->owner->expanded_diff)
    {
      all_diff.add_constraint(diff.first);
    }
    for(auto diff : state->owner->adopted_module_diff)
    {
      all_diff.add_constraint(diff.first.second);
    }
  }

  do{
    HYDLA_LOGGER_DEBUG_VAR(all_diff);
    // assumption: Constraints increase monotonically in this loop
    {
      VariableFinder finder;
      for(auto constraint : all_diff) finder.visit_node(constraint);
      if(phase_type==POINT_PHASE)
      {
        for(auto var : finder.get_variable_set())discrete_variables.insert(var);
      }
      else
      {
        for(auto var : finder.get_all_variable_set())discrete_variables.insert(var);
      }
    }
    
    {
      timer::Timer consistency_timer;
      CheckConsistencyResult cc_result;
      cc_result = consistency_checker->check_consistency(*relation_graph_, all_diff, discrete_variables, phase_type, state->profile);
      state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us(); 
      state->profile["# of CheckConsistency"]++;
      if(!cc_result.consistent_store.consistent()){
        return false;
      }else if (cc_result.inconsistent_store.consistent()){
        push_branch_states(state, cc_result);
      }
    }

    AskRelationGraph::asks_t asks;
    expanded = false;
    timer::Timer entailment_timer;
    if(phase_type == POINT_PHASE)
    {
      // guards not related to diff can be judged without check_entailment
      VariableFinder finder;
      for(auto diff : all_diff)finder.visit_node(diff);
      for(auto positive : discrete_nonprev_positives)
      {
        if(positive.second && !judged_asks.count(positive.first) && !finder.include_variables(positive.first->get_guard()))
        {
          guard_relation_graph_->set_entailed(positive.first, true);
          relation_graph_->set_expanded(positive.first->get_child(), true);
          local_diff[positive.first->get_child()] = true;
          positive_asks.insert(positive.first);
          all_diff.add_constraint(positive.first->get_child());
          judged_asks.insert(positive.first);
          expanded = true;
        }
      }
      for(auto negative : discrete_nonprev_negatives)
      {
        if(negative.second && !judged_asks.count(negative.first) && !finder.include_variables(negative.first->get_guard()))
        {
          negative_asks.insert(negative.first);
          state->expanded_diff[negative.first->get_child()] = false;
          all_diff.add_constraint(negative.first->get_child());
          judged_asks.insert(negative.first);
        }
      }
    }
    else
    {
      // All discrete causes for parent PP need to be check_entailment
      for(auto positive : state->discrete_positive_asks)asks.push_back(positive.first);
      for(auto negative : state->discrete_negative_asks)asks.push_back(negative.first);
    }
    

    for(auto variable : discrete_variables)
    {
      asks.merge(guard_relation_graph_->get_adjacent_asks(variable.get_name(), phase_type == POINT_PHASE));
    }
    
    for(auto ask : asks)
    {
      if(phase_type == POINT_PHASE  
         && state->owner == result_root
         && PrevSearcher().search_prev(ask->get_guard())){
        // in initial state, conditions about left-hand limits are considered to be invalid
        continue;
      }
      if(judged_asks.count(ask))continue;
     
      // omit judgments of continous guards
      if(state->in_following_step() && judge_continuity(state, ask)){
        judged_asks.insert(ask);
        continue;
      } 
      CheckConsistencyResult check_consistency_result;
      switch(consistency_checker->check_entailment(*relation_graph_, check_consistency_result, ask, phase_type, state->profile)){

      case BRANCH_PAR:
        HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
        push_branch_states(state, check_consistency_result);
        // Since we should choose entailed case in push_branch_states, we go down without break.
      case ENTAILED:
        HYDLA_LOGGER_DEBUG("--- entailed ask ---\n", *(ask->get_guard()));
        if(!guard_relation_graph_->get_entailed(ask))
        {
          relation_graph_->set_expanded(ask->get_child(), true);
          guard_relation_graph_->set_entailed(ask, true);
          positive_asks.insert(ask);
          local_diff[ask->get_child()] = true;
          all_diff.add_constraint(ask->get_child());
          expanded = true;
        }
        unknown_asks.erase(ask);
        judged_asks.insert(ask);
        break;
      case CONFLICTING:
        HYDLA_LOGGER_DEBUG("--- conflicted ask ---\n", *(ask->get_guard()));
        if(guard_relation_graph_->get_entailed(ask))
        {
          state->expanded_diff[ask->get_child()] = false;
          all_diff.add_constraint(ask->get_child());
          negative_asks.insert(ask);
        }
        unknown_asks.erase(ask);
        judged_asks.insert(ask);
        break;
      case BRANCH_VAR:
        HYDLA_LOGGER_DEBUG("--- branched ask ---\n", *(ask->get_guard()));
        unknown_asks.insert(ask);
        break;
      }
      state->profile["# of CheckEntailment"]+= 1;
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
  }while(expanded);

  if(!unknown_asks.empty()){
    throw SimulateError("unknown asks");
  }
  return true;
}

bool PhaseSimulator::judge_continuity(const simulation_job_sptr_t &todo, const ask_t &ask)
{
  // TODO: 何かうまく行かないコーナーケースがある気がするので考える．
  for(auto discrete_ask : todo->discrete_positive_asks)
  {
    if(discrete_ask.first == ask) return false;
  }
  for(auto discrete_ask : todo->discrete_negative_asks)
  {
    if(discrete_ask.first == ask) return false;
  }
  
  VariableFinder finder;
  finder.visit_node(ask->get_guard());
  variable_set_t variables(finder.get_all_variable_set());
  finder.clear();

  for(auto cause : todo->discrete_positive_asks){
    if(!cause.second) finder.visit_node(cause.first->get_child());
  }
  for(auto cause : todo->discrete_negative_asks){
    if(!cause.second) finder.visit_node(cause.first->get_child());
  }
  variable_set_t changing_variables(finder.get_all_variable_set());

  for(auto variable : variables){
    auto differential_pair = todo->owner->variable_map.find(Variable(variable.get_name(), variable.get_differential_count() + 1));
    if(differential_pair == todo->owner->variable_map.end() || differential_pair->second.undefined()) return false;
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
PhaseSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_job_sptr_t& current_todo)
{
  timer::Timer next_todo_timer;

  simulation_job_sptr_t next_todo(new SimulationJob());
  next_todo->id = ++todo_id;
  phase->expanded_diff.insert(current_todo->expanded_diff.begin(), current_todo->expanded_diff.end());
  AlwaysFinder always_finder;
  ConstraintStore non_always;
  always_set_t always_set;
  for(auto constraint : current_todo->expanded_constraints)
  {
    always_finder.find_always(constraint, &always_set, &non_always);
  }
  for(auto constraint : non_always)
  {
    relation_graph_->set_expanded(constraint, false);
    next_todo->expanded_diff[constraint] = false;
  }
  next_todo->parameter_map = phase->parameter_map;

  if(current_todo->phase_type == POINT_PHASE)
  {
    next_todo->phase_type = INTERVAL_PHASE;
    next_todo->owner = phase;
    phase->end_time = phase->current_time;
    next_todo->discrete_positive_asks = current_todo->discrete_positive_asks;
    next_todo->discrete_negative_asks = current_todo->discrete_negative_asks;
    next_todo->prev_map = phase->variable_map;
      
    current_todo->produced_phases.push_back(phase);
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
    

/*    
      if(phase->parent && phase->parent->parent)
      {
      phase->next_pp_candidate_map = phase->parent->parent->next_pp_candidate_map;
      }
      next_pp_candidate_map_t &candidate_map = phase->next_pp_candidate_map;
      VariableFinder var_finder;

      struct PPTimeInfo{
      value_t pp_time;
      bool on_time;
      parameter_map_t parameter_map;
      };
      map<ask_t, PPTimeInfo > calculated_pp_time_map;
      set<string> checked_variables;

      // get changed variables
      for(auto constraint : phase->changed_constraints)
      {
      var_finder.visit_node(constraint);
      }
      HYDLA_LOGGER_DEBUG(max_time);
      variable_set_t changed_variable_set = var_finder.get_all_variable_set();
      value_t time_limit(max_time);
      HYDLA_LOGGER_DEBUG(time_limit);
      time_limit -= phase->current_time;
      HYDLA_LOGGER_DEBUG(time_limit);
      // TODO: 複数のガード条件が同時に成り立つ場合に対応する

      value_t min_time_of_all;

      ask_t min_ask;
      bool min_on_time;
    
      // 各変数に関する最小時刻を更新する．
      for(auto variable : changed_variable_set)
      {
      string var_name = variable.get_name();
      // 既にチェック済みの変数なら省略（x'とxはどちらもxとして扱うため，二回呼ばれないようにする）
      if(checked_variables.count(var_name))continue;
      checked_variables.insert(var_name);

      value_t min_time_for_this_variable;
      if(candidate_map.count(var_name) )min_time_for_this_variable = candidate_map[var_name].pp_time;
      vector<ask_t> asks; guard_relation_graph_->get_adjacent_asks(var_name, asks);
      for(auto ask : asks)
      {
      value_t min_time_for_this_ask;
      bool on_time;
      if(calculated_pp_time_map.count(ask))
      {
      min_time_for_this_ask = calculated_pp_time_map[ask].pp_time;
      on_time = calculated_pp_time_map[ask].on_time;
      }
      else
      {
      var_finder.clear();
      var_finder.visit_node(ask->get_guard());
      variable_set_t related_variables = var_finder.get_all_variable_set();
      variable_map_t related_vm;
      for(auto related_variable : related_variables)
      {
      related_vm[related_variable] = phase->variable_map[related_variable];
      }
      find_min_time_result_t find_min_time_result;

      if(phase->positive_asks.count(ask))
      {
      symbolic_expression::node_sptr negated_node(new Not(ask->get_guard()));
      backend_->call("findMinTime", 3, "etmvtvlt", "f", &negated_node, &related_vm, &time_limit, &find_min_time_result);
      }
      else
      {
      symbolic_expression::node_sptr node(ask->get_guard());
      backend_->call("findMinTime", 3, "etmvtvlt", "f", &node, &related_vm, &time_limit, &find_min_time_result);
      }

      assert(find_min_time_result.size() <= 1);
      if(find_min_time_result.size() == 1)
      {
      min_time_for_this_ask = find_min_time_result[0].time;
      on_time = find_min_time_result[0].on_time;
      }
      }

      if(!min_time_for_this_ask.undefined())
      {
      if(min_time_for_this_variable.undefined()
      || compare_min_time(min_time_for_this_ask, min_time_for_this_variable) == -1)
      {
      CandidateOfNextPP candidate;
      min_time_for_this_variable = min_time_for_this_ask;
      candidate.causes.push_back(DiscreteCause(ask, on_time));
      candidate.pp_time = min_time_for_this_ask;
      candidate_map[var_name] = candidate;
      }
      }
      }
      }
      HYDLA_LOGGER_DEBUG("");
      set<ask_t> checked_asks;
      set<string> min_time_variables;
      // 各変数に関する最小時刻を比較して最小のものを選ぶ．
      for(auto var_name : variable_names)
      {
      if(candidate_map.count(var_name))
      {
      CandidateOfNextPP candidate = candidate_map[var_name];
      assert(candidate.causes.size() == 1);
      DiscreteCause &cause = candidate.causes[0];
      if(min_ask == cause.ask)min_time_variables.insert(var_name);
      if(checked_asks.count(cause.ask) )continue;
      checked_asks.insert(cause.ask);
      if(!min_ask.get() || compare_min_time(candidate.pp_time, min_time_of_all) == -1 )
      {
      HYDLA_LOGGER_DEBUG("");
      min_ask = cause.ask;
      min_time_of_all = candidate.pp_time;
      min_on_time = cause.on_time;
      min_time_variables.clear();
      }
      }
      }
      if(min_ask.get())
      {
      HYDLA_LOGGER_DEBUG_VAR(get_infix_string(min_ask) );
      HYDLA_LOGGER_DEBUG_VAR(min_time_of_all);
      HYDLA_LOGGER_DEBUG_VAR(min_on_time);
      >>>>>>> b6acc7cf7d6b65cb94d8c047472aa8e82dfbe725
      }
*/

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
    pp_time_result_t time_result;
    for(auto ask : phase->get_all_positive_asks())
    {
      symbolic_expression::node_sptr negated_node(new Not(ask->get_guard()));
      find_min_time_result_t find_min_time_result;
      variable_map_t related_vm = get_related_vm(ask->get_guard(), phase->variable_map);
      backend_->call("findMinTime", 3, "etmvtvlt", "f", &negated_node, &related_vm, &time_limit, &find_min_time_result);
      time_result = compare_min_time(time_result, find_min_time_result, ask, false);
    }
    for(auto ask : phase->get_all_negative_asks())
    {
      symbolic_expression::node_sptr node(ask->get_guard());
      find_min_time_result_t find_min_time_result;
      variable_map_t related_vm = get_related_vm(ask->get_guard(), phase->variable_map);
      backend_->call("findMinTime", 3, "etmvtvlt", "f", &node, &related_vm, &time_limit, &find_min_time_result);
      time_result = compare_min_time(time_result, find_min_time_result, ask, true);
    }
    
    if(time_result.empty())
    {
      DCCandidate candidate;
      candidate.time = max_time;
      time_result.push_back(candidate);
    }
    phase_result_sptr_t pr = phase;
    variable_map_t original_vm = pr->variable_map;
    // まずインタラクティブ実行のために最小限の情報だけ整理する
    auto time_it = time_result.begin();
    while(true)
    {
      DCCandidate &candidate = *time_it;
      // 全体を置き換えると，値の上限も下限もない記号定数が消えるので，追加のみを行う
      for(auto par_entry : candidate.parameter_map ){
        pr->parameter_map[par_entry.first] = par_entry.second;
      }
      pr->end_time = current_todo->owner->end_time + candidate.time;
      backend_->call("simplify", 1, "vln", "vl", &pr->end_time, &pr->end_time);

      pr->variable_map = value_modifier->shift_time(pr->current_time, original_vm);

      current_todo->produced_phases.push_back(pr);
      if(candidate.time.undefined() || candidate.time.infinite() )
      {
        pr->cause_for_termination = simulator::TIME_LIMIT;
      }
      else
      {
        for(auto diff_positive : candidate.diff_positive_asks)next_todo->discrete_positive_asks.insert(diff_positive);
        for(auto diff_negative : candidate.diff_negative_asks)next_todo->discrete_negative_asks.insert(diff_negative);

        next_todo->parameter_map = pr->parameter_map;
        next_todo->owner = pr;
        next_todo->prev_map = value_modifier->substitute_time(candidate.time, original_vm);
        pr->todo_list.push_back(next_todo);
      }
      // HAConverter, HASimulator用にTIME_LIMITのtodoも返す
/*
  TODO: implement
  if((opts_->ha_convert_mode || opts_->ha_simulator_mode) && pr->cause_for_termination == TIME_LIMIT)
  {
  next_todo->current_time = pr->end_time;
  next_todo->parameter_map = pr->parameter_map;
  next_todo->owner = pr;
  ret.push_back(next_todo);
  }
*/
      if(++time_it == time_result.end())break;
      pr = make_new_phase(pr);
      pr->todo_list.clear();
      next_todo.reset(new SimulationJob(*next_todo));
      next_todo->produced_phases.clear();
      next_todo->id = ++todo_id;
    }
  }
}

pp_time_result_t PhaseSimulator::compare_min_time(const pp_time_result_t &existing, const find_min_time_result_t &newcomers, const ask_t &ask, bool positive)
{
  pp_time_result_t result;
  if(existing.empty())
  {
    for(auto newcomer :newcomers)
    {
      map<ask_t, bool> positives, negatives;
      if(positive) positives[ask] = newcomer.on_time;
      else negatives[ask] = newcomer.on_time;
      DCCandidate candidate(newcomer.time, positives, negatives, newcomer.parameter_map);
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
          DCCandidate candidate(existing_it->time, existing_it->diff_positive_asks, existing_it->diff_negative_asks, less_map);
          result.push_back(candidate);
        }
        for(auto equal_map : compare_result.equal_maps)
        {
          map<ask_t, bool> positives = existing_it->diff_positive_asks,
            negatives = existing_it->diff_negative_asks;
          if(positive) positives[ask] = newcomer.on_time;
          else negatives[ask] = newcomer.on_time;
          DCCandidate candidate(existing_it->time, positives, negatives, equal_map);          result.push_back(candidate);
        }
        for(auto greater_map : compare_result.greater_maps)
        {
          map<ask_t, bool> positives,
            negatives;
          if(positive) positives[ask] = newcomer.on_time;
          else negatives[ask] = newcomer.on_time;
          DCCandidate candidate(newcomer.time, positives, negatives, greater_map);
          result.push_back(candidate);
        }
      }
    }
  }
  return result;
}

void PhaseSimulator::apply_diff(const phase_result_sptr_t &phase)
{
  // TODO: ask_relation_graph_についても適切に設定し直す
  for(auto diff: phase->expanded_diff)
  {
    relation_graph_->set_expanded(diff.first, diff.second);
  }
  for(auto diff: phase->adopted_module_diff)
  {
    relation_graph_->set_adopted(diff.first, diff.second);
  }
}

void PhaseSimulator::revert_diff(const phase_result_sptr_t &phase)
{
  // TODO: ask_relation_graph_についても適切に設定し直す
  for(auto diff: phase->expanded_diff)
  {
    relation_graph_->set_expanded(diff.first, !diff.second);
  }
  for(auto diff: phase->adopted_module_diff)
  {
    relation_graph_->set_adopted(diff.first, !diff.second);
  }
}

}
}
