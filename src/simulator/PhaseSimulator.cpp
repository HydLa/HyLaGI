#include <iostream>
#include <thread>
#include <mutex>

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

#include "MinTimeCalculator.h"

#include "Backend.h"

#include "TreeInfixPrinter.h"

#include "IntervalNewton.h"

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
  else
  {
    todo->set_parameter_constraint(todo->parent->get_parameter_constraint());
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

      // warn against unreferenced variables
      for(auto var: *variable_set_)
      {
        if(var.get_differential_count() == 0 &&
           !phase->variable_map.count(var))HYDLA_LOGGER_WARN(var, " is completely unbound at phase... ", *phase);
      }

      if(aborting)break;
    }
  }

  todo->profile["PhaseResult"] += phase_timer.get_elapsed_us();
}


std::list<phase_result_sptr_t> PhaseSimulator::make_results_from_todo(phase_result_sptr_t& todo)
{
  list<phase_result_sptr_t> result_list;
  timer::Timer preprocess_timer;

  const ConstraintStore parameter_cons = todo->get_parameter_constraint();
  std::vector<std::thread> threads(opts_->num_threads);
  for (int i=0; i<opts_->num_threads; ++i) {
    threads[i] = std::thread ([i,&parameter_cons,&todo,this](){
        (*backends_)[i]->call("resetConstraint", false, 0, "", "");
        (*backends_)[i]->call("addParameterConstraint", true, 1, "csn", "", &parameter_cons);
        (*backends_)[i]->call("addParameterConstraint", true, 1, "csn", "", &todo->additional_constraint_store);
        consistency_checkers[i]->set_prev_map(&todo->prev_map);
    });
  }
  for (int i=0; i<opts_->num_threads; ++i) {
    threads[i].join();
  }
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
      cut_high_order_epsilon(backend_.get(),phase, opts_->epsilon_mode);
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
          if(!consistency_checker->check_continuity(var, phase->variable_map))
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
  //For epsilon mode
  // if(opts_->epsilon_mode >= 0){
  //   parameter_map_t tmp_pm;
  //   int i=0;
  //   for(auto parmap : parameter_maps){
  //     if(i==0){
  //       tmp_pm = parmap;
  //     }
  //     for(auto par : parmap){
  //       HYDLA_LOGGER_DEBUG("#epsilon par  : ",par.first," : ",par.second);
  //     }
  //     i++;
  //   }
  //   parameter_maps.clear();
  //   parameter_maps.push_back(tmp_pm);
  // }
  ConstraintStore parameter_cons;
  backend_->call("getParameterConstraint", true, 0, "", "cs", &parameter_cons);
  return parameter_cons;
}

void PhaseSimulator::push_branch_states(phase_result_sptr_t &original, CheckConsistencyResult &result){
  phase_result_sptr_t branch_state_false(new PhaseResult());
  // copy necesarry information for branching
  branch_state_false->phase_type = original->phase_type;
  branch_state_false->id = ++phase_sum_;
  branch_state_false->step = original->step;
  branch_state_false->current_time = original->current_time;
  branch_state_false->prev_map = original->prev_map;
  branch_state_false->set_parameter_constraint(original->get_parameter_constraint());
  branch_state_false->additional_constraint_store = original->additional_constraint_store;

  branch_state_false->discrete_differential_set = original->discrete_differential_set;
  branch_state_false->parent = original->parent;
  branch_state_false->discrete_asks = original->discrete_asks;
  branch_state_false->always_list = original->always_list;
  branch_state_false->diff_sum = original->diff_sum;
  branch_state_false->next_pp_candidate_map = original->next_pp_candidate_map;


  branch_state_false->additional_constraint_store.add_constraint_store(replace_prev_store(original->parent, result.inconsistent_store));

  original->parent->todo_list.push_back(branch_state_false);
  original->additional_constraint_store.add_constraint_store(replace_prev_store(original->parent, result.consistent_store));
  if(original->phase_type == INTERVAL_PHASE)
  {
    original->prev_map = original->parent->variable_map;
    consistency_checker->set_prev_map(&original->prev_map);
  }
  HYDLA_LOGGER_DEBUG_VAR(original->additional_constraint_store);
  backend_->call("addParameterConstraint", true, 1, "csn", "", &original->additional_constraint_store);
  HYDLA_LOGGER_DEBUG_VAR(*branch_state_false);
  HYDLA_LOGGER_DEBUG_VAR(*original);
}

ConstraintStore PhaseSimulator::replace_prev_store(PhaseResult *parent, ConstraintStore orig)
{
  PrevReplacer replacer(*parent, *simulator_);
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
  }

  FullInformation root_information;
  root_information.negative_asks = relation_graph_->get_all_asks();
  result_root->set_full_information(root_information);

  if(opts_->max_time != ""){
    max_time = node_sptr(new se::Number(opts_->max_time));
  }else{
    max_time = node_sptr(new se::Infinity());
  }

  aborting = false;

  backend_->set_variable_set(*variable_set_);
  value_modifier.reset(new ValueModifier(*backend_));
}

void PhaseSimulator::replace_prev2parameter(PhaseResult &phase,
                                            variable_map_t &vm)
{
  PrevReplacer replacer(*phase.parent, *simulator_);
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



void PhaseSimulator::set_backend(backends_vector_t& back)
{
  backends_ = &back;
  backend_ = back[0];
  for (int i=0; i<opts_->num_threads; ++i) {
    consistency_checkers.push_back(boost::make_shared<ConsistencyChecker>(back[i]));
  }
  consistency_checker = consistency_checkers[0];
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
  const PhaseType phase_type = phase->phase_type;
  variable_set_t discrete_variables = get_discrete_variables(diff_sum, phase_type);

  do{
    HYDLA_LOGGER_DEBUG_VAR(diff_sum);
    expanded = false;
    timer::Timer entailment_timer;

    set<ask_t> adjacents;
    if(phase->phase_type == POINT_PHASE || !phase->in_following_step()){
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
    timer::Timer for_loop_timer;
    struct {
      std::mutex phase;
      std::mutex unknown_asks;
      std::mutex negative_asks;
      std::mutex diff_sum;
      std::mutex expanded_always;
      std::mutex positive_asks;
      std::mutex local_diff_sum;
      std::mutex expanded;
    } st_mtx;
    auto check_entailment_worker = [&phase,&phase_type,&unknown_asks,&negative_asks,&diff_sum,&expanded_always,&positive_asks,&local_diff_sum,&expanded,&st_mtx,this](asks_t::iterator begin, asks_t::iterator end, const int thread_number)
    {
      auto ask_it = begin;
      for(bool loop=true; loop;)
      {
        if (ask_it == end)
          loop = false;
        st_mtx.phase.lock();
        phase->profile["# of CheckEntailment_Loop"]+= 1;
        st_mtx.phase.unlock();
        const auto ask = *ask_it;

        if(phase_type == POINT_PHASE
            // in initial phase, conditions about left-hand limits are considered to be invalid
            && phase->parent == result_root.get()
            && PrevSearcher().search_prev(ask))
        {
          st_mtx.unknown_asks.lock();
          ask_it = unknown_asks.erase(ask_it);
          st_mtx.unknown_asks.unlock();
          continue;
        }

        CheckConsistencyResult check_consistency_result;
        switch(consistency_checkers[thread_number]->check_entailment(*relation_graph_, check_consistency_result, ask->get_guard(), ask->get_child(), unknown_asks, phase_type, phase->profile)){

          case BRANCH_PAR:
            HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
            st_mtx.phase.lock();
            push_branch_states(phase, check_consistency_result);
            st_mtx.phase.unlock();
            // Since we choose entailed case in push_branch_states, we go down without break.
          case ENTAILED:
            HYDLA_LOGGER_DEBUG("\n--- entailed ask ---\n", get_infix_string(ask));
            if(!relation_graph_->get_entailed(ask))
            {
              st_mtx.negative_asks.lock();
              if(negative_asks.erase(ask))
              {
                st_mtx.diff_sum.lock();
                diff_sum.erase(ask->get_child());
                st_mtx.diff_sum.unlock();
              }
              else
              {
                st_mtx.expanded_always.lock();
                expanded_always.add_constraint_store(relation_graph_->get_always_list(ask));
                st_mtx.expanded_always.unlock();
                st_mtx.positive_asks.lock();
                positive_asks.insert(ask);
                st_mtx.positive_asks.unlock();
                st_mtx.diff_sum.lock();
                diff_sum.add_constraint(ask->get_child());
                st_mtx.diff_sum.unlock();
              }
              st_mtx.negative_asks.unlock();
              st_mtx.local_diff_sum.lock();
              local_diff_sum.add_constraint(ask->get_child());
              st_mtx.local_diff_sum.unlock();
              relation_graph_->set_entailed(ask, true);
            }
            st_mtx.expanded.lock();
            expanded = true;
            st_mtx.expanded.unlock();
            st_mtx.unknown_asks.lock();
            ask_it = unknown_asks.erase(ask_it);
            st_mtx.unknown_asks.unlock();
            break;
          case CONFLICTING:
            HYDLA_LOGGER_DEBUG("\n--- conflicted ask ---\n", get_infix_string(ask));
            if(relation_graph_->get_entailed(ask))
            {
              st_mtx.positive_asks.lock();
              if(positive_asks.erase(ask))
              {
                st_mtx.diff_sum.lock();
                diff_sum.erase(ask->get_child());
                st_mtx.diff_sum.unlock();
              }
              else
              {
                st_mtx.negative_asks.lock();
                negative_asks.insert(ask);
                st_mtx.negative_asks.unlock();
                st_mtx.diff_sum.lock();
                diff_sum.add_constraint(ask->get_child());
                st_mtx.diff_sum.unlock();
              }
              st_mtx.positive_asks.unlock();
              st_mtx.local_diff_sum.lock();
              local_diff_sum.add_constraint(ask->get_child());
              st_mtx.local_diff_sum.unlock();
              relation_graph_->set_entailed(ask, false);
            }
            st_mtx.expanded.lock();
            expanded = true;
            st_mtx.expanded.unlock();
            st_mtx.unknown_asks.lock();
            ask_it = unknown_asks.erase(ask_it);
            st_mtx.unknown_asks.unlock();
            break;
          case BRANCH_VAR:
            HYDLA_LOGGER_DEBUG("\n--- branched ask ---\n", get_infix_string(ask));
            ask_it++;
            break;
        }
        st_mtx.phase.lock();
        phase->profile["# of CheckEntailment"]+= 1;
        st_mtx.phase.unlock();
      }
    };
    const int num_asks = unknown_asks.size();
    if (num_asks != 0) {
      const int max_threads = opts_->num_threads;
      const int num_threads = num_asks>max_threads ? max_threads : num_asks;
      const int asks_per_thread = num_asks/num_threads;
      int odd_asks = num_asks%num_threads;
      struct its_t {
        asks_t::iterator start, end;
      };
      std::vector<struct its_t> its(num_threads);
      asks_t::iterator itr = unknown_asks.begin();
      for (int i=0; i<num_threads; ++i) {
        its[i].start = itr;
        // its[i].end is NOT the same one as set::end(), but previous one of it.
        // Because we should not share iterators across several threads.
        if (odd_asks > 0) {
          std::advance(itr, asks_per_thread);
          odd_asks--;
        } else {
          std::advance(itr, asks_per_thread-1);
        }
        its[i].end = itr;
        itr++;
      }
      std::vector<std::thread> threads(num_threads);
      for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread(check_entailment_worker, its[i].start, its[i].end, i);
      }
      for (int i = 0; i < num_threads; ++i) {
        threads[i].join();
      }
    }
    phase->profile["CheckEntailment_Loop"] += for_loop_timer.get_elapsed_us();
    phase->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
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
    discrete_variables = get_discrete_variables(local_diff_sum, phase_type);
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

find_min_time_result_t PhaseSimulator::find_min_time_test(phase_result_sptr_t &phase, const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed)
{
  HYDLA_LOGGER_DEBUG("#epsilon find min time start");
  // 現在のガード条件(ask)に関する最小時刻の探索
  find_min_time_result_t min_time_for_this_ask;
  min_time_for_this_ask = calculate_tmp_min_time(phase, guard, min_time_calculator, guard_time_map, original_vm, time_limit, entailed);
  return min_time_for_this_ask;
}

find_min_time_result_t PhaseSimulator::calculate_tmp_min_time(phase_result_sptr_t &phase, const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed)
{
  find_min_time_result_t ret;
  bool limit_is_zero = false;
  //前回の離散変化条件との比較
  bool same_guard;
  for(auto entry: phase->parent->discrete_asks){
    if(guard == (*(entry.first)).get_guard()){
      same_guard = true;
      break;
    }
  }
  // 一時的な最小時間候補の導出
  find_min_time_result_t min_time_candidate;

  min_time_candidate = find_min_time(guard, min_time_calculator, guard_time_map, original_vm, time_limit, entailed, phase);

  find_min_time_result_t before = min_time_candidate;

  for(auto target : min_time_candidate){
    //最小時間候補を検査し、不適切な候補の削減を行う
    value_t target_time = target.time;
    backend_->call("limitIsZero", true, 1, "vln", "b", &target_time, &limit_is_zero);
    if(!limit_is_zero || !same_guard){
      //時刻をずらさない場合
      ret.push_back(target);
      continue;
    }
    // // TODO: 2回以上の離散時刻の変更に対応していない
    // while(limit_is_zero && same_guard){ //再帰でする必要がありそう
    //   //時刻をずらす場合
    //   find_min_time_result_t tmp_min_time_result;
    //   tmp_min_time_result = min_time_calculator.calculate_min_time(&guard_time_map, guard, entailed, target_time);
    //   for(auto tmp_target : tmp_min_time_result){
    //     bool ret;
    //     backend_->call("limitIsZero", 1, "vln", "b", &target_time, &ret);
    //     limit_is_zero = limit_is_zero & ret;
    //     ret.push_back(tmp_target);
    //   }
    // }

    // 2回以上の離散時刻の変更に対応している
    if(limit_is_zero && same_guard){
      //時刻ずらし
      find_min_time_result_t tmp_min_time;
      node_sptr moving_time = target_time.get_node();
      moving_time = node_sptr(new Times(node_sptr(new Number("-1")), moving_time));
      variable_map_t shifted_vm;
      ValueModifier modifier(*backend_);
      shifted_vm = modifier.shift_time(moving_time, original_vm);
      value_t tmp_time_limit = time_limit;
      tmp_time_limit -= target_time;
      guard_time_map.clear();
      for(auto var : original_vm) HYDLA_LOGGER_DEBUG("#epsilon original : ",var.first," : ",var.second);
      for(auto var : shifted_vm)  HYDLA_LOGGER_DEBUG("#epsilon shifted : ",var.first," : ",var.second);
      //再帰
      tmp_min_time = calculate_tmp_min_time(phase,guard,min_time_calculator,guard_time_map,shifted_vm,tmp_time_limit,entailed);
      //tmp_min_time = find_min_time(guard,min_time_calculator,guard_time_map,shifted_vm,tmp_time_limit,entailed);
      for(auto &tmp_candidate : tmp_min_time){
        tmp_candidate.time += target_time;
        if (opts_->fullsimplify) {
          backend_->call("fullsimplify", false, 1, "vln", "vl", &(tmp_candidate.time), &(tmp_candidate.time));
        } else {
          backend_->call("simplify", false, 1, "vln", "vl", &(tmp_candidate.time), &(tmp_candidate.time));
        }
        ret.push_back(tmp_candidate);
      }
    }
  }
  HYDLA_LOGGER_DEBUG("#epsilon check find min time result");
  for(auto candidate : before){
    HYDLA_LOGGER_DEBUG("#epsilon before : ",candidate.time);
  }
  for(auto candidate : ret){
    HYDLA_LOGGER_DEBUG("#epsilon after  : ",candidate.time);
  }
  return ret;
}

ValueRange PhaseSimulator::create_range_from_interval(itvd itv)
{
  value_t lower(itv.lower()), upper(itv.upper());
  backend_->call("transformToRational", false, 1, "vln", "vl", &lower, &lower);
  backend_->call("transformToRational", false, 1, "vln", "vl", &upper, &upper);
  return ValueRange(lower, upper);
}

find_min_time_result_t PhaseSimulator::find_min_time(const constraint_t &guard, MinTimeCalculator &min_time_calculator, guard_time_map_t &guard_time_map, variable_map_t &original_vm, Value &time_limit, bool entailed, phase_result_sptr_t &phase)
{
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(guard));
  std::list<AtomicConstraint *> guards = relation_graph_->get_atomic_guards(guard);
  bool by_newton = false;
  list<Parameter> parameters;
  constraint_t guard_for_newton;
  for(auto atomic_guard : guards)
  {
    constraint_t g = atomic_guard->constraint;

    if(!guard_time_map.count(g))
    {
      variable_map_t related_vm = get_related_vm(g, original_vm);
      /*
        TODO: implement
        if(opts_->epsilon_mode >= 0)
        {
        min_time_for_this_guard = find_min_time_epsilon(trigger, related_vm,
        time_limit, phase, backend_.get());
        }
      */
      if(opts_->interval && guard_for_newton.get() == nullptr)
      {
        cout << "apply Interval Newton method to " << get_infix_string(g) << "?('y' or 'n')" << endl;
        char c;
        cin >> c;
        bool by_newton = false;
        if(c == 'y')
        {
          guard_for_newton = g;
          by_newton = true;
        }
      }
      if(!by_newton)
      {
        constraint_t constraint_for_this_guard;
        variable_map_t related_vm = get_related_vm(g, original_vm);
        backend_->call("calculateConsistentTime", true, 2, "etmvt", "e", &g, &related_vm,  &constraint_for_this_guard);
        guard_time_map[g] = constraint_for_this_guard;
      }
    }
  }

  find_min_time_result_t min_time_for_this_ask;
  if(guard_for_newton.get() != nullptr)
  {
    variable_map_t related_vm = get_related_vm(guard_for_newton, original_vm);
    // TODO: deal with multiple parameter maps
    vector<parameter_map_t> parameter_map_vector = phase->get_parameter_maps();
    assert(parameter_map_vector.size() <= 1);
    parameter_map_t pm = parameter_map_vector.size()==1?parameter_map_vector.front():parameter_map_t();
    list<itvd> result_interval_list = calculate_interval_newton_nd(guard_for_newton, related_vm, pm);
    const type_info &guard_type = typeid(*guard_for_newton);

    // TODO: integrate process for both equalities and inequalities
    if(guard_type == typeid(Equal) || guard_type == typeid(UnEqual))
    {
      Parameter parameter_prev("t", -1, ++time_id);
      Parameter parameter_current("t", -1, ++time_id);
      itvd prev_interval = itvd(0, 0);
      while(true)
      {
        if(result_interval_list.empty())throw HYDLA_ERROR("there is no valid time for the guard :" + get_infix_string(guard));
        itvd current_interval = result_interval_list.front();
        result_interval_list.pop_front();

        // add temporary parameters for parameter_prev and parameter_current
        parameter_map_t pm_for_each = pm;
        ValueRange prev_range = create_range_from_interval(prev_interval);
        ValueRange current_range = create_range_from_interval(current_interval);
        pm_for_each[parameter_prev] = prev_range;
        pm_for_each[parameter_current] = current_range;

        backend_->call("resetConstraintForParameter", false, 1, "mp", "", &pm_for_each);

        if(guard_type == typeid(UnEqual))
        {
          guard_time_map[guard_for_newton] = constraint_t(new UnEqual(new se::Parameter(parameter_current), new SymbolicT()));
        }
        else
        {
          guard_time_map[guard_for_newton] = constraint_t(new Equal(new se::Parameter(parameter_current), new SymbolicT()));
        }
        constraint_t lb, ub;
        lb.reset(new LessEqual(new se::Parameter(parameter_prev), new SymbolicT()));
        ub.reset(new Less(new SymbolicT(), new se::Parameter(parameter_current)));
        constraint_t time_bound;
        // prev_t < t /\ t < current_t
        time_bound.reset(new LogicalAnd(lb, ub));

        min_time_for_this_ask = min_time_calculator.calculate_min_time(&guard_time_map, guard_for_newton, entailed, time_limit, time_bound);
        // TODO: deal with  branching of cases

        if(!min_time_for_this_ask.empty())
        {
          assert(min_time_for_this_ask.size() == 1);
          simulator_->introduce_parameter(parameter_current, current_range);
          ConstraintStore new_store = phase->get_parameter_constraint();
          new_store.add_constraint_store(current_range.create_range_constraint(node_sptr(new se::Parameter(parameter_current))));
          phase->set_parameter_constraint(new_store);
          backend_->call("resetConstraintForParameter", false, 1, "csn", "", &new_store);
          break;
        }
        prev_interval = current_interval;
      }
    }
    else
    {
      assert(guard_type == typeid(se::LessEqual) ||
             guard_type == typeid(se::Less) ||
             guard_type == typeid(se::Greater) ||
             guard_type == typeid(se::GreaterEqual));

      Parameter parameter_prev("t", -1, ++time_id);
      Parameter parameter_lower("t", -1, ++time_id);
      Parameter parameter_upper("t", -1, ++time_id);
      itvd prev_interval = itvd(0, 0);
      bool include_border = (guard_type == typeid(se::LessEqual) || guard_type == typeid(se::GreaterEqual));

      itvd lower_interval, upper_interval;

      if(entailed)
      {
        lower_interval = itvd(0,0);
      }
      else
      {
        assert(!result_interval_list.empty());
        lower_interval = result_interval_list.front();
        result_interval_list.pop_front();
      }

      while(true)
      {
        assert(!result_interval_list.empty());
        upper_interval = result_interval_list.front();
        result_interval_list.pop_front();

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
          guard_time_map[guard_for_newton] = constraint_t(new LogicalAnd(lb, ub));
        }

        constraint_t time_bound;
        {
          constraint_t lb, ub;
          lb.reset(new LessEqual(new se::Parameter(parameter_prev), new SymbolicT()));
          ub.reset(new Less(new SymbolicT(), new se::Parameter(parameter_upper)));

          // prev_t < t /\ t < current_t
          time_bound.reset(new LogicalAnd(lb, ub));
        }

        min_time_for_this_ask = min_time_calculator.calculate_min_time(&guard_time_map, guard_for_newton, entailed, time_limit, time_bound);
        // TODO: deal with  branching of cases

        if(!min_time_for_this_ask.empty())
        {
          assert(min_time_for_this_ask.size() == 1);
          simulator_->introduce_parameter(parameter_upper, upper_range);
          simulator_->introduce_parameter(parameter_lower, lower_range);
          ConstraintStore new_store = phase->get_parameter_constraint();
          new_store.add_constraint_store(lower_range.create_range_constraint(node_sptr(new se::Parameter(parameter_lower))));
          new_store.add_constraint_store(upper_range.create_range_constraint(node_sptr(new se::Parameter(parameter_upper))));
          phase->set_parameter_constraint(new_store);
          backend_->call("resetConstraintForParameter", false, 1, "csn", "", &new_store);
          break;
        }
        prev_interval = upper_interval;

        // TODO: deal with the last elements
        assert(!result_interval_list.empty());
        lower_interval = result_interval_list.front();
        result_interval_list.pop_front();
      }
    }
  }
  else min_time_for_this_ask = min_time_calculator.calculate_min_time(&guard_time_map, guard, entailed, time_limit);
  return min_time_for_this_ask;
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
    }
    check_break_points(phase, phase->variable_map);
    if(!aborting)
    {
      next_todo->set_parameter_constraint(phase->get_parameter_constraint());
      next_todo->id = ++phase_sum_;
      next_todo->phase_type = INTERVAL_PHASE;
      next_todo->parent = phase.get();
      next_todo->diff_sum = phase->diff_sum;
      next_todo->current_time = phase->end_time = phase->current_time;
      next_todo->discrete_asks = phase->discrete_asks;
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
      timer::Timer find_min_time_timer;
      for(auto ask : asks)
      {
        if(opts_->epsilon_mode >= 0)
        {
          candidate_map[ask] = find_min_time_test(phase,ask->get_guard(), min_time_calculator, guard_time_map, original_vm, time_limit, relation_graph_->get_entailed(ask));
        }else{
          candidate_map[ask] = find_min_time(ask->get_guard(), min_time_calculator, guard_time_map, original_vm, time_limit, relation_graph_->get_entailed(ask), phase);
        }
      }

      for(auto &entry : break_point_list)
      {
        auto break_point = entry.first;
        entry.second = find_min_time(break_point.condition, min_time_calculator, guard_time_map, original_vm, time_limit, false, phase);
      }
      phase->profile["FindMinTime"] += find_min_time_timer.get_elapsed_us();
      HYDLA_LOGGER_DEBUG("");
      pp_time_result_t time_result;
      // 各askに関する最小時刻を比較して最小のものを選ぶ．
      timer::Timer compare_min_time_timer;
      for(auto entry : candidate_map)
      {
        time_result = compare_min_time(time_result, entry.second, entry.first);
      }
      for(auto entry : break_point_list)
      {
        ask_t null_ask;
        time_result = compare_min_time(time_result, entry.second, null_ask);
      }
      phase->profile["CompareMinTime"] += compare_min_time_timer.get_elapsed_us();
      /*
        if(opts_->epsilon_mode >= 0){
        time_result = reduce_unsuitable_case(time_result, backend_.get(), phase);
      }*/


      if(opts_->epsilon_mode >= 0){
        for(auto entry : time_result){
          HYDLA_LOGGER_DEBUG("#epsilon DC before : ", entry.parameter_constraint);
        }
        time_result = reduce_unsuitable_case(time_result, backend_.get(), phase);
        for(auto entry : time_result){
          HYDLA_LOGGER_DEBUG("#epsilon DC after : ", entry.parameter_constraint);
        }
      }

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
          if (opts_->fullsimplify) {
            backend_->call("fullsimplify", false, 1, "vln", "vl", &phase->end_time, &phase->end_time);
          } else {
            backend_->call("simplify", false, 1, "vln", "vl", &phase->end_time, &phase->end_time);
          }
          if(candidate.time.undefined() || candidate.time.infinite() )
          {
            phase->simulation_state = TIME_LIMIT;
            phase->end_time = max_time;
          }
          else
          {
            next_todo->id = ++phase_sum_;
            next_todo->discrete_asks = candidate.discrete_asks;
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
            phase->simulation_state = SIMULATED;
            phase->todo_list.push_back(next_todo);
          }

          if(++time_it == time_result.end())break;
          //prepare new PhaseResult
          phase.reset(new PhaseResult(*phase));
          phase->id = ++phase_sum_;
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

list<itvd> PhaseSimulator::calculate_interval_newton_nd(const constraint_t& guard, const variable_map_t &related_vm, parameter_map_t &pm)
{
  node_sptr exp;
  node_sptr dexp;
  backend_->call("relationToFunction", true, 2, "etmvt", "e", &guard, &related_vm, &exp);
  backend_->call("differentiateWithTime", true, 1, "et", "e", &exp, &dexp);
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(exp));
  HYDLA_LOGGER_DEBUG_VAR(get_infix_string(dexp));
  itvd init = itvd(0.,100);
  list<itvd> result_intervals =
    interval::calculate_interval_newton_nd(init, exp, dexp, pm);
  return result_intervals;
}

}
}
