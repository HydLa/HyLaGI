#include "SymbolicPhaseSimulator.h"

#include <iostream>
#include <fstream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"
#include "Timer.h"

#include "ParameterReplacer.h"
#include "TellCollector.h"
#include "AskCollector.h"

#include "InitNodeRemover.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"

#include "Exceptions.h"
#include "AnalysisResultChecker.h"
#include "TreeInfixPrinter.h"
#include "Dumpers.h"

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;


using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;
using namespace hydla::timer;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;
using hydla::parse_tree::TreeInfixPrinter;

namespace hydla {
namespace simulator {
namespace symbolic {

CalculateVariableMapResult
SymbolicPhaseSimulator::check_false_conditions
(const module_set_sptr& ms, simulation_todo_sptr_t& state, const variable_map_t& vm){
  return analysis_result_checker_->check_false_conditions(ms, state, vm);
}


SymbolicPhaseSimulator::SymbolicPhaseSimulator(Simulator* simulator, const Opts& opts) :
  simulator_t(simulator, opts)
{
}

SymbolicPhaseSimulator::~SymbolicPhaseSimulator()
{
}

void SymbolicPhaseSimulator::initialize(variable_set_t &v, parameter_set_t &p, variable_range_map_t &m, continuity_map_t& c, const module_set_container_sptr& msc)
{
  simulator_t::initialize(v, p, m, c, msc);
  variable_derivative_map_ = c;
  solver_.reset(new MathematicaVCS(*opts_));
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);
}


void SymbolicPhaseSimulator::set_simulation_mode(const Phase& phase)
{
  current_phase_ = phase;
}

parameter_set_t SymbolicPhaseSimulator::get_parameter_set(){
  return *parameter_set_;
}


void SymbolicPhaseSimulator::add_continuity(const continuity_map_t& continuity_map){
  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){
    if(it->second>=0){
      for(int i=0; i<it->second;i++){
        solver_->set_continuity(it->first, i);
      }
    }else{
      node_sptr lhs(new Variable(it->first));
      for(int i=0; i<=-it->second;i++){
        solver_->set_continuity(it->first, i);
        lhs = node_sptr(new Differential(lhs));
      }
      node_sptr rhs(new Number("0"));
      node_sptr cons(new Equal(lhs, rhs));
      solver_->add_constraint(cons);
    }
  }
}

SymbolicPhaseSimulator::CheckEntailmentResult SymbolicPhaseSimulator::check_entailment(
  CheckConsistencyResult &cc_result,
  const node_sptr& guard,
  const continuity_map_t& cont_map
  )
{
  CheckEntailmentResult ce_result;
  solver_->start_temporary();
  add_continuity(cont_map);
  solver_->add_guard(guard);
  cc_result = solver_->check_consistency();
  if(!cc_result.true_parameter_maps.empty()){
    HYDLA_LOGGER_CLOSURE("%% entailable");
    if(!cc_result.false_parameter_maps.empty()){
      HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }
    else
    {
      solver_->end_temporary();
      solver_->start_temporary();
      add_continuity(cont_map);
      solver_->add_guard(node_sptr(new Not(guard)));
      cc_result = solver_->check_consistency();
      if(!cc_result.true_parameter_maps.empty()){
        HYDLA_LOGGER_CLOSURE("%% entailablity branches");
        if(!cc_result.false_parameter_maps.empty()){
          HYDLA_LOGGER_LOCATION(CLOSURE);
          HYDLA_LOGGER_CLOSURE("%% branches by parameters");
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
  solver_->end_temporary();
  return ce_result;
}


bool SymbolicPhaseSimulator::calculate_closure(simulation_todo_sptr_t& state,
    const module_set_sptr& ms)
{
  HYDLA_LOGGER_FUNC_BEGIN(CLOSURE);
  
  // preparation
  positive_asks_t& positive_asks = state->positive_asks;
  negative_asks_t& negative_asks = state->negative_asks;
  ask_set_t unknown_asks;
  expanded_always_t& expanded_always = state->expanded_always;
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  tells_t         tell_list;
  constraints_t   constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;

  bool expanded;
  
  do{
    tell_collector.collect_new_tells(&tell_list,
        &expanded_always,
        &positive_asks);

    HYDLA_LOGGER_LOCATION(CLOSURE);
    timer::Timer consistency_timer;

    constraint_list.clear();

    maker.reset();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      // send "Constraint" node, not "Tell"
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), state->phase == IntervalPhase, false);
    }

    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map);

    for(constraints_t::const_iterator it = state->temporary_constraints.begin(); it != state->temporary_constraints.end(); it++){
      constraint_list.push_back(*it);
    }

    solver_->add_constraint(constraint_list);

    {
      CheckConsistencyResult check_consistency_result = solver_->check_consistency();
      if(check_consistency_result.true_parameter_maps.empty()){
        HYDLA_LOGGER_CLOSURE("%% inconsistent for all cases");
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return false;
      }else if (check_consistency_result.false_parameter_maps.empty()){
        HYDLA_LOGGER_CLOSURE("%% consistent for all cases");
      }else{
        HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
        push_branch_states(state, check_consistency_result);
      }
    }
    
    state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();


    HYDLA_LOGGER_LOCATION(CLOSURE);
    
    ask_collector.collect_ask(&expanded_always, 
        &positive_asks, 
        &negative_asks,
        &unknown_asks);
    
    timer::Timer entailment_timer;
    
    {
      expanded = false;
      ask_set_t::iterator it  = unknown_asks.begin();
      ask_set_t::iterator end = unknown_asks.end();
      while(it!=end){
        if(state->phase == PointPhase){
          if(state->current_time->get_string() == "0" && PrevSearcher().search_prev((*it)->get_guard())){
            // if current time equals to 0, conditions about left-hand limits are considered to be invalid
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            continue;
          }else if(prev_guards_.find(*it) != prev_guards_.end() && 
            state->judged_prev_map.find(*it) != state->judged_prev_map.end())
          {
            // if this guard doesn't have non-prev variable and it has been already judged
            bool entailed = state->judged_prev_map.find(*it)->second;
            HYDLA_LOGGER_CLOSURE("%% ommitted guard: ", **it, ", entailed: ", entailed);
            if(entailed)
            {
              positive_asks.insert(*it);
              expanded = true;
            }else
            {
              negative_asks.insert(*it);
            }
            unknown_asks.erase(it++);
            continue;
          }
        }
        maker.visit_node((*it)->get_child(), state->phase == IntervalPhase, true);
        
        CheckConsistencyResult check_consistency_result;
        switch(check_entailment(check_consistency_result, (*it)->get_guard(), maker.get_continuity_map())){
          case ENTAILED:
            HYDLA_LOGGER_CLOSURE("--- entailed ask ---\n", *((*it)->get_guard()));
            positive_asks.insert(*it);
            if(prev_guards_.find(*it) != prev_guards_.end()){
              state->judged_prev_map.insert(std::make_pair(*it, true));
            }
            unknown_asks.erase(it++);
            expanded = true;
            break;
          case CONFLICTING:
            HYDLA_LOGGER_CLOSURE("--- conflicted ask ---\n", *((*it)->get_guard()));
            negative_asks.insert(*it);
            if(prev_guards_.find(*it) != prev_guards_.end()){
              state->judged_prev_map.insert(std::make_pair(*it, false));
            }
            unknown_asks.erase(it++);
            break;
          case BRANCH_VAR:
            HYDLA_LOGGER_CLOSURE("--- branched ask ---\n", *((*it)->get_guard()));
            it++;
            break;
          case BRANCH_PAR:
            HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
            push_branch_states(state, check_consistency_result);
            break;
        }
        maker.set_continuity_map(continuity_map);
      }
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
  }while(expanded);

  add_continuity(continuity_map);

  if(!unknown_asks.empty()){
    boost::shared_ptr<hydla::parse_tree::Ask> branched_ask = *unknown_asks.begin();
    // TODO: 極大性に対して厳密なものになっていない（実行アルゴリズムを実装しきれてない）
    HYDLA_LOGGER_CLOSURE("%% branched_ask:", TreeInfixPrinter().get_infix_string(branched_ask));
    {
      // 分岐先を生成（導出される方）
      simulation_todo_sptr_t new_todo(create_new_simulation_phase(state));
      new_todo->temporary_constraints.push_back((branched_ask)->get_guard());
      todo_container_->push_todo(new_todo);
    }
    {
      // 分岐先を生成（導出されない方）
      state->temporary_constraints.push_back(node_sptr(new Not((branched_ask)->get_guard())));
      negative_asks.insert(branched_ask);
      return calculate_closure(state, ms);
    }
  }
  
  HYDLA_LOGGER_FUNC_END(CLOSURE);
  return true;
}

CalculateVariableMapResult 
SymbolicPhaseSimulator::calculate_variable_map(
  const module_set_sptr& ms,
  simulation_todo_sptr_t& todo,
  const variable_map_t & vm,
  variable_range_maps_t& result_vms)
{
  HYDLA_LOGGER_FUNC_BEGIN(MS);
  
  // preparation
  if(current_phase_ == PointPhase){
    solver_->change_mode(DiscreteMode, opts_->approx_precision);
  }
  else
  {
    solver_->change_mode(ContinuousMode, opts_->approx_precision);
  }
  solver_->reset(vm, todo->parameter_map); //TODO: 左極限値と記号定数の初期化処理はもっと上でやった方が無駄が少ないはず

  timer::Timer cc_timer;
  
  // calculate closure
  bool result = calculate_closure(todo, ms);
  
  todo->profile["CalculateClosure"] += cc_timer.get_elapsed_us();

  if(!result)
  {
    HYDLA_LOGGER_FUNC_END(MS);
    return CVM_INCONSISTENT;
  }
  
  timer::Timer create_timer;
  SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
  todo->profile["CreateMap"] += create_timer.get_elapsed_us();
  SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;
  
  if(results.size() != 1 && current_phase_ == IntervalPhase){
    phase_result_sptr_t phase(new PhaseResult(*todo, simulator::NOT_UNIQUE_IN_INTERVAL));
    todo->parent->children.push_back(phase);
    HYDLA_LOGGER_FUNC_END(MS);
    return CVM_ERROR;
  }
  
  result_vms = results;

  HYDLA_LOGGER_FUNC_END(MS);
  return CVM_CONSISTENT;
}

SymbolicPhaseSimulator::todo_list_t 
  SymbolicPhaseSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo)
{
  HYDLA_LOGGER_FUNC_BEGIN(MS);
  
  solver_->reset_constraint(phase->variable_map, false);
  
  todo_list_t ret;
  
  simulation_todo_sptr_t next_todo(new SimulationTodo());
  next_todo->module_set_container = msc_no_init_;
  next_todo->parent = phase;
  next_todo->ms_to_visit = next_todo->module_set_container->get_full_ms_list();
  next_todo->expanded_always = phase->expanded_always;
  next_todo->parameter_map = phase->parameter_map;
  
  if(current_phase_ == PointPhase)
  {
    next_todo->phase = IntervalPhase;
    next_todo->current_time = phase->current_time;
    ret.push_back(next_todo);
  }
  else
  {
    assert(current_phase_ == IntervalPhase);
    next_todo->phase = PointPhase;
    phase->variable_map = shift_variable_map_time(phase->variable_map, phase->current_time);

    timer::Timer next_pp_timer;
    constraints_t disc_cause;
    //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
    for(positive_asks_t::const_iterator it = phase->positive_asks.begin(); it != phase->positive_asks.end(); it++){
      disc_cause.push_back(node_sptr(new Not((*it)->get_guard() ) ) );
    }
    //現在導出されていないガード条件を離散変化条件として追加
    for(negative_asks_t::const_iterator it = phase->negative_asks.begin(); it != phase->negative_asks.end(); it++){
      disc_cause.push_back((*it)->get_guard());
    }

    //assertionの否定を追加
    if(opts_->assertion){
      disc_cause.push_back(node_sptr(new Not(opts_->assertion)));
    }

    time_t max_time;
    if(opts_->max_time != ""){
      max_time.reset(new SymbolicValue(node_sptr(new hydla::parse_tree::Number(opts_->max_time))));
    }else{
      max_time.reset(new SymbolicValue(node_sptr(new hydla::parse_tree::Infinity)));
    }

    SymbolicVirtualConstraintSolver::PPTimeResult 
      time_result = solver_->calculate_next_PP_time(disc_cause, phase->current_time, max_time);

    unsigned int time_it = 0;
    result_list_t results;
    phase_result_sptr_t pr = next_todo->parent;

    while(true)
    {
      SymbolicVirtualConstraintSolver::PPTimeResult::NextPhaseResult &candidate = time_result.candidates[time_it];
      solver_->simplify(candidate.time);
      // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
      for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
        pr->parameter_map[it->first] = it->second;
      }
      pr->end_time = candidate.time;
      if(candidate.is_max_time) {
        pr->cause_of_termination = simulator::TIME_LIMIT;
      }
      results.push_back(pr);
      if(++time_it >= time_result.candidates.size())break;
      pr = make_new_phase(pr);
    }
    
    unsigned int result_it = 0;
    bool one_phase = false;
    
    if(time_result.candidates.size() > 0 && select_phase_)
    {
      result_it = select_phase_(results);
      one_phase = true;
    }
    
    while(true)
    {
      pr = results[result_it];
      if(pr->cause_of_termination != TIME_LIMIT)
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
  
  
  HYDLA_LOGGER_FUNC_END(MS);
  
  return ret;
}

variable_map_t SymbolicPhaseSimulator::range_map_to_value_map(
  phase_result_sptr_t& state,
  const variable_range_map_t& rm,
  parameter_map_t &parameter_map)
{
  variable_map_t ret;
  ParameterReplacer replacer;
  for(variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
    variable_t* variable = get_variable(r_it->first->get_name(), r_it->first->get_derivative_count());
    if(r_it->second.is_unique()){
      ret[variable] = r_it->second.get_lower_bound().value;
    }
    else
    {
      replacer.add_mapping(variable->get_name(), variable->get_derivative_count(), state->id);
      value_range_t range = r_it->second;
      // introduce parameter
      parameter_t* param = simulator_->introduce_parameter(r_it->first, state, range);
      ret[variable] = value_t(new SymbolicValue(node_sptr(
        new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
      parameter_map[param] = r_it->second;
      // TODO:ここで，Parameter(variable->get_name(), variable->get_derivative_count(), state->id)))はPhaseResult自体を参照していないので，
      // もしPhaseResultをこの処理以降に変更するような実装にした場合，整合性が取れなくなるのでどうにかする
      // TODO: ここで常に記号定数を導入するようにしておくと，記号定数が増えすぎて見づらくなる可能性がある．
      // 解軌道木上で一度しか出現しないUNDEFに対しては，記号定数を導入する必要が無いはずなので，どうにかそれを実現したい？
      // 逆にここでUNDEFにすると，次のIPでのデフォルト連続性と噛み合わずバグが発生する可能性があるので注意する．
    }
  }
  // 仮ID(-1)を設定した記号定数のIDを正しく設定し直す
  
  for(parameter_map_t::iterator it = simulator_->original_parameter_map_->begin();
      it != simulator_->original_parameter_map_->end(); it++)
  {
    ValueRange& range = it->second;
    value_t val = range.get_upper_bound().value;
    if(val.get() && !val->undefined())
    {
      replacer.replace_value(val);
      range.set_upper_bound(val, range.get_upper_bound().include_bound);
    }
    val = range.get_lower_bound().value;
    if(val.get() && !val->undefined())
    {
      replacer.replace_value(val);
      range.set_lower_bound(val, range.get_lower_bound().include_bound);
    }
  }
  
  
  
  for(parameter_map_t::iterator it = parameter_map.begin();
      it != parameter_map.end(); it++)
  {
    ValueRange& range = it->second;
    value_t val = range.get_upper_bound().value;
    if(val.get() && !val->undefined())
    {
      replacer.replace_value(val);
      range.set_upper_bound(val, range.get_upper_bound().include_bound);
    }
    val = range.get_lower_bound().value;
    if(val.get() && !val->undefined())
    {
      replacer.replace_value(val);
      range.set_lower_bound(val, range.get_lower_bound().include_bound);
    }
  }
  
  
  for(variable_map_t::iterator it = ret.begin();
      it != ret.end(); it++)
  {
    value_t val = it->second;
    if( val.get() && !val->undefined())
    {
      replacer.replace_value(val);
      ret[it->first] = val;
    }
  }
  
  
  return ret;
}


variable_map_t SymbolicPhaseSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
  variable_map_t shifted_vm;
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  for(; it!=end; ++it) {
    if(it->second->undefined())
      shifted_vm[it->first] = it->second;
    else
      shifted_vm[it->first] = solver_->shift_expr_time(it->second, time);
  }
  return shifted_vm;
}

} //namespace symbolic
} //namespace simulator
} //namespace hydla
