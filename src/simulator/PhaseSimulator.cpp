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

void PhaseSimulator::process_todo(simulation_todo_sptr_t &todo)
{
  timer::Timer phase_timer;
  module_set_container->reset(todo->ms_to_visit);
  make_results_from_todo(todo);
  for(auto phase : todo->children)
  {
    make_next_todo(phase, todo);
  }
  todo->profile["PhaseResult"] += phase_timer.get_elapsed_us();  
}

PhaseSimulator::result_list_t PhaseSimulator::calculate_phase_result(simulation_todo_sptr_t& todo)
{
  result_list_t result;
  // do nothing
  return result;
}

void PhaseSimulator::make_results_from_todo(simulation_todo_sptr_t& todo)
{
  timer::Timer preprocess_timer;
  result_list_t result;

  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &todo->parameter_map);
  backend_->call("addParameterConstraint", 1, "csn", "", &todo->initial_constraint_store);
  consistency_checker->set_prev_map(&todo->prev_map);
  if(todo->phase_type == PointPhase)
  {
    set_simulation_mode(PointPhase);
    relation_graph_->set_ignore_prev(true);
  }else{
    set_simulation_mode(IntervalPhase);
    relation_graph_->set_ignore_prev(false);
  }
  if(todo->parent == result_root)
  {
    // in the initial state, set all modules expanded
    for(auto module : module_set_container->get_max_module_set())
    {
      relation_graph_->set_expanded(module.second, true);
      todo->expanded_constraints.add_constraint(module.second);
      todo->expanded_diff.insert(make_pair(module.second, true));
    }
  }
  else
  {
    // apply diff
    for(auto diff : todo->expanded_diff)
    {
      HYDLA_LOGGER_DEBUG_VAR(diff.first);
      relation_graph_->set_expanded(diff.first, diff.second);
    }
  }

  todo->profile["Preprocess"] += preprocess_timer.get_elapsed_us();

  while(module_set_container->has_next())
  {
    constraint_diff_t local_diff;

    if(!relation_graph_is_taken_over)
    {
      // TODO: relation_graph_の状態が親フェーズから直接引き継いだものでない場合は，差分を用いることができないので全制約に関して設定する必要がある．
      assert(0);
    }
    else
    {
      if(todo->phase_type == PointPhase && todo->parent != result_root)
      {
        // TODO: prevガード条件でない場合の対応を考える
        for(auto positive : todo->discrete_positive_asks)
        {
          if(positive.on_time)
          {
            relation_graph_->set_expanded(positive.ask->get_child(), true);
            local_diff.insert(make_pair(positive.ask->get_child(), true));
            todo->positive_asks.insert(positive.ask);
          }
        }
        for(auto negative : todo->discrete_negative_asks)
        {
          if(negative.on_time)
          {
            relation_graph_->set_expanded(negative.ask->get_child(), false);
            local_diff.insert(make_pair(negative.ask->get_child(), false));
            todo->negative_asks.insert(negative.ask);
          }
        }
      }
      else if(todo->phase_type == IntervalPhase)
      {
        // IP のガード条件はこの時点では判定が難しいので，すべてoffにしておくことにする．TODO: どうにかする．
        for(auto ask : all_asks)
        {
          relation_graph_->set_expanded(ask->get_child(), false);
        }
      }
    }

    // TODO: unadopted_ms も差分をとるようにして使いたい
    module_set_t unadopted_ms = module_set_container->unadopted_module_set();
    std::string module_sim_string = "\"ModuleSet" + unadopted_ms.get_name() + "\"";
    timer::Timer ms_timer;

    relation_graph_->set_adopted(unadopted_ms, false);
    guard_relation_graph_->set_adopted(unadopted_ms, false);
    simulate_ms(unadopted_ms, todo, local_diff);
    guard_relation_graph_->set_adopted(unadopted_ms, true);
    relation_graph_->set_adopted(unadopted_ms, true);

    todo->profile[module_sim_string] += ms_timer.get_elapsed_us();

    todo->positive_asks.clear();
    todo->negative_asks.clear();

    // revert diff
    for(auto diff : local_diff)
    {
      relation_graph_->set_expanded(diff.first, !diff.second);
    }
  }

  if(todo->profile["# of CheckConsistency"]) todo->profile["Average of CheckConsistency"] = todo->profile["CheckConsistency"] / todo->profile["# of CheckConsistency"];
  if(todo->profile["# of CheckEntailment"]) todo->profile["Average of CheckEntailment"] =  todo->profile["CheckEntailment"] / todo->profile["# of CheckEntailment"];
}


void PhaseSimulator::simulate_ms(const module_set_t& unadopted_ms, simulation_todo_sptr_t& todo, constraint_diff_t &local_diff)
{
  HYDLA_LOGGER_DEBUG("\n--- next unadopted module set ---\n", unadopted_ms.get_infix_string());

  timer::Timer cc_timer;
  bool consistent = calculate_closure(todo, local_diff);
  todo->profile["CalculateClosure"] += cc_timer.get_elapsed_us();
  todo->profile["# of CalculateClosure"]++;

  // suspected
  if(!consistent)
  {
    timer::Timer timer;
    HYDLA_LOGGER_DEBUG("INCONSISTENT");
    for(auto module_set : consistency_checker->get_inconsistent_module_sets())
    {
      module_set_container->generate_new_ms(todo->unadopted_mss, module_set);
    }
    todo->profile["MscGenerateNewMS"] += timer.get_elapsed_us();
    return; 
  }
  HYDLA_LOGGER_DEBUG("CONSISTENT");

  timer::Timer postprocess_timer;

  // suspected
  {
    timer::Timer timer;
    module_set_container->remove_included_ms_by_current_ms();
    todo->unadopted_mss.insert(unadopted_ms);
    todo->profile["RemoveIncludedMs"] += timer.get_elapsed_us();
  }

  phase_result_sptr_t phase = make_new_phase(todo);

  vector<variable_map_t> create_result = consistency_checker->get_result_maps();
  if(create_result.size() != 1)
  {
    throw SimulateError("result variable map is not single.");
  }
  phase->variable_map = create_result[0];


  phase->current_constraints = todo->current_constraints = relation_graph_->get_constraints();
  if(todo->in_following_step()){
    timer::Timer timer;
    phase->changed_constraints = difference_calculator_.get_difference_constraints();
    if(phase->phase_type == IntervalPhase && phase->parent && phase->parent->parent){
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
    else if(phase->phase_type == PointPhase && phase->parent){
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

  for(auto positive_ask : todo->positive_asks)
  {
    todo->expanded_constraints.add_constraint(positive_ask->get_child());
  }
  backend_->call("createParameterMap", 0, "", "mp", &phase->parameter_map);
  
/*  TODO: implement
  if(opts_->assertion || break_condition_.get() != NULL){
    timer::Timer entailment_timer;

    backend_->call("resetConstraintForVariable", 0, "","");
    std::string fmt = "mv0";
    fmt += (phase->phase_type==PointPhase)?"n":"t";
    backend_->call("addConstraint", 1, fmt.c_str(), "", &phase->variable_map);
    backend_->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);

    if(opts_->assertion)
    {
      HYDLA_LOGGER_DEBUG("%% check_assertion");
      CheckConsistencyResult cc_result;
      switch(consistency_checker->check_entailment(*relation_graph_, cc_result, symbolic_expression::node_sptr(new symbolic_expression::Not(opts_->assertion)), todo->phase_type)){
      case CONFLICTING:
      case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
        std::cout << "Assertion Failed!" << std::endl;
        HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
        phase->cause_for_termination = ASSERTION;
        break;
      case ENTAILED:
        break;
      case BRANCH_PAR:
        HYDLA_LOGGER_DEBUG("%% failure of assertion depends on conditions of parameters");
        push_branch_states(todo, cc_result);
        std::cout << "Assertion Failed!" << std::endl;
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
  
  phase->module_set = unadopted_ms;
  todo->children.push_back(phase);
  todo->profile["Postprocess"] += postprocess_timer.get_elapsed_us();
  return;
}

void PhaseSimulator::push_branch_states(simulation_todo_sptr_t &original, CheckConsistencyResult &result){
  simulation_todo_sptr_t branch_state_false(create_new_simulation_phase(original));
  branch_state_false->initial_constraint_store.add_constraint_store(result.inconsistent_store);
  original->parent->todo_list.push_back(branch_state_false);
  original->initial_constraint_store.add_constraint_store(result.consistent_store);
  backend_->call("resetConstraintForParameter", 1, "csn", "", &original->initial_constraint_store);
}

phase_result_sptr_t PhaseSimulator::make_new_phase(simulation_todo_sptr_t& todo)
{
  phase_result_sptr_t phase(new PhaseResult(*todo));
  phase->id = ++phase_sum_;
  todo->parent->children.push_back(phase);
  return phase;
}


phase_result_sptr_t PhaseSimulator::make_new_phase(const phase_result_sptr_t& original)
{
  phase_result_sptr_t phase(new PhaseResult(*original));
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
  module_set_container = msc;
  result_root = root;

  simulator::module_set_t ms = module_set_container->get_max_module_set();

  relation_graph_.reset(new RelationGraph(ms)); 

  guard_relation_graph_.reset(new AskRelationGraph(ms));


  if(opts_->dump_relation){
    relation_graph_->dump_graph(std::cout);
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

  // search asks and initialize prev_asks_ and ask_map
  ac.collect_ask(constraints, &pat, &nat, &all_asks);
  FullInformation* root_information = new FullInformation();
  prev_asks_ = root_information->negative_asks = all_asks;
  result_root->set_full_information(root_information);
  for(auto it = prev_asks_.begin(); it != prev_asks_.end();){
    int id = (*it)->get_id();
    ask_map[id] = *it;
    VariableFinder finder;
    finder.visit_node((*it)->get_guard());
    if(!finder.get_variable_set().empty()){
      prev_asks_.erase(it++);
    }else{
      it++;
    }
  }

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

simulation_todo_sptr_t PhaseSimulator::create_new_simulation_phase(const simulation_todo_sptr_t& old) const
{
  simulation_todo_sptr_t sim(new SimulationTodo(*old));
  return sim;
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
  HYDLA_LOGGER_DEBUG_VAR(vm);
}

void PhaseSimulator::set_backend(backend_sptr_t back)
{
  backend_ = back;
  consistency_checker.reset(new ConsistencyChecker(backend_));
}

void PhaseSimulator::set_simulation_mode(const PhaseType& phase)
{
  current_phase_ = phase;
}

bool PhaseSimulator::calculate_closure(simulation_todo_sptr_t& state, constraint_diff_t &local_diff)
{
  ask_set_t& positive_asks = state->positive_asks;
  ask_set_t& negative_asks = state->negative_asks;

  ask_set_t unknown_asks;

  AskCollector  ask_collector;

  bool expanded;
  do{
    for(auto diff : state->expanded_diff)state->diff_constraints.add_constraint(diff.first);
    for(auto diff : state->adopted_modules_diff)state->diff_constraints.add_constraint(diff.first);
    for(auto diff : local_diff)state->diff_constraints.add_constraint(diff.first);

    HYDLA_LOGGER_DEBUG_VAR(state->diff_constraints);
    {
      timer::Timer consistency_timer;
      CheckConsistencyResult cc_result;
      cc_result = consistency_checker->check_consistency(*relation_graph_, state->diff_constraints, state->phase_type, state->profile);

      state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us(); 
      state->profile["# of CheckConsistency"]++;
      if(!cc_result.consistent_store.consistent()){
        HYDLA_LOGGER_DEBUG("%% inconsistent for all cases");
        return false;
      }else if (!cc_result.inconsistent_store.consistent()){
        HYDLA_LOGGER_DEBUG("%% consistent for all cases");
      }else{
        HYDLA_LOGGER_DEBUG("%% consistency depends on conditions of parameters\n");
        push_branch_states(state, cc_result);
      }
    }

    if(state->in_following_step()){
      difference_calculator_.collect_ask(guard_relation_graph_,
                                         positive_asks, negative_asks, unknown_asks);
    }else{
      ask_collector.collect_ask(state->expanded_constraints,
                                &positive_asks,
                                &negative_asks,
                                &unknown_asks);
    }

    timer::Timer entailment_timer;

    {
      expanded = false;
      auto it  = unknown_asks.begin();
      while(it != unknown_asks.end())
      {
        if(state->phase_type == PointPhase  
           && state->parent == result_root
           && PrevSearcher().search_prev((*it)->get_guard())){
          // in initial state, conditions about left-hand limits are considered to be invalid
          negative_asks.insert(*it);
          unknown_asks.erase(it++);
          continue;
        }
        if(state->phase_type == IntervalPhase && 
           state->in_following_step()){
/* TOOD: これが何なのか考える
   if(!state->discrete_causes.find(*it)->second){
   if(difference_calculator_.is_continuous(state, (*it)->get_guard())){
   if(state->parent->get_all_positive_asks().count(*it)){

   relation_graph_->set_expanded((*it)->get_child(), true);
   expanded = true;
   }
   unknown_asks.erase(it++);
   continue;
   }
   }
*/
        }

        CheckConsistencyResult check_consistency_result;
        switch(consistency_checker->check_entailment(*relation_graph_, check_consistency_result, *it, state->phase_type, state->profile)){
        case ENTAILED:
          HYDLA_LOGGER_DEBUG("--- entailed ask ---\n", *((*it)->get_guard()));
          positive_asks.insert(*it);
          relation_graph_->set_expanded((*it)->get_child(), true);
          if(!state->parent->get_all_positive_asks().count(*it)){
            difference_calculator_.add_difference_constraints((*it)->get_child(), relation_graph_);
          }
          unknown_asks.erase(it++);
          expanded = true;
          break;
        case CONFLICTING:
          HYDLA_LOGGER_DEBUG("--- conflicted ask ---\n", *((*it)->get_guard()));
          negative_asks.insert(*it);
          if(!state->parent->get_all_negative_asks().count(*it)){
            difference_calculator_.add_difference_constraints((*it)->get_child(), relation_graph_);
          }
          unknown_asks.erase(it++);
          break;
        case BRANCH_VAR:
          HYDLA_LOGGER_DEBUG("--- branched ask ---\n", *((*it)->get_guard()));
          it++;
          break;
        case BRANCH_PAR:
          HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
          push_branch_states(state, check_consistency_result);
          break;
        }
        state->profile["# of CheckEntailment"]+= 1;
      }
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
  }while(expanded);

  if(!unknown_asks.empty()){
    throw SimulateError("unknown asks");
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
    related_vm[related_variable] = vm.find(related_variable)->second;
  }
  return related_vm;
}

void
PhaseSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo)
{
  timer::Timer next_todo_timer;

  simulation_todo_sptr_t next_todo(new SimulationTodo());
  next_todo->parent = phase;
  next_todo->ms_to_visit = module_set_container->get_full_ms_list();
  
  AlwaysFinder always_finder;
  ConstraintStore non_always;
  for(auto constraint : current_todo->expanded_constraints)
  {
    always_finder.find_always(constraint, &next_todo->expanded_always, &non_always);
  }
  for(auto constraint : non_always)
  {
    relation_graph_->set_expanded(constraint, false);
    next_todo->expanded_diff.insert(make_pair(constraint, false));
  }
  next_todo->parameter_map = phase->parameter_map;

  if(current_phase_ == PointPhase)
  {
    next_todo->phase_type = IntervalPhase;
    next_todo->current_time = phase->current_time;
    // TODO: 離散変化した変数が関わるガード条件はここから取り除く必要が有りそう（単純なコピーではだめ）
    next_todo->discrete_positive_asks = current_todo->discrete_positive_asks;
    next_todo->discrete_negative_asks = current_todo->discrete_negative_asks;
    next_todo->prev_map = phase->variable_map;
    current_todo->children.push_back(phase);
    phase->todo_list.push_back(next_todo);
  }
  else
  {
    PhaseSimulator::replace_prev2parameter(*phase->parent, phase->variable_map, phase->parameter_map);
    next_todo->phase_type = PointPhase;
    
    HYDLA_LOGGER_DEBUG_VAR(phase->variable_map);

    backend_->call("resetConstraint", 0, "", "");
    backend_->call("addParameterConstraint", 1, "mp", "", &phase->parameter_map);
    
    pp_time_result_t time_result;
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

    for(auto ask : phase->get_all_positive_asks())
    {
      symbolic_expression::node_sptr negated_node(new Not(ask->get_guard()));
      find_min_time_result_t find_min_time_result;
      variable_map_t related_vm = get_related_vm(ask->get_guard(), phase->variable_map);
      backend_->call("findMinTime", 3, "etmvtvlt", "f", &negated_node, &related_vm, &time_limit, &find_min_time_result);
      compare_min_time(time_result, find_min_time_result, ask, false);
    }
    for(auto ask : phase->get_all_negative_asks())
    {
      symbolic_expression::node_sptr node(ask->get_guard());
      find_min_time_result_t find_min_time_result;
      variable_map_t related_vm = get_related_vm(ask->get_guard(), phase->variable_map);
      backend_->call("findMinTime", 3, "etmvtvlt", "f", &node, &related_vm, &time_limit, &find_min_time_result);
      compare_min_time(time_result, find_min_time_result, ask, true);
    }

    if(time_result.empty())
    {
      DCCandidate candidate;
      candidate.time = max_time;
      time_result.push_back(candidate);
    }
    result_list_t results;
    phase_result_sptr_t pr = next_todo->parent;
    // まずインタラクティブ実行のために最小限の情報だけ整理する
    auto time_it = time_result.begin();
    HYDLA_LOGGER_DEBUG_VAR(time_result.size());
    while(true)
    {
      DCCandidate &candidate = *time_it;
      // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
      for(auto par_entry : candidate.parameter_map ){
        pr->parameter_map[par_entry.first] = par_entry.second;
      }
      pr->end_time = current_todo->current_time + candidate.time;
      backend_->call("simplify", 1, "vln", "vl", &pr->end_time, &pr->end_time);
      results.push_back(pr);
      if(++time_it == time_result.end())break;
      pr = make_new_phase(pr);
      current_todo->children.push_back(pr);
    }

    unsigned int result_it = 0;

    // todoを実際に作成する
    time_it = time_result.begin();
    while(true)
    {
      pr = results[result_it];
      DCCandidate &candidate = *time_it++;
      if(candidate.time.undefined() || candidate.time.infinite() )
      {
        pr->cause_for_termination = simulator::TIME_LIMIT;
      }
      else
      {
        next_todo->discrete_positive_asks = candidate.diff_positive_asks;
        next_todo->discrete_negative_asks = candidate.diff_negative_asks;
        next_todo->current_time = pr->end_time;
        next_todo->parameter_map = pr->parameter_map;
        next_todo->parent = pr;
        next_todo->prev_map = value_modifier->substitute_time(candidate.time, phase->variable_map);
        pr->todo_list.push_back(next_todo);
      }

 
      // HAConverter, HASimulator用にTIME_LIMITのtodoも返す
/*
  TODO: implement
  if((opts_->ha_convert_mode || opts_->ha_simulator_mode) && pr->cause_for_termination == TIME_LIMIT)
  {
  next_todo->current_time = pr->end_time;
  next_todo->parameter_map = pr->parameter_map;
  next_todo->parent = pr;
  ret.push_back(next_todo);
  }
*/
      if(++result_it >= results.size())break;
      next_todo = create_new_simulation_phase(next_todo);
    }
  }
  phase->variable_map = value_modifier->shift_time(phase->current_time, phase->variable_map);
}

void PhaseSimulator::compare_min_time(pp_time_result_t &existing, const find_min_time_result_t &newcomers, const ask_t &ask, bool positive)
{
  if(existing.empty())
  {
    for(auto newcomer :newcomers)
    {
      std::list<DiscreteAsk> positives, negatives;
      if(positive) positives.push_back(DiscreteAsk(ask, newcomer.on_time));
      else negatives.push_back(DiscreteAsk(ask, newcomer.on_time));
      DCCandidate candidate(newcomer.time, positives, negatives, newcomer.parameter_map);
      existing.push_front(candidate);      
    }
    return;
  }

  for(auto newcomer : newcomers)
  {
    for(auto existing_it = existing.begin(); existing_it != existing.end();)
    {
      compare_min_time_result_t compare_result;
      backend_->call("compareMinTime", 4, "vltvltmpmp", "cp", &existing_it->time, &newcomer.time, &existing_it->parameter_map, &newcomer.parameter_map, &compare_result);
      if(!compare_result.equal_maps.empty() || !compare_result.greater_maps.empty())
      {
        //existing one must be updated
        for(auto less_map : compare_result.less_maps)
        {
          DCCandidate candidate(existing_it->time, existing_it->diff_positive_asks, existing_it->diff_negative_asks, less_map);
          existing.push_front(candidate);
        }
        for(auto equal_map : compare_result.equal_maps)
        {
       
j          std::list<DiscreteAsk> positives = existing_it->diff_positive_asks,
                                 negatives = existing_it->diff_negative_asks;
          if(positive) positives.push_back(DiscreteAsk(ask, newcomer.on_time));
          else negatives.push_back(DiscreteAsk(ask, newcomer.on_time));
          DCCandidate candidate(existing_it->time, positives, negatives, equal_map);   
          existing.push_front(candidate);
        }
        for(auto greater_map : compare_result.greater_maps)
        {
          std::list<DiscreteAsk> positives, negatives;
          if(positive) positives.push_back(DiscreteAsk(ask, newcomer.on_time));
          else negatives.push_back(DiscreteAsk(ask, newcomer.on_time));
          DCCandidate candidate(newcomer.time, positives, negatives, greater_map);
          existing.push_front(candidate);
        }
        existing_it = existing.erase(existing_it);
      }
      else 
      {
        existing_it++;
      }
    }
  }
  return;
}


}
}
