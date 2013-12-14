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

#include "PrevReplacer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "VariableFinder.h"
#include "VariableSearcher.h"

#include "InitNodeRemover.h"
#include "MathematicaLink.h"
#include "REDUCELinkFactory.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"

#include "Backend.h"
#include "Exceptions.h"
#include "AnalysisResultChecker.h"
#include "UnsatCoreFinder.h"
#include "TreeInfixPrinter.h"
#include "Dumpers.h"

using namespace hydla::backend;
using namespace hydla::backend::mathematica;
using namespace hydla::backend::reduce;


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
using hydla::simulator::VariableFinder;

namespace hydla {
namespace simulator {
namespace symbolic {

void SymbolicPhaseSimulator::init_arc(const parse_tree_sptr& parse_tree){
/* TODO: 一時無効
  analysis_result_checker_ = boost::shared_ptr<AnalysisResultChecker >(new AnalysisResultChecker(*opts_));
  analysis_result_checker_->initialize(parse_tree);
  analysis_result_checker_->set_solver(solver_);
  analysis_result_checker_->check_all_module_set((opts_->analysis_mode == "cmmap" ? true : false));
*/
}

module_set_list_t SymbolicPhaseSimulator::calculate_mms(
  simulation_todo_sptr_t& state,
  const variable_map_t& vm)
{
  timer::Timer cmms_timer;
  module_set_list_t ret = analysis_result_checker_->calculate_mms(state,vm,todo_container_);
  state->profile["CalculateMMS"] += cmms_timer.get_elapsed_us();
  return ret;
}

CalculateVariableMapResult
SymbolicPhaseSimulator::check_conditions
(const module_set_sptr& ms, simulation_todo_sptr_t& state, const variable_map_t& vm, bool b){
  timer::Timer cc_timer;
  bool cc_result = analysis_result_checker_->check_conditions(ms, state, vm, b, todo_container_);
  state->profile["CheckCondition " + ms->get_name()] += cc_timer.get_elapsed_us();
  if(cc_result){
    return CVM_CONSISTENT;
  }else{
    return CVM_INCONSISTENT;
  }
  return CVM_ERROR;
}

void
SymbolicPhaseSimulator::mark_nodes_by_unsat_core
(const module_set_sptr& ms,
 simulation_todo_sptr_t& todo,
 const variable_map_t& vm
 ){
  UnsatCoreFinder::unsat_constraints_t S;
  UnsatCoreFinder::unsat_continuities_t S4C;
  HYDLA_LOGGER_VAR(MS, ms);
  unsat_core_finder_->find_unsat_core(ms,S,S4C,todo,vm);
  ModuleSet module_set;
  for(UnsatCoreFinder::unsat_constraints_t::iterator it = S.begin(); it != S.end(); it++){
    HYDLA_LOGGER(MS, "unsat moduleset: ", *it->second);
    for(ModuleSet::module_list_const_iterator mit = it->second->begin(); mit != it->second->end(); mit++) module_set.add_module(*mit);
  }
  for(UnsatCoreFinder::unsat_continuities_t::iterator it = S4C.begin(); it != S4C.end(); it++){
    HYDLA_LOGGER(MS, "unsat moduleset: ", *it->second);
    for(ModuleSet::module_list_const_iterator mit = it->second->begin(); mit != it->second->end(); mit++) module_set.add_module(*mit);
  }
  HYDLA_LOGGER_VAR(MS, module_set);
  todo->module_set_container->mark_nodes(todo->maximal_mss, module_set);
}

void
SymbolicPhaseSimulator::find_unsat_core
(const module_set_sptr& ms,
 simulation_todo_sptr_t& todo,
 const variable_map_t& vm
 ){
  UnsatCoreFinder::unsat_constraints_t S;
  UnsatCoreFinder::unsat_continuities_t S4C;
  cout << "start find unsat core " << endl;
  cout << ms << endl;
  unsat_core_finder_->find_unsat_core(ms,S,S4C,todo,vm);
  unsat_core_finder_->print_unsat_cores(S,S4C);
  cout << "end find unsat core " << endl;
}


SymbolicPhaseSimulator::SymbolicPhaseSimulator(Simulator* simulator, const Opts& opts) :
  simulator_t(simulator, opts)
{
  unsat_core_finder_.reset(new UnsatCoreFinder());
}

SymbolicPhaseSimulator::~SymbolicPhaseSimulator()
{
}

void SymbolicPhaseSimulator::set_backend(backend_sptr_t back)
{
  PhaseSimulator::set_backend(back);
  unsat_core_finder_->set_backend(back);
}

void SymbolicPhaseSimulator::initialize(variable_set_t &v, parameter_map_t &p, variable_map_t &m, continuity_map_t& c, const module_set_container_sptr& msc)
{  
  simulator_t::initialize(v, p, m, c, msc);
  variable_derivative_map_ = c;
}


void SymbolicPhaseSimulator::set_simulation_mode(const Phase& phase)
{
  current_phase_ = phase;
}

void SymbolicPhaseSimulator::add_continuity(const continuity_map_t& continuity_map, const Phase &phase){
  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){

    std::string fmt = "v";
    if(phase == PointPhase)
    {
      fmt += "n";
    }
    else
    {
      fmt += "z";
    }
    fmt += "vp";
    if(it->second>=0){
      for(int i=0; i<it->second;i++){
        variable_t var(it->first, i);
        backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
    }else{
      for(int i=0; i<=-it->second;i++){
        variable_t var(it->first, i);
        backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
      if(phase == IntervalPhase)
      {
        node_sptr rhs(new Number("0"));
        fmt = phase == PointPhase?"vn":"vt";
        fmt += "en";
        variable_t var(it->first, -it->second + 1);
        backend_->call("addEquation", 2, fmt.c_str(), "", &var, &rhs);
      }
    }
  }
}

CheckConsistencyResult SymbolicPhaseSimulator::check_consistency(const Phase& phase)
{
  CheckConsistencyResult ret;
  if(phase == PointPhase)
  {
    backend_->call("checkConsistencyPoint", 0, "", "cc", &ret);
  }
  else
  {
    backend_->call("checkConsistencyInterval", 0, "", "cc", &ret);
  }
  return ret;
}

SymbolicPhaseSimulator::CheckEntailmentResult SymbolicPhaseSimulator::check_entailment(
  CheckConsistencyResult &cc_result,
  const node_sptr& guard,
  const continuity_map_t& cont_map,
  const Phase& phase
  )
{
  CheckEntailmentResult ce_result;
  backend_->call("startTemporary", 0, "", "");
  add_continuity(cont_map, phase);
  const char* fmt = (phase == PointPhase)?"en":"et";
  backend_->call("addConstraint", 1, fmt, "", &guard);
  cc_result = check_consistency(phase);
  assert(cc_result.size() == 2);
  if(!cc_result[0].empty()){
    HYDLA_LOGGER_CLOSURE("%% entailable");
    if(!cc_result[1].empty()){
      HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }
    else
    {
      backend_->call("endTemporary", 0, "", "");
      backend_->call("startTemporary", 0, "", "");
      add_continuity(cont_map, phase);
      node_sptr not_node = node_sptr(new Not(guard));
      const char* fmt = (phase == PointPhase)?"en":"et";
      backend_->call("addConstraint", 1, fmt, "", &not_node);
      cc_result = check_consistency(phase);
      assert(cc_result.size() == 2);
      if(!cc_result[0].empty()){
        HYDLA_LOGGER_CLOSURE("%% entailablity branches");
        if(!cc_result[1].empty()){
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
  backend_->call("endTemporary", 0, "", "");
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

  VariableSearcher variable_searcher;

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
      if(opts_->reuse && !state->changed_variables.empty() && !variable_searcher.visit_node(state->changed_variables,*it,true))
        continue;

      // send "Constraint" node, not "Tell"
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), state->phase == IntervalPhase, false);
    }

    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map, state->phase);
    
    for(constraints_t::const_iterator it = state->temporary_constraints.begin(); it != state->temporary_constraints.end(); it++){
      constraint_list.push_back(*it);
    }
    const char* fmt = (state->phase == PointPhase)?"csn":"cst";
    backend_->call("addConstraint", 1, fmt, "", &constraint_list);

    {
      CheckConsistencyResult cc_result;
      cc_result = check_consistency(state->phase);
      assert(cc_result.size() == 2);
      if(cc_result[0].empty()){
        HYDLA_LOGGER_CLOSURE("%% inconsistent for all cases");
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return false;
      }else if (cc_result[1].empty()){
        HYDLA_LOGGER_CLOSURE("%% consistent for all cases");
      }else{
        HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
        push_branch_states(state, cc_result);
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
        if(opts_->reuse && !state->changed_variables.empty() && !variable_searcher.visit_node(state->changed_variables,*it,true)){
          if(state->parent->positive_asks.find(*it) != state->parent->positive_asks.end())
            positive_asks.insert(*it);
          else negative_asks.insert(*it);
          unknown_asks.erase(it);
          it++;
          continue;
        }
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
        switch(check_entailment(check_consistency_result, (*it)->get_guard(), maker.get_continuity_map(), state->phase)){
          case ENTAILED:
            HYDLA_LOGGER_CLOSURE("--- entailed ask ---\n", *((*it)->get_guard()));
            positive_asks.insert(*it);
            if(prev_guards_.find(*it) != prev_guards_.end()){
              state->judged_prev_map.insert(std::make_pair(*it, true));
            }
            unknown_asks.erase(it++);
            expanded = true;
            
            if(opts_->reuse && !state->changed_variables.empty()){
              VariableFinder variable_finder;
              variable_finder.visit_node(*it,true);
              VariableFinder::variable_set_t tmp_vars = variable_finder.get_variable_set();
              for(VariableFinder::variable_set_t::iterator it=tmp_vars.begin(); it != tmp_vars.end(); it++){
                state->changed_variables.insert(it->first);
              }
            }
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

  add_continuity(continuity_map, state->phase);

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
  variable_maps_t& result_vms)
{
  HYDLA_LOGGER_FUNC_BEGIN(MS);
  
  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &todo->parameter_map);
  backend_->call("addPrevConstraint", 1, "mvp", "", &vm); //TODO: 左極限値と記号定数の初期化処理はもっと上でやった方が無駄が少ないはず

  timer::Timer cc_timer;
  
  // calculate closure
  bool result = calculate_closure(todo, ms);
  
  todo->profile["CalculateClosure"] += cc_timer.get_elapsed_us();

  if(!result)
  {
    return CVM_INCONSISTENT;
  }
  
  timer::Timer create_timer;
  vector<variable_map_t> create_result;
  if(todo->phase == PointPhase)
  {
    backend_->call("createVariableMap", 0, "", "cv", &create_result);
  }
  else
  {
    backend_->call("createVariableMapInterval", 0, "", "cv", &create_result);
  }
  todo->profile["CreateMap"] += create_timer.get_elapsed_us();

  HYDLA_LOGGER_VAR(MS, create_result.size());
  
  if(current_phase_ == IntervalPhase)
  {
    if(create_result.size() != 1){
      phase_result_sptr_t phase(new PhaseResult(*todo, simulator::NOT_UNIQUE_IN_INTERVAL));
      todo->parent->children.push_back(phase);
      HYDLA_LOGGER_FUNC_END(MS);
      return CVM_ERROR;
    }
    
    for(vector<variable_map_t>::iterator cr_it = create_result.begin(); cr_it != create_result.end(); cr_it++)
    {
      replace_prev2parameter(todo->parent, *cr_it, todo->parameter_map);
    }
  }


  result_vms = create_result;

  return CVM_CONSISTENT;
}

void SymbolicPhaseSimulator::set_changed_variables(phase_result_sptr_t& phase, simulation_todo_sptr_t& todo)
{
  TellCollector current_tell_collector(phase->module_set);
  tells_t current_tell_list;
  expanded_always_t& current_expanded_always = phase->expanded_always;
  positive_asks_t& current_positive_asks = phase->positive_asks;
  current_tell_collector.collect_all_tells(&current_tell_list,&current_expanded_always,&current_positive_asks);
  
  TellCollector prev_tell_collector(phase->parent->module_set);
  tells_t prev_tell_list;
  expanded_always_t& prev_expanded_always = phase->parent->expanded_always;
  positive_asks_t& prev_positive_asks = phase->parent->positive_asks;
  prev_tell_collector.collect_all_tells(&prev_tell_list,&prev_expanded_always,&prev_positive_asks);

  tells_t tmp_tells;
  tells_t::iterator tmp_it;
  for(tells_t::iterator it = current_tell_list.begin(); it != current_tell_list.end(); it++){
    tmp_it = std::find(prev_tell_list.begin(),prev_tell_list.end(),*it);
    if(tmp_it == prev_tell_list.end()){
      tmp_tells.push_back(*it);
    }else{
      prev_tell_list.erase(tmp_it);
    }
  }
  for(tells_t::iterator it = prev_tell_list.begin(); it != prev_tell_list.end(); it++)
    tmp_tells.push_back(*it);

  VariableFinder variable_finder;
  for(tells_t::iterator it = tmp_tells.begin(); it != tmp_tells.end(); it++){
    variable_finder.visit_node(*it,false);
  }
  VariableFinder::variable_set_t tmp_vars = variable_finder.get_variable_set();
  for(VariableFinder::variable_set_t::iterator it=tmp_vars.begin(); it != tmp_vars.end(); it++){
    todo->changed_variables.insert(it->first);
  }
}

SymbolicPhaseSimulator::todo_list_t 
  SymbolicPhaseSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo)
{
  HYDLA_LOGGER_FUNC_BEGIN(MS);
  
  backend_->call("resetConstraint", 0, "", "");
  const char* fmt = (current_todo->phase == PointPhase)?"mv0n":"mv0t";
  backend_->call("addConstraint", 1, fmt, "", &phase->variable_map);
  backend_->call("addParameterConstraint", 1, "mp", "", &phase->parameter_map);
  
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
    if(opts_->reuse && phase->id > 1)
      set_changed_variables(phase, next_todo);
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

    Backend::PPTimeResult time_result; 
    time_t time_limit(max_time->clone());
    *time_limit -= *phase->current_time;
    backend_->call("calculateNextPointPhaseTime", 2, "vltcst", "cp", &(time_limit), &disc_cause, &time_result);

    unsigned int time_it = 0;
    result_list_t results;
    phase_result_sptr_t pr = next_todo->parent;

    while(true)
    {
      Backend::PPTimeResult::NextPhaseResult &candidate = time_result.candidates[time_it];
      node_sptr time_node = candidate.time->get_node();
/*
      value_range_t tmp_range;
      if(solver_->approx_val(candidate.time, tmp_range))
      {
        parameter_t* param = simulator_->introduce_parameter(&simulator_->system_time_, pr, tmp_range);
        candidate.time = value_t(new SymbolicValue(node_sptr( 
                                                      new Parameter("time", 0, pr->id))));
        candidate.parameter_map[param] = tmp_range;
      }
*/
      // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
      for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
        pr->parameter_map[it->first] = it->second;
      }
      if(candidate.is_max_time) {
        pr->cause_of_termination = simulator::TIME_LIMIT;
      }
      else
      {
        time_node = node_sptr(new Plus(time_node, current_todo->current_time->get_node()));
      }
      backend_->call("simplify", 1, "et", "vl", &time_node, &pr->end_time);
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
      HYDLA_LOGGER_PHASE("%%time: ", *pr->current_time);
      if(pr->cause_of_termination != TIME_LIMIT)
      {
        next_todo->current_time = pr->end_time;
        next_todo->parameter_map = pr->parameter_map;
        next_todo->parent = pr;
        ret.push_back(next_todo);
      }
    	// HAConverter, HASimulator用にTIME_LIMITのtodoも返す
    	if((opts_->ha_convert_mode || opts_->ha_simulator_mode) && pr->cause_of_termination == TIME_LIMIT)
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

void SymbolicPhaseSimulator::replace_prev2parameter(
  phase_result_sptr_t& state,
  variable_map_t& vm,
  parameter_map_t &parameter_map)
{
  PrevReplacer replacer(parameter_map, state, *simulator_);
   
  for(variable_map_t::iterator it = vm.begin();
      it != vm.end(); it++)
  {
    HYDLA_LOGGER(PHASE, it->first, it->second);
    ValueRange& range = it->second;
    value_t val;
    if(range.unique())
    {
      val = range.get_unique();
      HYDLA_LOGGER(PHASE, *val);
      replacer.replace_value(val);
      range.set_unique(val);
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

  HYDLA_LOGGER(MS, "param: ", parameter_map);
  
  // TODO: オリジナルの記号定数表についても置き換えを行う。（記号定数表の中にprevが残っている可能性がある

/*
  // approx values in vm
  // TODO:ここでやると，変数同士の相関性を生かせない．
  if(state->phase == PointPhase)
  {
    for(variable_map_t::iterator it = ret.begin();
        it != ret.end();it++)
    { 
      value_range_t tmp_range;
      if(solver_->approx_val(it->second, tmp_range))
      {
        variable_t* variable = it->first;
            
        parameter_t* param = simulator_->introduce_parameter(variable, state, tmp_range);
        ret[variable] = value_t(new SymbolicValue(node_sptr(
          new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
        parameter_map[param] = tmp_range;
      }
    }
  }
*/
}


variable_map_t SymbolicPhaseSimulator::apply_time_to_vm(const variable_map_t& vm, const time_t& tm)
{
  HYDLA_LOGGER_FUNC_BEGIN(PHASE);
  HYDLA_LOGGER_PHASE("%% time: ", *tm);
  variable_map_t result;
  for(variable_map_t::const_iterator it = vm.begin(); it != vm.end(); it++)
  {
    if(it->second.undefined())
    {
      result[it->first] = it->second;
    }
    else if(it->second.unique())
    {
      value_t val = it->second.get_unique();
      range_t& range = result[it->first];
      value_t ret;
      backend_->call("applyTime2Expr", 2, "vltvlt", "vl", &val, &tm, &ret);
      range.set_unique(ret);
    }
    else
    {
      range_t range = it->second;
      for(uint i = 0; i < range.get_lower_cnt(); i++)
      {
        ValueRange::bound_t bd = it->second.get_lower_bound(i);
        value_t val = bd.value;
        value_t ret;
        backend_->call("applyTime2Expr", 2, "vltvlt", "vl", &val, &tm, &ret);
        range.set_lower_bound(ret, bd.include_bound);
      }
      for(uint i = 0; i < range.get_upper_cnt(); i++)
      {

        ValueRange::bound_t bd = it->second.get_upper_bound(i);
        value_t val = bd.value;
        value_t ret;
        backend_->call("applyTime2Expr", 2, "vltvlt", "vl", &val, &tm, &ret);
        range.set_upper_bound(ret, bd.include_bound);
      }
      result[it->first] = range;
    }
  }
  HYDLA_LOGGER_FUNC_END(PHASE);  
  return result;
}


variable_map_t SymbolicPhaseSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
  variable_map_t shifted_vm;
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  for(; it!=end; ++it) {
    if(it->second.undefined())
      shifted_vm[it->first] = it->second;
    else if(it->second.unique())
    {
      value_t val = it->second.get_unique();
      range_t& range = shifted_vm[it->first];
      value_t ret;
      backend_->call("exprTimeShift", 2, "vltvlt", "vl", &val, &time, &ret);
      range.set_unique(ret);
    }
    else
    {
      range_t range = it->second;
      for(uint i = 0; i < range.get_lower_cnt(); i++)
      {
        ValueRange::bound_t bd = it->second.get_lower_bound(i);
        value_t val = bd.value;
        value_t ret;
        backend_->call("exprTimeShift", 2, "vltvlt", "vl", &val, &time, &ret);
        range.set_lower_bound(ret, bd.include_bound);
      }
      for(uint i = 0; i < range.get_upper_cnt(); i++)
      {

        ValueRange::bound_t bd = it->second.get_upper_bound(i);
        value_t val = bd.value;
        value_t ret;
        backend_->call("exprTimeShift", 2, "vltvlt", "vl", &val, &time, &ret);
        range.set_upper_bound(ret, bd.include_bound);
      }
      shifted_vm[it->first] = range;
    }
  }
  return shifted_vm;
}

} //namespace symbolic
} //namespace simulator
} //namespace hydla
