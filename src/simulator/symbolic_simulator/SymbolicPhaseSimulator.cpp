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
#include "AskConstraintVariableFinder.h"

#include "InitNodeRemover.h"
#include "MathematicaLink.h"
#include "REDUCELinkFactory.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"
#include "NonPrevSearcher.h"

#include "Backend.h"
#include "Exceptions.h"
#include "AnalysisResultChecker.h"
#include "UnsatCoreFinder.h"

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
  HYDLA_LOGGER_DEBUG_VAR(*ms);
  unsat_core_finder_->find_unsat_core(ms,S,S4C,todo,vm);
  ModuleSet module_set;
  for(UnsatCoreFinder::unsat_constraints_t::iterator it = S.begin(); it != S.end(); it++){
    HYDLA_LOGGER_DEBUG("unsat moduleset: ", *it->second);
    for(ModuleSet::module_list_const_iterator mit = it->second->begin(); mit != it->second->end(); mit++) module_set.add_module(*mit);
  }
  for(UnsatCoreFinder::unsat_continuities_t::iterator it = S4C.begin(); it != S4C.end(); it++){
    HYDLA_LOGGER_DEBUG("unsat moduleset: ", *it->second);
    for(ModuleSet::module_list_const_iterator mit = it->second->begin(); mit != it->second->end(); mit++) module_set.add_module(*mit);
  }
  HYDLA_LOGGER_DEBUG_VAR(module_set);
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
  unsat_core_finder_->find_unsat_core(ms,S,S4C, todo->parent->positive_asks, todo->parent->negative_asks, vm, todo->parent->parameter_map, todo->parent->phase);
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

void SymbolicPhaseSimulator::initialize(variable_set_t &v, parameter_map_t &p, variable_map_t &m, continuity_map_t& c, parse_tree_sptr pt, const module_set_container_sptr& msc)
{  
  simulator_t::initialize(v, p, m, c, pt, msc);
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
  HYDLA_LOGGER_DEBUG(get_infix_string(guard) );
  backend_->call("startTemporary", 0, "", "");
  add_continuity(cont_map, phase);
  const char* fmt = (phase == PointPhase)?"en":"et";
  backend_->call("addConstraint", 1, fmt, "", &guard);
  cc_result = check_consistency(phase);
  assert(cc_result.size() == 2);
  if(!cc_result[0].empty()){
    HYDLA_LOGGER_DEBUG("%% entailable");
    if(!cc_result[1].empty()){
      HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
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
        HYDLA_LOGGER_DEBUG("%% entailablity branches");
        if(!cc_result[1].empty()){
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
  backend_->call("endTemporary", 0, "", "");
  return ce_result;
}

bool SymbolicPhaseSimulator::calculate_closure(simulation_todo_sptr_t& state,
    const module_set_sptr& ms)
{
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

  change_variables_t changing_variables;

  if(opts_->reuse && state->in_following_step() ){
    if(state->phase == PointPhase){
      ask_collector.collect_ask(&expanded_always, 
          &positive_asks, 
          &negative_asks,
          &unknown_asks);
      apply_discrete_causes_to_guard_judgement( state->discrete_causes, positive_asks, negative_asks, unknown_asks );
      set_changing_variables( state->parent, ms, positive_asks, negative_asks, changing_variables );
      //std::cout << changing_variables << std::endl;
    }
    else{
      changing_variables = state->parent->changed_variables;
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
    tell_collector.collect_new_tells(&tell_list,
        &expanded_always,
        &positive_asks);

    timer::Timer consistency_timer;

    constraint_list.clear();

    maker.reset();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      if(opts_->reuse && state->in_following_step() &&
          !variable_searcher.visit_node(changing_variables,*it,state->phase == IntervalPhase)){
        continue;
      }

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
        HYDLA_LOGGER_DEBUG("%% inconsistent for all cases");
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return false;
      }else if (cc_result[1].empty()){
        HYDLA_LOGGER_DEBUG("%% consistent for all cases");
      }else{
        HYDLA_LOGGER_DEBUG("%% consistency depends on conditions of parameters\n");
        push_branch_states(state, cc_result);
      }
    }
    
    state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();


    ask_collector.collect_ask(&expanded_always, 
        &positive_asks, 
        &negative_asks,
        &unknown_asks);

    timer::Timer entailment_timer;
    
    {
      expanded = false;
      ask_set_t cv_unknown_asks, notcv_unknown_asks;
      if(opts_->reuse && state->in_following_step()){
        for( auto ask : unknown_asks ){
          if( variable_searcher.visit_node(changing_variables, ask, state->phase == IntervalPhase) ){
            cv_unknown_asks.insert(ask);
          }else{
            notcv_unknown_asks.insert(ask);
          }
        }
        unknown_asks = cv_unknown_asks;
      }
      ask_set_t::iterator it  = unknown_asks.begin();
      ask_set_t::iterator end = unknown_asks.end();
      while(it!=end){
        /*
        if(opts_->reuse && !state->parent->changed_variables.empty() && !variable_searcher.visit_node(state->parent->changed_variables,*it,true)){
          if(state->parent->positive_asks.find(*it) != state->parent->positive_asks.end())
            positive_asks.insert(*it);
          else negative_asks.insert(*it);
          unknown_asks.erase(it);
          it++;
          continue;
        }
        */
        if(state->phase == PointPhase){
          if(state->current_time.get_string() == "0" && PrevSearcher().search_prev((*it)->get_guard())){
            // if current time equals to 0, conditions about left-hand limits are considered to be invalid
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            continue;
          }else if(prev_guards_.find(*it) != prev_guards_.end() && 
            state->judged_prev_map.find(*it) != state->judged_prev_map.end())
          {
            // if this guard doesn't have non-prev variable and it has been already judged
            bool entailed = state->judged_prev_map.find(*it)->second;
            HYDLA_LOGGER_DEBUG("%% ommitted guard: ", **it, ", entailed: ", entailed);
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

        if(opts_->reuse && state->in_following_step()){
          apply_previous_solution(changing_variables, it,
              state->phase == IntervalPhase, state->parent );
        }
        
        CheckConsistencyResult check_consistency_result;
        switch(check_entailment(check_consistency_result, (*it)->get_guard(), maker.get_continuity_map(), state->phase)){
          case ENTAILED:
            HYDLA_LOGGER_DEBUG("--- entailed ask ---\n", *((*it)->get_guard()));
            if(opts_->reuse && state->in_following_step()){
              if(state->phase == PointPhase){
                apply_entailment_change(it, state->parent->negative_asks,
                    false, changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }else{
                apply_entailment_change(it, state->parent->parent->negative_asks,
                    true, changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }
            }
            positive_asks.insert(*it);
            if(prev_guards_.find(*it) != prev_guards_.end()){
              state->judged_prev_map.insert(std::make_pair(*it, true));
            }
            unknown_asks.erase(it++);
            expanded = true;
           /* 
            if(opts_->reuse && !state->parent->changed_variables.empty()){
              VariableFinder variable_finder;
              variable_finder.visit_node(*it,true);
              VariableFinder::variable_set_t tmp_vars = variable_finder.get_variable_set();
              for(VariableFinder::variable_set_t::iterator it=tmp_vars.begin(); it != tmp_vars.end(); it++){
                state->parent->changed_variables.insert(it->first);
              }
            }
            */
            break;
          case CONFLICTING:
            HYDLA_LOGGER_DEBUG("--- conflicted ask ---\n", *((*it)->get_guard()));
            if(opts_->reuse && state->in_following_step() ){
              if(state->phase == PointPhase){
                entailment_changed = apply_entailment_change(it, state->parent->positive_asks,
                    false, changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }else{
                entailment_changed = apply_entailment_change(it, state->parent->parent->positive_asks,
                    true, changing_variables,
                    notcv_unknown_asks, unknown_asks );
              }
              if(entailment_changed) break;
            }
            negative_asks.insert(*it);
            if(prev_guards_.find(*it) != prev_guards_.end()){
              state->judged_prev_map.insert(std::make_pair(*it, false));
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
        if(entailment_changed) break;
        maker.set_continuity_map(continuity_map);
      }
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    if(entailment_changed) continue;
  }while(expanded);

  add_continuity(continuity_map, state->phase);

  if(!unknown_asks.empty()){
    boost::shared_ptr<hydla::parse_tree::Ask> branched_ask = *unknown_asks.begin();
    // TODO: 極大性に対して厳密なものになっていない（実行アルゴリズムを実装しきれてない）
    HYDLA_LOGGER_DEBUG("%% branched_ask:", get_infix_string(branched_ask));
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

  return true;
}

CalculateVariableMapResult 
SymbolicPhaseSimulator::calculate_variable_map(
  const module_set_sptr& ms,
  simulation_todo_sptr_t& todo,
  const variable_map_t & vm,
  variable_maps_t& result_vms)
{
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

  HYDLA_LOGGER_DEBUG_VAR(create_result.size());
  
  if(current_phase_ == IntervalPhase)
  {
    if(create_result.size() != 1){
      phase_result_sptr_t phase(new PhaseResult(*todo, simulator::NOT_UNIQUE_IN_INTERVAL));
      todo->parent->children.push_back(phase);
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

void SymbolicPhaseSimulator::apply_discrete_causes_to_guard_judgement( ask_set_t& discrete_causes,
                                                                       positive_asks_t& positive_asks,
                                                                       negative_asks_t& negative_asks,
                                                                       ask_set_t& unknown_asks ){
  /*
  std::cout << "before" << std::endl;
  std::cout << "A+: " << positive_asks << std::endl;
  std::cout << "A-: " << negative_asks << std::endl;
  std::cout << "Au: " << unknown_asks << std::endl;
  */

  NonPrevSearcher np_searcher;
  ask_set_t prev_asks = unknown_asks;

  for( auto ask : unknown_asks ){
    if( np_searcher.judge_non_prev(ask) ){
      prev_asks.erase(ask);
    }else{
      unknown_asks.erase(ask);
    }
  }

  for( auto prev_ask : prev_asks ){
    if( discrete_causes.find(prev_ask) != discrete_causes.end() ){
      positive_asks.insert( prev_ask );
    }else{
      negative_asks.insert( prev_ask );
    }
  }

  /*
  std::cout << "after" << std::endl;
  std::cout << "A+: " << positive_asks << std::endl;
  std::cout << "A-: " << negative_asks << std::endl;
  std::cout << "Au: " << unknown_asks << std::endl;
  */
}

void SymbolicPhaseSimulator::set_changing_variables( const phase_result_sptr_t& parent_phase,
                                                           const module_set_sptr& present_ms,
                                                           const positive_asks_t& positive_asks,
                                                           const negative_asks_t& negative_asks,
                                                           change_variables_t& changing_variables ){
  //条件なし制約の差分取得
  module_set_sptr parent_ms = parent_phase->module_set;
  TellCollector parent_t_collector(parent_ms);
  tells_t parent_tells;
  //条件なし制約だけ集める
  expanded_always_t empty_ea;
  positive_asks_t empty_asks;
  parent_t_collector.collect_all_tells(&parent_tells, &empty_ea, &empty_asks );

  TellCollector t_collector(present_ms);
  tells_t tells;
  t_collector.collect_all_tells(&tells, &empty_ea, &empty_asks );

  changing_variables = get_difference_variables_from_2tells( parent_tells, tells );

  //導出状態の差分取得
  //現在はpositiveだけど、parentではpositiveじゃないやつ
  //現在はnegativeだけど、parentではpositiveなやつ
  AskConstraintVariableFinder v_finder;
  VariableSearcher v_searcher;
  positive_asks_t parent_positives = parent_phase->positive_asks;
  int cv_count = changing_variables.size();
  for( auto ask : positive_asks ){
    if(parent_positives.find(ask) == parent_positives.end() ){
      v_finder.visit_node(ask, false);
      VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
      for( auto var : tmp_vars ) changing_variables.insert(var.first);
    }
  }

  for( auto ask : negative_asks ){
    if(parent_positives.find(ask) != parent_positives.end() ){
      v_finder.visit_node(ask, false);
      VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
      for( auto var : tmp_vars ) changing_variables.insert(var.first);
    }
  }

  if(changing_variables.size() > cv_count){
    cv_count = changing_variables.size();
    while(true){
      for( auto tell : tells ){
        bool has_cv = v_searcher.visit_node(changing_variables, tell, false );
        if(has_cv){
          v_finder.clear();
          v_finder.visit_node(tell, false);
          VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
          for( auto var : tmp_vars )
            changing_variables.insert(var.first);
        }
      }
      if(changing_variables.size() > cv_count ){
        cv_count = changing_variables.size();
        continue;
      }
      break;
    }
  }
}

void SymbolicPhaseSimulator::set_changed_variables(phase_result_sptr_t& phase)
{
  if(phase->parent.get() == NULL)return;
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

  phase->changed_variables = get_difference_variables_from_2tells(current_tell_list, prev_tell_list);
}

change_variables_t SymbolicPhaseSimulator::get_difference_variables_from_2tells(const tells_t& larg, const tells_t& rarg){
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
    v_finder.visit_node(tell, false);

  VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
  for( auto var : tmp_vars )
    cv.insert(var.first);

  VariableSearcher v_searcher;
  int v_count = cv.size();
  while(true){
    for( auto tell : intersection_tells ){
      bool has_cv = v_searcher.visit_node(cv, tell, false );
      if(has_cv){
        v_finder.clear();
        v_finder.visit_node(tell, false);
        tmp_vars = v_finder.get_variable_set();
        for( auto var : tmp_vars )
          cv.insert(var.first);
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

bool SymbolicPhaseSimulator::apply_entailment_change( const ask_set_t::iterator it,
                                                      const ask_set_t& previous_asks,
                                                      const bool in_IP,
                                                      change_variables_t& changing_variables,
                                                      ask_set_t& notcv_unknown_asks,
                                                      ask_set_t& unknown_asks ){
  bool ret = false;
  if(previous_asks.find(*it) != previous_asks.end() ){
    VariableFinder v_finder;
    v_finder.visit_node(*it, in_IP);
    VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
    int v_count = changing_variables.size();
    for(auto var : tmp_vars){
      changing_variables.insert(var.first);
    }
    if(changing_variables.size() > v_count){
      ask_set_t change_asks;
      VariableSearcher v_searcher;
      for(auto ask : notcv_unknown_asks){
        if(v_searcher.visit_node(changing_variables, ask->get_child(), in_IP) ){
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

void SymbolicPhaseSimulator::apply_previous_solution(const change_variables_t& changing_variables,
                                                     const ask_set_t::iterator it,
                                                     const bool in_IP,
                                                     const phase_result_sptr_t parent ){
  VariableFinder v_finder;
  v_finder.visit_node((*it)->get_guard(),in_IP); 
  VariableFinder::variable_set_t tmp_vars = v_finder.get_variable_set();
  continuity_map_t continuity_map;
  for(auto var : tmp_vars){
    continuity_map_t::iterator tmp_it = continuity_map.find(var.first);
    if(tmp_it != continuity_map.end() ){
      if(tmp_it->second < var.second ){
        continuity_map.erase(tmp_it);
        continuity_map.insert(var);
      }
    }else{
      continuity_map.insert(var);
    }
  }
  for(auto var : continuity_map){
    if(changing_variables.find(var.first) == changing_variables.end()){
      for(int i=0; i<=var.second; i++){
        if(in_IP){
          //add_constraint var.firstのvar.second微分 = parent.variable_mapから追加
          //parameterも考慮
        }else{
          //add_constraint var.firstのvar.second微分 = prev[var.firstのvar.second微分]
        }
      }
    }
  }
}

SymbolicPhaseSimulator::todo_list_t 
  SymbolicPhaseSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo)
{
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
    ret.push_back(next_todo);
  }
  else
  {
    assert(current_phase_ == IntervalPhase);
    phase->variable_map = shift_variable_map_time(phase->variable_map, phase->current_time);
    next_todo->phase = PointPhase;

    timer::Timer next_pp_timer;
    dc_causes_t dc_causes;
    
    //TODO: どこかで事前にmapを作って、毎回作らないようにする。
    std::map<int, boost::shared_ptr<Ask> > ask_map;

    //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
    for(positive_asks_t::const_iterator it = phase->positive_asks.begin(); it != phase->positive_asks.end(); it++){
      node_sptr negated_node(new Not((*it)->get_guard()));
      int id = (*it)->get_id();
      ask_map[id] = *it;
      dc_causes.push_back(dc_cause_t(negated_node, id) );
    }
    //現在導出されていないガード条件を離散変化条件として追加
    for(negative_asks_t::const_iterator it = phase->negative_asks.begin(); it != phase->negative_asks.end(); it++){
      node_sptr node((*it)->get_guard() );
      int id = (*it)->get_id();
      ask_map[id] = *it;
      dc_causes.push_back(dc_cause_t(node, id));
    }

    //assertionの否定を追加
    if(opts_->assertion){
      node_sptr assert_node(new Not(opts_->assertion));
      dc_causes.push_back(dc_cause_t(assert_node, -2));
    }
    if(break_condition_.get() != NULL)
    {
      dc_causes.push_back(dc_cause_t(break_condition_, -3));
    }

    value_t max_time;
    if(opts_->max_time != ""){
      max_time = node_sptr(new hydla::parse_tree::Number(opts_->max_time));
    }else{
      max_time = node_sptr(new hydla::parse_tree::Infinity());
    }

    pp_time_result_t time_result; 
    value_t time_limit(max_time);
    time_limit -= phase->current_time;
    backend_->call("calculateNextPointPhaseTime", 2, "vltdc", "cp", &time_limit, &dc_causes, &time_result);

    unsigned int time_it = 0;
    result_list_t results;
    phase_result_sptr_t pr = next_todo->parent;


    // まずインタラクティブ実行のために最小限の情報だけ整理する
    while(true)
    {
      NextPhaseResult &candidate = time_result[time_it];
      node_sptr time_node = candidate.minimum.time.get_node();

      // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
      for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
        pr->parameter_map[it->first] = it->second;
      }

      time_node = node_sptr(new Plus(time_node, current_todo->current_time.get_node()));
      backend_->call("simplify", 1, "et", "vl", &time_node, &pr->end_time);
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

      NextPhaseResult &candidate = time_result[result_it];
      HYDLA_LOGGER_DEBUG_VAR(result_it);
      for(uint id_it = 0; id_it < candidate.minimum.ids.size(); id_it++)
      { 
        int id = candidate.minimum.ids[id_it];
        if(id == -1) {
          pr->cause_of_termination = simulator::TIME_LIMIT;
        }
        else if(id >= 0)
        {
          HYDLA_LOGGER_DEBUG_VAR(id);
          HYDLA_LOGGER_DEBUG_VAR(*ask_map[id]);
          next_todo->discrete_causes.insert(ask_map[id]);
        }
      }


      HYDLA_LOGGER_DEBUG("%%time: ", pr->current_time);
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
  
  return ret;
}

void SymbolicPhaseSimulator::replace_prev2parameter(
  phase_result_sptr_t& state,
  variable_map_t& vm,
  parameter_map_t &parameter_map)
{
  PrevReplacer replacer(parameter_map, state, *simulator_, opts_->approx);
   
  for(variable_map_t::iterator it = vm.begin();
      it != vm.end(); it++)
  {
    HYDLA_LOGGER_DEBUG(it->first, it->second);
    ValueRange& range = it->second;
    value_t val;
    if(range.unique())
    {
      val = range.get_unique();
      HYDLA_LOGGER_DEBUG(val);
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


variable_map_t SymbolicPhaseSimulator::apply_time_to_vm(const variable_map_t& vm, const value_t& tm)
{
  HYDLA_LOGGER_DEBUG("%% time: ", tm);
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
  return result;
}

} //namespace symbolic
} //namespace simulator
} //namespace hydla
