#include "SymbolicSimulator.h"

#include <iostream>
#include <fstream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"
#include "Timer.h"

//仮追加
#include "RTreeVisitor.h"
#include "TellCollector.h"
#include "AskCollector.h"

#include "InitNodeRemover.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
//#include "../virtual_constraint_solver/reduce/REDUCEVCS.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"

#include "SimulateError.h"

#include "AnalysisResultChecker.h"

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;

//using namespace hydla::vcs::reduce;

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;
using namespace hydla::timer;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;

namespace hydla {
namespace symbolic_simulator {

SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
  simulator_t(opts)
{
}

SymbolicSimulator::~SymbolicSimulator()
{
}
 
SymbolicSimulator::CalculateVariableMapResult
SymbolicSimulator::check_false_conditions
(const module_set_sptr& ms, simulation_phase_sptr_t& state, const variable_map_t& vm, variable_map_t& result_vm, todo_and_results_t& result_todo){
  return analysis_result_checker_->check_false_conditions(ms,state,vm,result_vm,result_todo);
}

void SymbolicSimulator::initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, const module_set_sptr& ms, continuity_map_t& c)
{
  simulator_t::initialize(v, p, m, ms, c);
  variable_derivative_map_ = c;
  solver_.reset(new MathematicaVCS(*opts_));
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);

  if(opts_->analysis_mode == "use" || opts_->analysis_mode == "simulate"){
    analysis_result_checker_.reset(new AnalysisResultChecker(*opts_));
    analysis_result_checker_->set_solver(solver_);
  }
}


void SymbolicSimulator::set_simulation_mode(const Phase& phase)
{
  current_phase_ = phase;
}

parameter_set_t SymbolicSimulator::get_parameter_set(){
  return *parameter_set_;
}

void SymbolicSimulator::push_branch_states(simulation_phase_sptr_t &original, SymbolicVirtualConstraintSolver::check_consistency_result_t &result, CalculateClosureResult &dst){
  for(int i=0; i<(int)result.true_parameter_maps.size();i++){
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->phase_result->parameter_map = result.true_parameter_maps[i];
    dst.push_back(branch_state);
  }
  for(int i=0; i<(int)result.false_parameter_maps.size();i++){
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->phase_result->parameter_map = result.false_parameter_maps[i];
    dst.push_back(branch_state);
  }
}


void SymbolicSimulator::add_continuity(const continuity_map_t& continuity_map){
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

SymbolicSimulator::CheckEntailmentResult SymbolicSimulator::check_entailment(
  SymbolicVirtualConstraintSolver::check_consistency_result_t &cc_result,
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
    }else
    {
      solver_->end_temporary();
      solver_->start_temporary();
      add_continuity(cont_map);
      solver_->add_guard(node_sptr(new Not(guard)));
      cc_result = solver_->check_consistency();
      if(!cc_result.true_parameter_maps.empty()){
        HYDLA_LOGGER_CLOSURE("%% entailment branches");
        if(!cc_result.false_parameter_maps.empty()){
          HYDLA_LOGGER_CLOSURE("%% inevitable entailment depends on conditions of parameters");
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

CalculateClosureResult SymbolicSimulator::calculate_closure(simulation_phase_sptr_t& state,
    const module_set_sptr& ms)
{    
  HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::calculate_closure ***\n");

  phase_result_sptr_t &pr = state->phase_result;
  //前準備
  positive_asks_t& positive_asks = pr->positive_asks;
  negative_asks_t& negative_asks = pr->negative_asks;
  ask_set_t unknown_asks;
  expanded_always_t& expanded_always = pr->expanded_always;
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  tells_t         tell_list;
  constraints_t   constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;

  bool expanded;
  
  do{
    // tell制約を集める
    tell_collector.collect_new_tells(&tell_list,
        &expanded_always,
        &positive_asks);

    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_consistency in calculate_closure\n");
    timer::Timer consistency_timer;
    //tellじゃなくて制約部分のみ送る
    constraint_list.clear();

    maker.reset();

    // 制約を追加し，制約ストアが矛盾をおこしていないかどうか調べる
    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), pr->phase == IntervalPhase, false);
    }

    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map);

    for(constraints_t::const_iterator it = state->temporary_constraints.begin(); it != state->temporary_constraints.end(); it++){
      constraint_list.push_back(*it);
    }

    solver_->add_constraint(constraint_list);

    {
      SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
      if(check_consistency_result.true_parameter_maps.empty()){
        // 必ず矛盾する場合
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return CalculateClosureResult();
      }else if (check_consistency_result.false_parameter_maps.empty()){
        // 必ず充足可能な場合
        // 何もしない
      }else{
        // 記号定数の条件によって充足可能性が変化する場合
        HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
        CalculateClosureResult result;
        push_branch_states(state, check_consistency_result, result);
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return result;
      }
    }
    
    state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();


    // ask制約のエンテール処理
    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_entailment in calculate_closure\n");
    
    // ask制約を集める
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
        if(pr->phase == PointPhase){
          if(pr->current_time->get_string() == "0" && PrevSearcher().search_prev((*it)->get_guard())){
            // 時刻0では左極限値に関する条件を常に偽とする
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            continue;
          }else if(prev_guards_.find(*it) != prev_guards_.end() && 
            judged_prev_map_.find(*it) != judged_prev_map_.end())
          {
            // 判定済みのprev条件だった場合
            bool entailed = judged_prev_map_.find(*it)->second;
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
        maker.visit_node((*it)->get_child(), pr->phase == IntervalPhase, true);
        
        SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result;
        switch(check_entailment(check_consistency_result, (*it)->get_guard(), maker.get_continuity_map())){
          case ENTAILED:
            HYDLA_LOGGER_CLOSURE("--- entailed ask ---\n", *((*it)->get_guard()));
            positive_asks.insert(*it);
            if(prev_guards_.find(*it) != prev_guards_.end()){
              judged_prev_map_.insert(std::make_pair(*it, true));
            }
            //eraseと後置インクリメントは同時にやらないとイテレータが壊れるので，注意
            unknown_asks.erase(it++);
            expanded = true;
            break;
          case CONFLICTING:
            HYDLA_LOGGER_CLOSURE("--- conflicted ask ---\n", *((*it)->get_guard()));
            negative_asks.insert(*it);
            if(prev_guards_.find(*it) != prev_guards_.end()){
              judged_prev_map_.insert(std::make_pair(*it, false));
            }
            unknown_asks.erase(it++);
            break;
          case BRANCH_VAR:
            HYDLA_LOGGER_CLOSURE("--- branched ask ---\n", *((*it)->get_guard()));
            it++;
            break;
          case BRANCH_PAR:
            HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
            CalculateClosureResult result;
            push_branch_states(state, check_consistency_result, result);
            state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
            return result;
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
    CalculateClosureResult result;
    {
      // 分岐先を生成（導出されない方）
      simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
      new_state->temporary_constraints.push_back(node_sptr(new Not((branched_ask)->get_guard())));
      result.push_back(new_state);
    }
    {
      // 分岐先を生成（導出される方）
      simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
      new_state->temporary_constraints.push_back((branched_ask)->get_guard());
      result.push_back(new_state);
    }
    return result;
  }

  if(opts_->assertion){
    timer::Timer entailment_timer;
    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_assertion");
    SymbolicVirtualConstraintSolver::check_consistency_result_t cc_result;
    CalculateClosureResult ret;
    switch(check_entailment(cc_result, node_sptr(new Not(opts_->assertion)), continuity_map_t())){
      case ENTAILED:
        std::cout << "Assertion Failed!" << std::endl;
        HYDLA_LOGGER_CLOSURE("%% Assertion Failed!");
        is_safe_ = false;
        ret.push_back(state);
        break;
      case CONFLICTING:
        break;
      case BRANCH_VAR:
        HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of variables");
          // 分岐先を生成
        {
          simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
          new_state->temporary_constraints.push_back(opts_->assertion);
          ret.push_back(new_state);
        }
        {
          simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
          new_state->temporary_constraints.push_back(node_sptr(new Not(opts_->assertion)));
          ret.push_back(new_state);
        }
        break;
      case BRANCH_PAR:
        HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of parameters");
        push_branch_states(state, cc_result, ret);
        break;
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    if(!ret.empty())
      return ret;
  }
  HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::calculate_closure ***\n");
  return CalculateClosureResult(1, state);
}

SymbolicSimulator::CalculateVariableMapResult 
SymbolicSimulator::calculate_variable_map(
  const module_set_sptr& ms,
  simulation_phase_sptr_t& state,
  const variable_map_t & vm,
  variable_map_t& result_vm,
  todo_and_results_t& result_todo)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::calculate_variable_map***");
  //前準備
  phase_result_sptr_t& pr = state->phase_result;
  
  if(current_phase_ == PointPhase){
    solver_->change_mode(DiscreteMode, opts_->approx_precision);
  }
  else
  {
    solver_->change_mode(ContinuousMode, opts_->approx_precision);
  }
  solver_->reset(vm, pr->parameter_map);

  timer::Timer cc_timer;
  
  //閉包計算
  CalculateClosureResult result = calculate_closure(state, ms);
  
  state->profile["CalculateClosure"] += cc_timer.get_elapsed_us();

  if(result.size() == 0)
  {
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::calculate_variable_map(result.size() ==0)***\n");
    return CVM_INCONSISTENT;
  }
  else if(result.size() > 1){
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::calculate_variable_map(result.size() != 1 : ",result.size() ,")***\n");
    for(unsigned int i=0;i<result.size();i++){
      result_todo.push_back(PhaseSimulator::TodoAndResult(result[i], phase_result_sptr_t()));
    }
    return CVM_BRANCH;
  }
  
  
  timer::Timer create_timer;
  SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
  state->profile["CreateMap"] += create_timer.get_elapsed_us();
  SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;
  
  if(results.size() != 1){
    // TODO:現状，ここで変数表が複数現れる場合は考えていない．
    assert(current_phase_ != PointPhase);
    state->phase_result->cause_of_termination = simulator::NOT_UNIQUE_IN_INTERVAL;
    result_todo.push_back(PhaseSimulator::TodoAndResult(simulation_phase_sptr_t(), state->phase_result));
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::calculate_variable_map(result.size() != 1 && consistent = true)***\n");
    return CVM_ERROR;
  }
  
  assert(results.size()>0);
  result_vm = range_map_to_value_map(pr, results[0], pr->parameter_map);

  return CVM_CONSISTENT;
}

SymbolicSimulator::todo_and_results_t 
  SymbolicSimulator::make_next_todo(const module_set_sptr& ms, simulation_phase_sptr_t& state, variable_map_t &vm)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::make_next_todo***");
  
  solver_->reset_constraint(vm);
  
  phase_result_sptr_t& pr = state->phase_result;
  todo_and_results_t phases;
  
  simulation_phase_sptr_t new_state_original(create_new_simulation_phase());
  phase_result_sptr_t& new_pr_original = new_state_original->phase_result;
  new_pr_original->step         = pr->step+1;
  new_pr_original->expanded_always = pr->expanded_always;
  new_pr_original->parameter_map = pr->parameter_map;
  new_pr_original->parent = pr;

  pr->module_set         = ms;
  
  if(current_phase_ == PointPhase)
  {
    new_pr_original->phase        = IntervalPhase;
    new_pr_original->current_time = pr->current_time;
    pr->variable_map = vm;
    PhaseSimulator::TodoAndResult tr;
    tr.result = pr;
    if(is_safe_){
      tr.todo = new_state_original;
    }else{
      pr->cause_of_termination = simulator::ASSERTION;
    }
    phases.push_back(tr);
  }
  else
  {
    new_pr_original->phase        = PointPhase;
    pr->variable_map = shift_variable_map_time(vm, pr->current_time);
    
    if(is_safe_){
      timer::Timer next_pp_timer;
      constraints_t disc_cause;
      //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
      for(positive_asks_t::const_iterator it = pr->positive_asks.begin(); it != pr->positive_asks.end(); it++){
        disc_cause.push_back(node_sptr(new Not((*it)->get_guard() ) ) );
      }
      //現在導出されていないガード条件を離散変化条件として追加
      for(negative_asks_t::const_iterator it = pr->negative_asks.begin(); it != pr->negative_asks.end(); it++){
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
        time_result = solver_->calculate_next_PP_time(disc_cause, pr->current_time, max_time);

      for(unsigned int time_it=0; time_it<time_result.candidates.size(); time_it++){
        simulation_phase_sptr_t branch_state(create_new_simulation_phase(state));
        phase_result_sptr_t &bpr = branch_state->phase_result;
        SymbolicVirtualConstraintSolver::PPTimeResult::NextPhaseResult &candidate = time_result.candidates[time_it];
        
        // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
        for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
          bpr->parameter_map[it->first] = it->second;
        }
        simulation_phase_sptr_t new_state(create_new_simulation_phase(new_state_original));
        phase_result_sptr_t& npr = new_state->phase_result;
        
        PhaseSimulator::TodoAndResult tr;
        
        solver_->simplify(candidate.time);
        bpr->end_time = candidate.time;
        if(!candidate.is_max_time ) {
          npr->current_time = candidate.time;
          npr->parameter_map = bpr->parameter_map;
          npr->parent = bpr;
          tr.todo = new_state;
        }else{
          bpr->cause_of_termination = simulator::TIME_LIMIT;
        }
        tr.result = bpr;
        phases.push_back(tr);
      }
      state->profile["NextPP"] += next_pp_timer.get_elapsed_us();
    }else{
      phases.push_back(PhaseSimulator::TodoAndResult(simulation_phase_sptr_t(), pr));
      pr->cause_of_termination = simulator::ASSERTION;
    }
  }
  HYDLA_LOGGER_MS("#*** End SymbolicSimulator::make_next_todo***");
  
  return phases;
}

variable_map_t SymbolicSimulator::range_map_to_value_map(
  phase_result_sptr_t& state,
  const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t& rm,
  parameter_map_t &parameter_map)
{
  variable_map_t ret = *variable_map_;
  for(vcs::SymbolicVirtualConstraintSolver::variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
    variable_t* variable = get_variable(r_it->first->get_name(), r_it->first->get_derivative_count());
    if(r_it->second.is_unique()){
      ret[variable] = r_it->second.get_lower_bound().value;
    }
    else if(!r_it->second.is_undefined())
    {
      parameter_t param(r_it->first, state);
      parameter_set_->push_front(simulator::ParameterAndRange(param, r_it->second));
      parameter_map[&(parameter_set_->front().parameter)] = r_it->second;
      ret[variable] = value_t(new SymbolicValue(node_sptr(
        new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
      // TODO:記号定数導入後，各変数の数式中に出現する変数を記号定数に置き換える
    }
    else
    {
      // TODO: 変数の値が完全に不明な場合，無視するものとしている
      // ただしここで常に記号定数を導入するようにしておくと，記号定数が増えすぎて見づらくなる可能性がある．
      // 解軌道木上で一度しか出現しないUNDEFに対しては，記号定数を導入する必要が無いはずなので，どうにかそれを実現したい？
    }
  }
  return ret;
}


variable_map_t SymbolicSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
  variable_map_t shifted_vm;
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  for(; it!=end; ++it) {
    if(it->second->is_undefined())
      shifted_vm[it->first] = it->second;
    else
      shifted_vm[it->first] = solver_->shift_expr_time(it->second, time);
  }
  return shifted_vm;
}

} //namespace symbolic_simulator
} //namespace hydla
