#include <iostream>

#include "PhaseSimulator.h"

#include "Logger.h"
#include "Timer.h"

#include "PrevReplacer.h"
#include "AskCollector.h"
#include "VariableFinder.h"
#include "ContinuityMapMaker.h"
#include "PrevSearcher.h"
#include "Exceptions.h"
#include "AlwaysFinder.h"
#include "EpsilonMode.h"
#include "TimeModifier.h"

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

PhaseSimulator::PhaseSimulator(Simulator* simulator,const Opts& opts): breaking(false), simulator_(simulator), opts_(&opts), select_phase_(NULL), break_condition_(symbolic_expression::node_sptr()) {
}

PhaseSimulator::~PhaseSimulator(){}

PhaseSimulator::result_list_t PhaseSimulator::calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont)
{
  HYDLA_LOGGER_DEBUG("%% current time:", todo->current_time);
  HYDLA_LOGGER_DEBUG("%% prev map:", todo->prev_map);
  timer::Timer phase_timer;
  result_list_t result;

  module_set_container->reset(todo->ms_to_visit);

  if(todo_cont == NULL)
  {
    todo_container_t tmp_cont;
    todo_container_ = &tmp_cont;
    simulation_todo_sptr_t tmp_todo = todo;
    while(true)
    {
      result_list_t tmp_result = make_results_from_todo(tmp_todo);
      result.insert(result.begin(), tmp_result.begin(), tmp_result.end());
      if(tmp_cont.empty())break;
      tmp_todo = tmp_cont.pop_todo();
      module_set_container->reset(tmp_todo->ms_to_visit);
    }
  }
  else
  {
    todo_container_ = todo_cont;
    result = make_results_from_todo(todo);
  }

  todo->profile["PhaseResult"] += phase_timer.get_elapsed_us();
  return result;
}

PhaseSimulator::result_list_t PhaseSimulator::make_results_from_todo(simulation_todo_sptr_t& todo)
{
  result_list_t result;
  bool has_next = false;

  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &todo->parameter_map);
  backend_->call("addPrevConstraint", 1, "mvp", "", &todo->prev_map);
  relation_graph_->set_expanded_all(false);
  if(todo->phase_type == PointPhase)
  {
    set_simulation_mode(PointPhase);
    if(todo->parent != result_root)
    {
      // if it's not in first phases, judge entailment of the prev guards in advance
      for(auto prev_guard : prev_asks_)
      {
        CheckConsistencyResult cc_result;
        // TODO: 離散変化時刻の計算時の情報を生かせれば、そもそもこの判定自体が不要なはず。
        // 前のフェーズで成り立っていたかと、不等式を含むかどうかとかで判定できる。
        // もしくは離散変化時刻の計算時に、PPを含むかどうかまで計算しておく。     
        switch(consistency_checker->check_entailment(*relation_graph_, cc_result, prev_guard, todo->phase_type)){
        case ENTAILED:
          todo->judged_prev_map.insert(std::make_pair(prev_guard, true));
          break;
        case CONFLICTING:
          todo->judged_prev_map.insert(std::make_pair(prev_guard, false));
          break;
        default:
          assert(0); // entailablities of prev guards must be determined
          break;
        }
      }
    }
  }else{
    set_simulation_mode(IntervalPhase);
  }
  if(todo->parent == result_root)
  {
    // in the initial state, set all modules expanded
    for(auto module : module_set_container->get_max_module_set())
    {
      todo->expanded_constraints.add_constraint(module.second);
    }
  }


  while(module_set_container->has_next())
  {
    module_set_t ms = module_set_container->get_module_set();
    
    relation_graph_->set_expanded_all(false);
    for(auto constraint : todo->expanded_constraints)
    {
      relation_graph_->set_expanded(constraint, true);
    }

    for(auto entry : todo->judged_prev_map)
    {
      if(entry.second)
      {
        todo->positive_asks.insert(entry.first);
        relation_graph_->set_expanded(entry.first->get_child(), true);
      }
      else
      {
        todo->negative_asks.insert(entry.first);
      }
    }


    std::string module_sim_string = "\"ModuleSet" + ms.get_name() + "\"";
    timer::Timer ms_timer;
    result_list_t tmp_result = simulate_ms(ms, todo);
    if(!tmp_result.empty())
    {
      has_next = true;
      result.insert(result.begin(), tmp_result.begin(), tmp_result.end());
    }
    todo->profile[module_sim_string] += ms_timer.get_elapsed_us();
    todo->positive_asks.clear();
    todo->negative_asks.clear();
    relation_graph_->set_expanded_all(false);
  }

  //無矛盾な解候補モジュール集合が存在しない場合
  if(!has_next)
  {
    // make dummy phase and push into tree.
    phase_result_sptr_t phase(new PhaseResult(*todo, simulator::INCONSISTENCY));
    todo->parent->children.push_back(phase);
  }
  return result;
}



PhaseSimulator::result_list_t PhaseSimulator::simulate_ms(const module_set_t& ms, simulation_todo_sptr_t& todo)
{
  HYDLA_LOGGER_DEBUG("--- next module set ---\n", ms.get_infix_string());
  relation_graph_->set_adopted(ms);
  result_list_t result;
  // TODO:変数の値による分岐も無視している？

  backend_->call("resetConstraintForVariable", 0, "", "");

  ConstraintStore store =
    calculate_constraint_store(ms, todo);
  if(!store.consistent())
  {
    HYDLA_LOGGER_DEBUG("INCONSISTENT");
    module_set_container->generate_new_ms(todo->maximal_mss, consistency_checker->get_inconsistent_module_set());
    return result;
  }
  HYDLA_LOGGER_DEBUG("CONSISTENT");

  module_set_container->remove_included_ms_by_current_ms();
  if(!(opts_->nd_mode || opts_->interactive_mode)) module_set_container->reset(module_set_set_t());
  todo->maximal_mss.insert(ms);

  phase_result_sptr_t phase = make_new_phase(todo, store);
  phase->module_set = ms;


  // 変数表はここで作成する
  vector<variable_map_t> create_result;
  if(phase->phase_type == PointPhase)
  {
    backend_->call("createVariableMap", 1, "csn", "cv", &store, &create_result);
  }
  else
  {
    backend_->call("createVariableMapInterval", 1, "cst", "cv", &store, &create_result);
  }
  if(create_result.size() != 1)
  {
    throw SimulateError("result variable map is not single.");
  }
  phase->variable_map = create_result[0];

  if(opts_->reuse && todo->parent != result_root){
    set_changed_variables(phase);
    if(phase->phase_type == IntervalPhase && phase->parent.get() && phase->parent->parent.get())
    {
      for(auto var_entry : phase->variable_map)
      {
        bool changed = false;
        for(auto var_name : phase->changed_variables)
        {
          if(var_entry.first.get_name() == var_name)
          {
            changed = true;
          }
        }
        if(!changed)
        {
          phase->variable_map[var_entry.first] =
            phase->parent->parent->variable_map[var_entry.first];            
        }
      }
    }
  }

  for(auto positive_ask : todo->positive_asks)
  {
    phase->expanded_constraints.add_constraint(positive_ask->get_child());
  }
  todo->current_constraints = relation_graph_->get_constraints();


  if(opts_->assertion || break_condition_.get() != NULL){
    timer::Timer entailment_timer;

    backend_->call("resetConstraintForVariable", 0, "","");
    std::string fmt = "mv0";
    fmt += (phase->phase_type==PointPhase)?"n":"t";
    backend_->call("addConstraint", 1, fmt.c_str(), "", &phase->variable_map);
    backend_->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);

/*  TODO: implement
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
    */
  }

  if(opts_->epsilon_mode){
    phase->variable_map = cut_high_order_epsilon(backend_.get(),phase);
  }

  result.push_back(phase);

  return result;
}


void PhaseSimulator::push_branch_states(simulation_todo_sptr_t &original, CheckConsistencyResult &result){
  simulation_todo_sptr_t branch_state_false(create_new_simulation_phase(original));
  branch_state_false->initial_constraint_store.add_constraint_store(result.inconsistent_store);
  todo_container_->push_todo(branch_state_false);
  original->initial_constraint_store.add_constraint_store(result.consistent_store);
  //backend_->call("resetConstraint", 1, "csn", "", &original->initial_constraint_store);
  // TODO: implement branch here
  throw SimulateError("branch in calculate closure.");
}


phase_result_sptr_t PhaseSimulator::make_new_phase(simulation_todo_sptr_t& todo, const ConstraintStore& store)
{
  phase_result_sptr_t phase(new PhaseResult(*todo));
  phase->id = ++phase_sum_;
  phase->constraint_store = store;
  todo->parent->children.push_back(phase);
  return phase;
}


phase_result_sptr_t PhaseSimulator::make_new_phase(const phase_result_sptr_t& original)
{
  phase_result_sptr_t phase(new PhaseResult(*original));
  phase->id = ++phase_sum_;
  phase->parent->children.push_back(phase);
  phase->cause_for_termination = simulator::NONE;
  return phase;
}

void PhaseSimulator::initialize(variable_set_t &v,
                                parameter_map_t &p,
                                variable_map_t &m,
                                continuity_map_t& c,
                                module_set_container_sptr &msc)
{
  variable_set_ = &v;
  parameter_map_ = &p;
  variable_map_ = &m;
  phase_sum_ = 0;
  module_set_container = msc;

  simulator::module_set_t ms = module_set_container->get_max_module_set();

  relation_graph_.reset(new RelationGraph(ms)); 

  if(opts_->dump_relation){
    relation_graph_->dump_graph(std::cout);
    exit(EXIT_SUCCESS);
  }


  AskCollector ac;

  ConstraintStore constraints;
  for(auto module : ms)
  {
    constraints.add_constraint(module.second);
  }
  positive_asks_t pat;
  negative_asks_t nat;

  // search asks and initialize prev_asks_ and ask_map
  ac.collect_ask(constraints, &pat, &nat, &prev_asks_);
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

  
  backend_->set_variable_set(*variable_set_);
  variable_derivative_map_ = c;
}


simulation_todo_sptr_t PhaseSimulator::create_new_simulation_phase(const simulation_todo_sptr_t& old) const
{
  simulation_todo_sptr_t sim(new SimulationTodo(*old));
  return sim;
}


void PhaseSimulator::substitute_parameter_condition(phase_result_sptr_t pr, parameter_map_t pm)
{
  HYDLA_LOGGER_DEBUG("");
	// 変数に代入
	variable_map_t ret;
  variable_map_t &vm = pr->variable_map;
  for(variable_map_t::iterator it = vm.begin();
      it != vm.end(); it++)
  {
    if(it->second.undefined())continue;
    assert(it->second.unique());
    value_t tmp_val = it->second.get_unique_value();
    backend_->call("substituteParameterCondition",
                   2, "vlnmp", "vl", &tmp_val, &pm, &tmp_val);
    it->second.set_unique_value(tmp_val);
  }

	// 時刻にも代入
  HYDLA_LOGGER_DEBUG("");
  backend_->call("substituteParameterCondition",
                 2, "vlnmp", "vl", &pr->current_time, &pm, &pr->current_time);
	if(pr->phase_type == IntervalPhase){
    backend_->call("substituteParameterCondition",
                   2, "vlnmp", "vl", &pr->end_time, &pm, &pr->end_time);
	}
}

void PhaseSimulator::replace_prev2parameter(
                                            phase_result_sptr_t &phase,
                                            variable_map_t &vm,
                                            parameter_map_t &parameter_map)
{
  assert(phase->parent.get() != NULL);

  PrevReplacer replacer(parameter_map, phase, *simulator_, opts_->approx);
  for(variable_map_t::iterator it = vm.begin();
      it != vm.end(); it++)
  {
    HYDLA_LOGGER_DEBUG(it->first, it->second);
    ValueRange& range = it->second;
    value_t val;
    if(range.unique())
    {
      val = range.get_unique_value();
      HYDLA_LOGGER_DEBUG(val);
      replacer.replace_value(val);
      range.set_unique_value(val);
    }
    else
    {
      if(range.get_upper_cnt()>0)
      {
        val = range.get_upper_bound().value;
        replacer.replace_value(val);
        range.set_upper_bound(val, range.get_upper_bound().include_bound);
      }
      if(range.get_lower_cnt() > 0)
      {
        val = range.get_lower_bound().value;
        replacer.replace_value(val);
        range.set_lower_bound(val, range.get_lower_bound().include_bound);
      }
    }
  }
}

void PhaseSimulator::set_break_condition(symbolic_expression::node_sptr break_cond)
{
  break_condition_ = break_cond;
}

PhaseSimulator::node_sptr PhaseSimulator::get_break_condition()
{
  return break_condition_;
}

string timein(string message="")
{
  string ret;
  while(1)
  {
    if(!message.empty()) std::cout << message << std::endl;
    std::cout << '>' ;
    std::cin >> ret;
    if(!std::cin.fail()) break;
    std::cin.clear();
    std::cin.ignore( 1024, '\n' );
  }
  std::cin.clear();
  std::cin.ignore( 1024, '\n' );
  return ret;
}

void PhaseSimulator::init_arc(const parse_tree_sptr& parse_tree){
/* TODO: 一時無効
  analysis_result_checker_ = boost::shared_ptr<AnalysisResultChecker >(new AnalysisResultChecker(*opts_));
  analysis_result_checker_->initialize(parse_tree);
  analysis_result_checker_->set_solver(solver_);
  analysis_result_checker_->check_all_module_set((opts_->analysis_mode == "cmmap" ? true : false));
*/
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


bool PhaseSimulator::calculate_closure(simulation_todo_sptr_t& state,
    const module_set_t& ms)
{
  // preparation
  positive_asks_t& positive_asks = state->positive_asks;
  negative_asks_t& negative_asks = state->negative_asks;

  ask_set_t unknown_asks;

  AskCollector  ask_collector;

  continuity_map_t continuity_map;

  bool expanded;

  if(opts_->reuse && state->in_following_step() ){
    if(state->phase_type == PointPhase){
      ask_collector.collect_ask(state->expanded_constraints,
          &positive_asks,
          &negative_asks,
          &unknown_asks);
      apply_discrete_causes_to_guard_judgement( state->parent, state->discrete_causes, positive_asks, negative_asks, unknown_asks );
      set_changing_variables( state->parent, ms, positive_asks, negative_asks, state->changing_variables );
    }
    else{
      state->changing_variables = state->parent->changed_variables;
    }
  }

  ask_set_t original_p_asks = positive_asks;
  ask_set_t original_n_asks = negative_asks;
  ask_set_t original_u_asks = unknown_asks;
  bool entailment_changed = false;

  do{
    if(entailment_changed){
      positive_asks = original_p_asks;
      negative_asks = original_n_asks;
      unknown_asks = original_u_asks;
      entailment_changed = false;
    }

    timer::Timer consistency_timer;

/*
    for(auto tell : tell_list){
      if(opts_->reuse && state->in_following_step())
      {
        VariableFinder variable_finder;
        variable_finder.visit_node(tell);
        if(!variable_finder.include_variables(state->changing_variables)
           && (current_phase_ != IntervalPhase || !variable_finder.include_variables_prev(state->changing_variables)))
        {
          continue;
        }
        // TODO: 何やってるか確認
      }
    }
*/
    {
      CheckConsistencyResult cc_result;
      cc_result = consistency_checker->check_consistency(*relation_graph_, state->phase_type);
      if(!cc_result.consistent_store.consistent()){
        HYDLA_LOGGER_DEBUG("%% inconsistent for all cases");
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return false;
      }else if (!cc_result.inconsistent_store.consistent()){
        HYDLA_LOGGER_DEBUG("%% consistent for all cases");
      }else{
        HYDLA_LOGGER_DEBUG("%% consistency depends on conditions of parameters\n");
        push_branch_states(state, cc_result);
      }
    }

    state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();

    if(opts_->reuse && state->in_following_step()){
      apply_previous_solution(state->changing_variables,
          state->phase_type == IntervalPhase, state->parent,
          continuity_map, state->current_time );
    }

    ask_collector.collect_ask(state->expanded_constraints,
        &positive_asks,
        &negative_asks,
        &unknown_asks);

    timer::Timer entailment_timer;

    {
      expanded = false;
      ask_set_t cv_unknown_asks, notcv_unknown_asks;
      if(opts_->reuse && state->in_following_step()){
        for( auto ask : unknown_asks ){
          if(has_variables(ask, state->changing_variables, state->phase_type == IntervalPhase)){
            cv_unknown_asks.insert(ask);
          }else{
            notcv_unknown_asks.insert(ask);
          }
        }
        unknown_asks = cv_unknown_asks;
      }
      ask_set_t::iterator it  = unknown_asks.begin();
      ask_set_t::iterator end = unknown_asks.end();
      while(it!=end)
      {
        if(state->phase_type == PointPhase){
          if(state->parent == result_root && PrevSearcher().search_prev((*it)->get_guard())){
            // in initial state, conditions about left-hand limits are considered to be invalid
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            continue;
          }
        }else if(opts_->reuse && state->in_following_step()
            && state->parent->negative_asks.find(*it) != state->parent->negative_asks.end()
            && state->discrete_causes.find(*it) == state->discrete_causes.end()){
          VariableFinder v_finder;
          v_finder.visit_node((*it)->get_guard(), true);
          auto variables = v_finder.get_all_variable_set();
          bool continuity = true;
          for(auto var : variables){
            //すべてのdiscrete_causesの後件に変数値が含まれていないことを調べないといけない
            //以下の処理は黒ジャンプには対応しているが、白ジャンプには対応していない 
            auto var_d = state->parent->variable_map.find(Variable(var.get_name(),var.get_differential_count()+1));
            if(var_d->second.undefined()){
              continuity = false;
              break;
            }
          }
          if(continuity){
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            continue;
          }
        }

        CheckConsistencyResult check_consistency_result;
        switch(consistency_checker->check_entailment(*relation_graph_, check_consistency_result, *it, state->phase_type)){
          case ENTAILED:
            HYDLA_LOGGER_DEBUG("--- entailed ask ---\n", *((*it)->get_guard()));
            if(opts_->reuse && state->in_following_step()){
              if(state->phase_type == PointPhase){
                apply_entailment_change(it, state->parent->negative_asks,
                    false, state->changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }else{
                apply_entailment_change(it, state->parent->parent->negative_asks,
                    true, state->changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }
            }
            positive_asks.insert(*it);
            relation_graph_->set_expanded((*it)->get_child(), true);
            unknown_asks.erase(it++);
            expanded = true;
            break;
          case CONFLICTING:
            HYDLA_LOGGER_DEBUG("--- conflicted ask ---\n", *((*it)->get_guard()));
            if(opts_->reuse && state->in_following_step() ){
              if(state->phase_type == PointPhase){
                entailment_changed = apply_entailment_change(it, state->parent->positive_asks,
                    false, state->changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }else{
                entailment_changed = apply_entailment_change(it, state->parent->parent->positive_asks,
                    true, state->changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }
              if(entailment_changed) break;
            }
            negative_asks.insert(*it);
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
        if(entailment_changed) break;
      }
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    if(entailment_changed) continue;
  }while(expanded);

  if(!unknown_asks.empty()){
    boost::shared_ptr<symbolic_expression::Ask> branched_ask = *unknown_asks.begin();
    // TODO: 極大性に対して厳密なものになっていない（実行アルゴリズムを実装しきれてない）
    HYDLA_LOGGER_DEBUG("%% branched_ask:", get_infix_string(branched_ask));
    {
      // 分岐先を生成（導出される方）
      // TODO: 分岐時の制約の追加をしていない。
      simulation_todo_sptr_t new_todo(create_new_simulation_phase(state));
      todo_container_->push_todo(new_todo);
    }
    {
      // 分岐先を生成（導出されない方）
      // TODO: 分岐時の制約の追加をしていない。
      negative_asks.insert(branched_ask);
      return calculate_closure(state, ms);
    }
  }
  return true;
}

ConstraintStore
PhaseSimulator::calculate_constraint_store(
  const module_set_t& ms,
  simulation_todo_sptr_t& todo)
{
  timer::Timer cc_timer;
  ConstraintStore result_store;
  bool result = calculate_closure(todo, ms);
  todo->profile["CalculateClosure"] += cc_timer.get_elapsed_us();

  if(!result)
  {
    result_store.set_consistency(false);
    return result_store;
  }

  //TODO: ここで追加するより、全体を簡約したものをつなげて持っておいた方が良さそう
  
  ConstraintStore tmp_constraint_store;
  for(int i = 0; i < relation_graph_->get_connected_count(); i++)
  {
    for(auto constraint : relation_graph_->get_constraints(i))
    {
      tmp_constraint_store.add_constraint(constraint);
    }
  }
  
  backend_->call("resetConstraintForVariable", 0, "", "");
  ContinuityMapMaker maker;
  for(auto constraint : tmp_constraint_store){
    maker.visit_node(constraint, todo->phase_type == IntervalPhase, false);
  }
  consistency_checker->add_continuity(maker.get_continuity_map(), todo->phase_type);


  if(todo->phase_type == PointPhase)
  {
    backend_->call("addConstraint", 1, "csn", "", &tmp_constraint_store);
    backend_->call("getConstraintStorePoint", 0, "", "cs", &result_store);
  }
  else
  {
    backend_->call("addConstraint", 1, "cst", "", &tmp_constraint_store);
    backend_->call("getConstraintStoreInterval", 0, "", "cs", &result_store);
    replace_prev2parameter(todo->parent, result_store, todo->parameter_map);
  }

  return result_store;
}

void PhaseSimulator::apply_discrete_causes_to_guard_judgement(
    const phase_result_sptr_t& parent,
    const ask_set_t& discrete_causes,
    positive_asks_t& positive_asks,
    negative_asks_t& negative_asks,
    ask_set_t& unknown_asks ){

  PrevSearcher searcher;
  ask_set_t prev_asks,reduced_u_asks;

  for( auto ask : unknown_asks ){
    if( searcher.search_prev(ask) ){
      prev_asks.insert(ask);
    }
    else{
      reduced_u_asks.insert(ask);
    }
  }

  unknown_asks = reduced_u_asks;

  for( auto prev_ask : prev_asks ){
    if( discrete_causes.find(prev_ask) != discrete_causes.end() ){
      if ( parent->positive_asks.find(prev_ask) != parent->positive_asks.end() )
      {
        negative_asks.insert( prev_ask );
      }else{
        positive_asks.insert( prev_ask );
      }
    }else{
      if ( parent->positive_asks.find(prev_ask) != parent->positive_asks.end() )
      {
        positive_asks.insert( prev_ask );
      }else{
        negative_asks.insert( prev_ask );
      }
    }
  }

}

void PhaseSimulator::set_changing_variables(
    const phase_result_sptr_t& parent_phase,
    const module_set_t& present_ms,
    const positive_asks_t& positive_asks,
    const negative_asks_t& negative_asks,
    change_variables_t& changing_variables ){
  //条件なし制約の差分取得
  /* Todo: implement
  
  module_set_t parent_ms = parent_phase->module_set;
  TellCollector parent_t_collector(parent_ms);
  tells_t parent_tells;
  //条件なし制約だけ集める
  always_set_t empty_ea;
  positive_asks_t empty_asks;
  parent_t_collector.collect_all_tells(&parent_tells, &empty_ea, &empty_asks );

  TellCollector t_collector(present_ms);
  tells_t tells;
  t_collector.collect_all_tells(&tells, &empty_ea, &empty_asks );

  changing_variables = get_difference_variables_from_2tells( parent_tells, tells );

  //導出状態の差分取得
  //現在はpositiveだけど、parentではpositiveじゃないやつ
  //現在はnegativeだけど、parentではpositiveなやつ
  VariableFinder v_finder;
  positive_asks_t parent_positives = parent_phase->positive_asks;
  int cv_count = changing_variables.size();
  for( auto ask : positive_asks ){
    if(parent_positives.find(ask) == parent_positives.end() ){
      v_finder.visit_node(ask, false);
      VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
      for( auto var : tmp_vars ) changing_variables.insert(var.get_name());
    }
  }

  for( auto ask : negative_asks ){
    if(parent_positives.find(ask) != parent_positives.end() ){
      v_finder.visit_node(ask);
      VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
      for( auto var : tmp_vars ) changing_variables.insert(var.get_name());
    }
  }

  if(changing_variables.size() > cv_count){
    cv_count = changing_variables.size();
    while(true){
      for( auto tell : tells ){
        bool has_cv = has_variables(tell, changing_variables, false);
        if(has_cv){
          v_finder.clear();
          v_finder.visit_node(tell);
          VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
          for( auto var : tmp_vars )
            changing_variables.insert(var.get_name());
        }
      }
      if(changing_variables.size() > cv_count ){
        cv_count = changing_variables.size();
        continue;
      }
      break;
    }
  }
  */
}

void PhaseSimulator::set_changed_variables(phase_result_sptr_t& phase)
{
/* TODO: implement
  if(phase->parent.get() == NULL)return;
  TellCollector current_tell_collector(phase->module_set);
  tells_t current_tell_list;
  always_set_t& current_expanded_always = phase->expanded_always;
  positive_asks_t& current_positive_asks = phase->positive_asks;
  current_tell_collector.collect_all_tells(&current_tell_list,&current_expanded_always,&current_positive_asks);

  TellCollector prev_tell_collector(phase->parent->module_set);
  tells_t prev_tell_list;
  always_set_t& prev_expanded_always = phase->parent->expanded_always;
  positive_asks_t& prev_positive_asks = phase->parent->positive_asks;
  prev_tell_collector.collect_all_tells(&prev_tell_list,&prev_expanded_always,&prev_positive_asks);
  phase->changed_variables = get_difference_variables_from_2tells(current_tell_list, prev_tell_list);
*/
}



change_variables_t PhaseSimulator::get_difference_variables_from_2tells(const tells_t& larg, const tells_t& rarg){
  change_variables_t cv;
  tells_t l_tells = larg;
  tells_t r_tells = rarg;

  tells_t symm_diff_tells, intersection_tells;
  for( auto tell : l_tells ){
    tells_t::iterator it = std::find( r_tells.begin(), r_tells.end(), tell );
    if( it == r_tells.end() )
      symm_diff_tells.push_back(tell);
    else{
      intersection_tells.push_back(tell);
      r_tells.erase(it);
    }
  }
  for( auto tell : r_tells ) symm_diff_tells.push_back(tell);

  VariableFinder v_finder;
  for( auto tell : symm_diff_tells )
    v_finder.visit_node(tell);

  VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
  for( auto var : tmp_vars )
    cv.insert(var.get_name());

  int v_count = cv.size();
  while(true){
    for( auto tell : intersection_tells ){
      bool has_cv = has_variables(tell, cv, false);
      if(has_cv){
        v_finder.clear();
        v_finder.visit_node(tell);
        tmp_vars = v_finder.get_variable_set();
        for( auto var : tmp_vars )
          cv.insert(var.get_name());
      }
    }
    if(cv.size() > v_count ){
      v_count = cv.size();
      continue;
    }
    break;
  }

  return cv;
}

bool PhaseSimulator::has_variables(symbolic_expression::node_sptr node, const change_variables_t & change_variables, bool include_prev)
{
  VariableFinder variable_finder;
  variable_finder.visit_node(node);
  if(variable_finder.include_variables(change_variables) ||
     (include_prev && variable_finder.include_variables_prev(change_variables)))
  {
    return true;
  }
  return false;
}

bool PhaseSimulator::apply_entailment_change(
    const ask_set_t::iterator it,
    const ask_set_t& previous_asks,
    const bool in_IP,
    change_variables_t& changing_variables,
    ask_set_t& notcv_unknown_asks,
    ask_set_t& unknown_asks ){
  bool ret = false;
  if(previous_asks.find(*it) != previous_asks.end() ){
    VariableFinder v_finder;
    v_finder.visit_node(*it);
    VariableFinder::variable_set_t tmp_vars = in_IP?v_finder.get_all_variable_set():v_finder.get_variable_set();
    int v_count = changing_variables.size();
    for(auto var : tmp_vars){
      changing_variables.insert(var.get_name());
    }
    if(changing_variables.size() > v_count){
      ask_set_t change_asks;
      for(auto ask : notcv_unknown_asks){
        if(has_variables(ask->get_child(), changing_variables, in_IP) ){
          unknown_asks.insert(ask);
          change_asks.insert(ask);
        }
      }
      if( !change_asks.empty() ){
        ret = true;
        for(auto ask : change_asks){
          notcv_unknown_asks.erase(ask);
        }
      }
    }
  }
  return ret;
}

void PhaseSimulator::apply_previous_solution(
    const change_variables_t& variables,
    const bool in_IP,
    const phase_result_sptr_t parent,
    continuity_map_t& continuity_map,
    const value_t& current_time ){
  for(auto pair : parent->variable_map){
    std::string var_name = pair.first.get_name();
    if(variables.find(var_name) == variables.end() ){
      if(continuity_map.find(var_name) == continuity_map.end() )
        continuity_map.insert( make_pair(var_name, pair.first.differential_count) );
      else if(continuity_map[var_name] < pair.first.differential_count){
        continuity_map.erase(var_name);
        continuity_map.insert( make_pair(var_name, pair.first.differential_count) );
      }
      std::string fmt = "v";
      if(in_IP){
        // 前IPの解を追加
        // TODO:undefである場合の対応
        // TODO:とりあえずunique_valueのみ対応
        fmt += "t";
        fmt += "vlt";
        value_t val = parent->parent->variable_map.find(pair.first)->second.get_unique_value();
        value_t ret;
        backend_->call("exprTimeShiftInverse", 2, "vltvlt", "vl", &val, &current_time, &ret);
        backend_->call("addEquation", 2, fmt.c_str(), "", &pair.first, &ret);
      }else{
        // x=x-
        fmt += "n";
        fmt += "vp";
        backend_->call("addInitEquation", 2, fmt.c_str(), "", &pair.first, &pair.first);
      }
    }
  }
  consistency_checker->add_continuity(continuity_map, in_IP ? IntervalPhase : PointPhase );
}

PhaseSimulator::todo_list_t
  PhaseSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo)
{
  todo_list_t ret;

  simulation_todo_sptr_t next_todo(new SimulationTodo());
  next_todo->parent = phase;
  next_todo->ms_to_visit = module_set_container->get_full_ms_list();
  
  AlwaysFinder always_finder;
  // TODO: positive_asksの後件がalwaysだった場合に、expanded_alwaysから消しても良いはず
  for(auto constraint : phase->expanded_constraints)
  {
    always_finder.find_always(constraint, next_todo->expanded_constraints);
  }
  next_todo->parameter_map = phase->parameter_map;


  if(current_phase_ == PointPhase)
  {
    next_todo->phase_type = IntervalPhase;
    next_todo->current_time = phase->current_time;
    // TODO: 離散変化した変数が関わるガード条件はここから取り除く必要が有りそう（単純なコピーではだめ）
    next_todo->discrete_causes = current_todo->discrete_causes;
    next_todo->prev_map = phase->variable_map;
    ret.push_back(next_todo);
  }
  else
  {
    backend_->call("resetConstraint", 0, "", "");
    backend_->call("addConstraint", 1, "cst", "", &phase->constraint_store);
    backend_->call("addParameterConstraint", 1, "mp", "", &phase->parameter_map);
    
    PhaseSimulator::replace_prev2parameter(phase->parent, phase->variable_map, phase->parameter_map);
    variable_map_t vm_before_time_shift = phase->variable_map;
    phase->variable_map = shift_time_of_vm(phase->variable_map, phase->current_time);
    next_todo->phase_type = PointPhase;

    timer::Timer next_pp_timer;
    dc_causes_t dc_causes;

    //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
    for(auto ask : phase->positive_asks){
      symbolic_expression::node_sptr negated_node(new Not(ask->get_guard()));
      dc_causes.push_back(dc_cause_t(negated_node, ask->get_id() ) );
    }
    //現在導出されていないガード条件を離散変化条件として追加
    for(auto ask : phase->negative_asks){
      symbolic_expression::node_sptr node(ask->get_guard() );
      dc_causes.push_back(dc_cause_t(node, ask->get_id() ));
    }

    //assertionの否定を追加
    if(opts_->assertion){
      symbolic_expression::node_sptr assert_node(new Not(opts_->assertion));
      dc_causes.push_back(dc_cause_t(assert_node, -2));
    }
    if(break_condition_.get() != NULL)
    {
      dc_causes.push_back(dc_cause_t(break_condition_, -3));
    }

    value_t max_time;
    if(opts_->max_time != ""){
      max_time = symbolic_expression::node_sptr(new symbolic_expression::Number(opts_->max_time));
    }else{
      max_time = symbolic_expression::node_sptr(new symbolic_expression::Infinity());
    }

    pp_time_result_t time_result;
    value_t time_limit(max_time);
    time_limit -= phase->current_time;
    if(opts_->cheby)
    {
      try
      {
        backend_->call("calculateNextPointPhaseTime", 2, "vltdc", "cp", &(time_limit), &dc_causes, &time_result);
      }
      catch(const std::runtime_error &se)
      {
        std::cout << "Error occurs in calculateNextPointPhaseTime." << endl;
        std::cout << "Do you want to change next time ? (y or n)" << endl;
        std::string line;
        std::cin.clear();
        getline(std::cin, line);
        std::cin.clear();
        if(line[0] == 'y')
        {
          std::cout << "Parameter ? (y or n)" << endl;
          std::string pa;
          std::cin.clear();
          getline(std::cin, pa);
          std::cin.clear();
          if(pa[0] == 'y')
          {
            ValueRange time_range;
            std::string low;
            std::string up;

            variable_t time("time", 0);

            std::cout << "Input Lower time." << endl;
            low = timein();

            std::cout << "Input Upper time." << endl;
            up = timein();

            value_t lower_time(low);
            value_t upper_time(up);

            time_range.set_lower_bound(lower_time, true);
            time_range.set_upper_bound(upper_time, true);

            parameter_t par = simulator_->introduce_parameter(time, phase, time_range);

            next_todo->current_time = symbolic_expression::node_sptr(new symbolic_expression::Parameter("time", 0, phase->id));
            phase->end_time = symbolic_expression::node_sptr(new symbolic_expression::Parameter("time", 0, phase->id));

            next_todo->parameter_map[par] = time_range;
            phase->parameter_map[par] = time_range;

            ret.push_back(next_todo);

            return ret;
          }
          else
          {
            std::cout << "Input Next PP Time." << endl;
            std::string p_time;

            p_time = timein();

            next_todo->current_time = p_time;

            ret.push_back(next_todo);

            return ret;
          }
        }
        else
        {
          backend_->call("calculateNextPointPhaseTime", 2, "vltdc", "cp", &(time_limit), &dc_causes, &time_result);
        }
      }
    }
    else
    {
      backend_->call("calculateNextPointPhaseTime", 2, "vltdc", "cp", &(time_limit), &dc_causes, &time_result);
    }

    if(opts_->epsilon_mode){
      time_result = reduce_unsuitable_case(time_result,backend_.get(),phase);
    }

    unsigned int time_it = 0;
    result_list_t results;
    phase_result_sptr_t pr = next_todo->parent;


    // まずインタラクティブ実行のために最小限の情報だけ整理する
    while(true)
    {
      DCCandidate &candidate = time_result[time_it];
      // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
      for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
        pr->parameter_map[it->first] = it->second;
      }

      pr->end_time = current_todo->current_time + candidate.minimum.time;
      backend_->call("simplify", 1, "vln", "vl", &pr->end_time, &pr->end_time);
      results.push_back(pr);
      if(++time_it >= time_result.size())break;
      pr = make_new_phase(pr);
    }

    unsigned int result_it = 0;
    bool one_phase = false;

    // 場合の選択を行う場合はここで
    if(time_result.size() > 0 && select_phase_)
    {
      result_it = select_phase_(results);
      one_phase = true;
    }


    // todoを実際に作成する
    while(true)
    {
      pr = results[result_it];

      DCCandidate &candidate = time_result[result_it];
      HYDLA_LOGGER_DEBUG_VAR(result_it);
      for(uint id_it = 0; id_it < candidate.minimum.ids.size(); id_it++)
      {
        int id = candidate.minimum.ids[id_it];
        if(id == -1) {
          pr->cause_for_termination = simulator::TIME_LIMIT;
        }
        else if(id >= 0)
        {
          HYDLA_LOGGER_DEBUG_VAR(id);
          HYDLA_LOGGER_DEBUG_VAR(candidate.minimum.time);
          HYDLA_LOGGER_DEBUG_VAR(*ask_map[id]);
          next_todo->discrete_causes.insert(ask_map[id]);
        }
      }


      if(pr->cause_for_termination != TIME_LIMIT)
      {
        next_todo->current_time = pr->end_time;
        next_todo->parameter_map = pr->parameter_map;
        next_todo->parent = pr;
        next_todo->prev_map = apply_time_to_vm(vm_before_time_shift, candidate.minimum.time);
        ret.push_back(next_todo);
      }
    	// HAConverter, HASimulator用にTIME_LIMITのtodoも返す
    	if((opts_->ha_convert_mode || opts_->ha_simulator_mode) && pr->cause_for_termination == TIME_LIMIT)
    	{
        next_todo->current_time = pr->end_time;
        next_todo->parameter_map = pr->parameter_map;
        next_todo->parent = pr;
        ret.push_back(next_todo);
    	}

      if(one_phase || ++result_it >= results.size())break;
      next_todo = create_new_simulation_phase(next_todo);
    }
    current_todo->profile["NextPP"] += next_pp_timer.get_elapsed_us();
  }

  return ret;
}

void PhaseSimulator::replace_prev2parameter(
  phase_result_sptr_t& state,
  ConstraintStore& store,
  parameter_map_t &parameter_map)
{
  PrevReplacer replacer(parameter_map, state, *simulator_, opts_->approx);
  for(auto constraint : store)
  {
    replacer.replace_node(constraint);
  }
}

variable_map_t PhaseSimulator::apply_time_to_vm(const variable_map_t& vm, const value_t& tm)
{
  HYDLA_LOGGER_DEBUG("%% time: ", tm);
  variable_map_t result;
  TimeModifier modifier(*backend_);
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    result[it->first] = modifier.substitute_time(tm, it->second);
  }
  return result;
}


variable_map_t PhaseSimulator::shift_time_of_vm(const variable_map_t& vm, const value_t& tm)
{
  HYDLA_LOGGER_DEBUG("%% time: ", tm);
  variable_map_t result;
  TimeModifier modifier(*backend_);
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    result[it->first] = modifier.shift_time(tm, it->second);
  }
  return result;
}


}
}
