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

#include "Exceptions.h"
#include "TreeInfixPrinter.h"

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
using hydla::parse_tree::TreeInfixPrinter;

namespace hydla {
namespace symbolic_simulator {

SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
  simulator_t(opts)
{
}

SymbolicSimulator::~SymbolicSimulator()
{
}

void SymbolicSimulator::initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c, const module_set_container_sptr& msc)
{
  simulator_t::initialize(v, p, m, c, msc);
  variable_derivative_map_ = c;
  solver_.reset(new MathematicaVCS(*opts_));
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);
}


void SymbolicSimulator::set_simulation_mode(const Phase& phase)
{
  current_phase_ = phase;
}

parameter_set_t SymbolicSimulator::get_parameter_set(){
  return *parameter_set_;
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

SymbolicSimulator::FalseConditionsResult SymbolicSimulator::find_false_conditions(const module_set_sptr& ms){

  HYDLA_LOGGER_MS("#*** SymbolicSimulator::find_false_conditions ***");
  HYDLA_LOGGER_MS(ms->get_name());

  solver_->change_mode(FalseConditionsMode, opts_->approx_precision);

  SymbolicSimulator::FalseConditionsResult ret = FALSE_CONDITIONS_FALSE;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  expanded_always_t expanded_always;
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  tells_t         tell_list;
  constraints_t   constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  negative_asks_t tmp_negative; 

  variable_map_t vm;
  parameter_map_t pm;
  solver_->reset(vm,pm);

  // ask制約を集める 
  ask_collector.collect_ask(&expanded_always, 
      &positive_asks, 
      &tmp_negative, 
      &negative_asks);

  node_sptr condition_node;

  for(int i = 0; i < (1 << negative_asks.size()) && ret != FALSE_CONDITIONS_TRUE; i++){
    positive_asks_t tmp_positive_asks;
    solver_->start_temporary();

    negative_asks_t::iterator it = negative_asks.begin();
    for(int j = 0; j < (int)negative_asks.size(); j++, it++){
      if((i & (1 << j)) != 0){
        solver_->add_guard((*it)->get_guard());
        tmp_positive_asks.insert(*it);
      }else{
        solver_->add_guard(node_sptr(new Not((*it)->get_guard())));
      }
    }

    tell_collector.collect_all_tells(&tell_list,
        &expanded_always, 
        &tmp_positive_asks);

    maker.reset();
    constraint_list.clear();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    solver_->add_constraint(constraint_list);
    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map);

    {
      node_sptr tmp_node;
      switch(solver_->find_false_conditions(tmp_node)){
        case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_TRUE:
          // この制約モジュール集合は必ず矛盾
          ret = FALSE_CONDITIONS_TRUE;
          break;
        case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_FALSE:
          break;
        case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_VARIABLE_CONDITIONS:
          // この制約モジュール集合は矛盾する場合がある
          if(condition_node == NULL) condition_node = node_sptr(tmp_node);
          else condition_node = node_sptr(new LogicalOr(condition_node, tmp_node));
          ret = FALSE_CONDITIONS_VARIABLE_CONDITIONS;
          break;
        default:
          assert(0);
          break;
      }
    }
    solver_->end_temporary();
  }
  if(ret == FALSE_CONDITIONS_VARIABLE_CONDITIONS){
    solver_->node_simplify(condition_node);
    if(condition_node == NULL){
      // 条件が出てきたが、簡約したら実は TRUE だった場合
      false_conditions_.erase(ms);
      if(opts_->optimization_level >= 3){
        false_map_t::iterator it = false_conditions_.begin();
        while(it != false_conditions_.end()){
          if((*it).first->is_super_set(*ms)){
            false_conditions_.erase(it++);
          }else{
            it++;
          }
        }
      }
      ret = FALSE_CONDITIONS_TRUE;
    }else{
      false_conditions_[ms] = condition_node;
      false_map_t::iterator it = false_conditions_.begin();
      while(it != false_conditions_.end() && opts_->optimization_level >= 3){
        if((*it).first->is_super_set(*ms) && (*it).first != ms){
          node_sptr tmp = (*it).second;
          if(tmp != NULL) tmp = node_sptr(new LogicalOr(tmp,condition_node));
          else tmp = node_sptr(condition_node);
          switch(solver_->node_simplify(tmp)){
            case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_TRUE:
              // この制約モジュール集合は必ず矛盾
              false_conditions_.erase(it++);
              break;
            case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_FALSE:
              it++;
              break;
            case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_VARIABLE_CONDITIONS:
              // この制約モジュール集合は矛盾する場合がある
              (*it).second = tmp;
              it++;
              break;
            default:
              assert(0);
              break;
          }
        }else{
          it++;
        }
      }
      HYDLA_LOGGER_MS("found false conditions :", TreeInfixPrinter().get_infix_string(condition_node));
    }
  }else if(ret == FALSE_CONDITIONS_TRUE){
    false_conditions_.erase(ms);
    if(opts_->optimization_level >= 3){
      false_map_t::iterator it = false_conditions_.begin();
      while(it != false_conditions_.end()){
        if((*it).first->is_super_set(*ms)){
          false_conditions_.erase(it++);
        }else{
          it++;
        }
      }
    }
  }else{
    false_conditions_[ms] = node_sptr();
    HYDLA_LOGGER_MS("not found false conditions");
  }
  HYDLA_LOGGER_MS("#*** end SymbolicSimulator::find_false_conditions ***");
  return ret;
}

SymbolicSimulator::CheckEntailmentResult SymbolicSimulator::check_entailment(
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

bool SymbolicSimulator::calculate_closure(simulation_todo_sptr_t& state,
    const module_set_sptr& ms)
{
  HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::calculate_closure ***\n");

  //前準備
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
        // 必ず矛盾する場合
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return false;
      }else if (check_consistency_result.false_parameter_maps.empty()){
        // 必ず充足可能な場合
        // 何もしない
      }else{
        // 記号定数の条件によって充足可能性が変化する場合
        HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
        push_branch_states(state, check_consistency_result);
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
        if(state->phase == PointPhase){
          if(state->current_time->get_string() == "0" && PrevSearcher().search_prev((*it)->get_guard())){
            // 時刻0では左極限値に関する条件を常に偽とする
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            continue;
          }else if(prev_guards_.find(*it) != prev_guards_.end() && 
            state->judged_prev_map.find(*it) != state->judged_prev_map.end())
          {
            // 判定済みのprev条件だった場合
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
            //eraseと後置インクリメントは同時にやらないとイテレータが壊れるので，注意
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
      todo_container_->push_back(new_todo);
    }
    {
      // 分岐先を生成（導出されない方）
      state->temporary_constraints.push_back(node_sptr(new Not((branched_ask)->get_guard())));
      negative_asks.insert(branched_ask);
      return calculate_closure(state, ms);
    }
  }
  
  HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::calculate_closure ***\n");
  return true;
}

SymbolicSimulator::CalculateVariableMapResult 
SymbolicSimulator::calculate_variable_map(
  const module_set_sptr& ms,
  simulation_todo_sptr_t& todo,
  const variable_map_t & vm,
  variable_range_map_t& result_vm)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::calculate_variable_map***");
  //前準備
  
  if(current_phase_ == PointPhase){
    if(opts_->optimization_level >= 2 && todo->current_time->get_string() != "0"){
      if(opts_->optimization_level == 2){
        if(false_conditions_.find(ms) == false_conditions_.end()){
          return CVM_INCONSISTENT;
        }
        if(checkd_module_set_.find(ms) == checkd_module_set_.end()){
          checkd_module_set_.insert(ms);
          if(find_false_conditions(ms) == FALSE_CONDITIONS_TRUE){
            return CVM_INCONSISTENT;
          }
        }
      }
      solver_->reset(vm, todo->parameter_map);
      if(false_conditions_[ms] != NULL){
        solver_->change_mode(FalseConditionsMode, opts_->approx_precision);
        HYDLA_LOGGER_CLOSURE("#*** check_false_conditions***");
        HYDLA_LOGGER_CLOSURE(ms->get_name() , " : " , TreeInfixPrinter().get_infix_string(false_conditions_[ms]));
      
        solver_->set_false_conditions(false_conditions_[ms]);
        
        CheckConsistencyResult check_consistency_result = solver_->check_consistency();
        if(check_consistency_result.true_parameter_maps.empty()){
          // 必ず矛盾する場合
          return CVM_INCONSISTENT;
        }else if (check_consistency_result.false_parameter_maps.empty()){
          // 必ず充足可能な場合
          // 何もしない
        }else{
          // 記号定数の条件によって充足可能性が変化する場合
          push_branch_states(todo, check_consistency_result);
        }
      }
    }
    solver_->change_mode(DiscreteMode, opts_->approx_precision);
  }
  else
  {
    solver_->change_mode(ContinuousMode, opts_->approx_precision);
  }
  solver_->reset(vm, todo->parameter_map); //TODO: 左極限値と記号定数の初期化処理はもっと上でやった方が無駄が少ないはず

  timer::Timer cc_timer;
  
  //閉包計算
  bool result = calculate_closure(todo, ms);
  
  todo->profile["CalculateClosure"] += cc_timer.get_elapsed_us();

  if(!result)
  {
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::calculate_variable_map(result.size() ==0)***\n");
    return CVM_INCONSISTENT;
  }
  
  timer::Timer create_timer;
  SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
  todo->profile["CreateMap"] += create_timer.get_elapsed_us();
  SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;
  
  if(results.size() != 1){
    // TODO:現状，ここで変数表が複数現れる場合は考えていない．
    assert(current_phase_ != PointPhase);
    todo->parent->cause_of_termination = simulator::NOT_UNIQUE_IN_INTERVAL;
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::calculate_variable_map(result.size() != 1 && consistent = true)***\n");
    return CVM_ERROR;
  }
  
  assert(results.size()>0);
  
  result_vm = results[0];

  return CVM_CONSISTENT;
}

SymbolicSimulator::todo_list_t 
  SymbolicSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::make_next_todo***");
  
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
    assert(curent_phase_ == IntervalPhase);
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
    phase_result_sptr_t pr = next_todo->parent;
    while(true)
    {
      SymbolicVirtualConstraintSolver::PPTimeResult::NextPhaseResult &candidate = time_result.candidates[time_it];
      solver_->simplify(candidate.time);
      
      // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
      for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
        next_todo->parameter_map[it->first] = it->second;
      }
      
      pr->end_time = candidate.time;
      pr->parameter_map = next_todo->parameter_map;
      
      if(!candidate.is_max_time ) {
        next_todo->current_time = candidate.time;
        ret.push_back(next_todo);
      }else{
        pr->cause_of_termination = simulator::TIME_LIMIT;
      }
      
      if(++time_it >= time_result.candidates.size())break;
      next_todo = create_new_simulation_phase(next_todo);
      next_todo->parent = make_new_phase(pr);
      pr = next_todo->parent;
    }
    current_todo->profile["NextPP"] += next_pp_timer.get_elapsed_us();
  }
  HYDLA_LOGGER_MS("#*** End SymbolicSimulator::make_next_todo***");
  
  return ret;
}

variable_map_t SymbolicSimulator::range_map_to_value_map(
  phase_result_sptr_t& state,
  const variable_range_map_t& rm,
  parameter_map_t &parameter_map)
{
  variable_map_t ret = *variable_map_;
  for(variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
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
      // TODO:ここで，Parameter(variable->get_name(), variable->get_derivative_count(), state->id)))はPhaseResult自体を参照していないので，
      // もしPhaseResultをこの処理以降に変更するような実装に変更した場合，整合性が取れなくなるのでどうにかする
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
