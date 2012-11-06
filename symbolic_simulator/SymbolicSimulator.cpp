#include "SymbolicSimulator.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
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

#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
//#include "../virtual_constraint_solver/reduce/REDUCEVCS.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"

#include "SimulateError.h"

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

void SymbolicSimulator::initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c)
{
  simulator_t::initialize(v, p, m, c);
  variable_derivative_map_ = c;

  // 使用するソルバの決定
  /*  if(opts_->solver == "r" || opts_->solver == "Reduce") {
      solver_.reset(new REDUCEVCS(opts_, variable_map_));
      }else{*/
  solver_.reset(new MathematicaVCS(*opts_));
  //}
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);
}
void SymbolicSimulator::set_parameter_set(parameter_t param){
  parameter_set_->push_front(param);
  //std::cout << "ss sps size : " << parameter_set_->size() << std::endl;
  solver_->set_parameter_set(*parameter_set_);
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
  HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::add_continuity ***\n");
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
  HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::add_continuity ***\n");
}


CalculateClosureResult SymbolicSimulator::calculate_closure(simulation_phase_sptr_t& state,
    const module_set_sptr& ms,
    expanded_always_t &expanded_always,
    positive_asks_t &positive_asks,
    negative_asks_t &negative_asks){    
  HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::calculate_closure ***\n");

  //前準備
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  tells_t         tell_list;
  constraints_t   constraint_list;
  boost::shared_ptr<hydla::parse_tree::Ask>  const *branched_ask;
  phase_result_sptr_t &pr = state->phase_result;

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
        return CalculateClosureResult();
      }else if (check_consistency_result.false_parameter_maps.empty()){
        // 必ず充足可能な場合
        // 何もしない
      }else{
        // 記号定数の条件によって充足可能性が変化する場合
        HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
        CalculateClosureResult result;
        push_branch_states(state, check_consistency_result, result);
        return result;
      }
    }
    
    state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always, 
        &positive_asks, 
        &negative_asks);

    // ask制約のエンテール処理
    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_entailment in calculate_closure\n");
    
    timer::Timer entailment_timer;
    
    {
      expanded = false;
      branched_ask=NULL;
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
      
        // 時刻0では左極限値に関する条件を常に偽とする
        if(pr->phase == PointPhase && pr->current_time->get_string() == "0" && PrevSearcher().search_prev((*it)->get_guard())){
          it++;
          continue;
        }
        solver_->start_temporary();
        maker.visit_node((*it)->get_child(), pr->phase == IntervalPhase, true);
        add_continuity(maker.get_continuity_map());
        solver_->add_guard((*it)->get_guard());

        SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
        if(!check_consistency_result.true_parameter_maps.empty()){
          HYDLA_LOGGER_CLOSURE("%% entailable");
          if(!check_consistency_result.false_parameter_maps.empty()){
            HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
            CalculateClosureResult result;
            push_branch_states(state, check_consistency_result, result);
            return result;
          }
          solver_->end_temporary();
          solver_->start_temporary();
          add_continuity(maker.get_continuity_map());
          solver_->add_guard(node_sptr(new Not((*it)->get_guard())));
          check_consistency_result = solver_->check_consistency();
          if(!check_consistency_result.true_parameter_maps.empty()){
            HYDLA_LOGGER_CLOSURE("%% entailment branches");
            // ガード条件による分岐が発生する可能性あり
            if(!check_consistency_result.false_parameter_maps.empty()){
              HYDLA_LOGGER_CLOSURE("%% inevitable entailment depends on conditions of parameters");
              CalculateClosureResult ret;
              push_branch_states(state, check_consistency_result, ret);
              return ret;
            }
            HYDLA_LOGGER_CLOSURE("--- branched ask ---\n", *((*it)->get_guard()));
            branched_ask = &(*it);
            it++;
            solver_->end_temporary();
            continue;
          }
          HYDLA_LOGGER_CLOSURE("--- entailed ask ---\n", *((*it)->get_guard()));
          positive_asks.insert(*it);
          //eraseと後置インクリメントは同時にやらないとイテレータが壊れるので，注意
          negative_asks.erase(it++);
          expanded = true;
        }else{
          it++;
        }

        maker.set_continuity_map(continuity_map);
        solver_->end_temporary();
      }
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
  }while(expanded);

  add_continuity(continuity_map);

  if(branched_ask!=NULL){
    HYDLA_LOGGER_CLOSURE("%% branched_ask:", TreeInfixPrinter().get_infix_string(*branched_ask));
    CalculateClosureResult result;
    {
      // 分岐先を生成（導出されない方）
      simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
      new_state->temporary_constraints.push_back(node_sptr(new Not((*branched_ask)->get_guard())));
      result.push_back(new_state);
    }
    {
      // 分岐先を生成（導出される方）
      simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
      new_state->temporary_constraints.push_back((*branched_ask)->get_guard());
      result.push_back(new_state);
    }
    return result;
  }

  if(opts_->assertion){
    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_assertion");
    solver_->start_temporary();
    solver_->add_constraint(node_sptr(new Not(opts_->assertion)));
    SymbolicVirtualConstraintSolver::check_consistency_result_t result = solver_->check_consistency();
    solver_->end_temporary();
    if(!result.true_parameter_maps.empty()){
      if(!result.false_parameter_maps.empty()){
        // TODO:assertion失敗なので，本来ならやり直す必要はない．trueがassertion failedで，falseはこのまま続行すべき
        // ガード条件による分岐が発生する可能性あり
        HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of parameters");
        CalculateClosureResult ret;
        push_branch_states(state, result, ret);
        return ret;
      }else{
        std::cout << "Assertion Failed!" << std::endl;
        HYDLA_LOGGER_CLOSURE("%% Assertion Failed!");
        is_safe_ = false;
        CalculateClosureResult ret;
        ret.push_back(state);
        return ret;
      }
    }
  }
  HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::calculate_closure ***\n");
  return CalculateClosureResult(1, state);
}

SymbolicSimulator::simulation_phases_t SymbolicSimulator::simulate_ms_point(const module_set_sptr& ms, 
    simulation_phase_sptr_t& state,
    variable_map_t& vm,
    bool& consistent)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::simulate_ms_point***");
  //前準備
  
  phase_result_sptr_t& pr = state->phase_result;

  solver_->change_mode(DiscreteMode, opts_->approx_precision);

  positive_asks_t positive_asks(pr->positive_asks);
  negative_asks_t negative_asks;
  expanded_always_t ea(pr->expanded_always);

  solver_->reset(vm, pr->parameter_map);
  
  timer::Timer cc_timer;

  //閉包計算
  CalculateClosureResult result = calculate_closure(state, ms, ea,positive_asks,negative_asks);
  
  state->profile["CalculateClosure"] += cc_timer.get_elapsed_us();


  if(result.size() != 1){
    consistent = false;
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_point(result.size() != 1)***\n");
    return result;
  }
  
  timer::Timer create_timer;
  // Interval Phaseへ移行（次状態の生成）
  SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
  state->profile["CreateMap"] += create_timer.get_elapsed_us();

  assert(create_result.result_maps.size()>0);
  
  pr->module_set = ms;

  simulation_phase_sptr_t new_state_original(create_new_simulation_phase());
  phase_result_sptr_t& new_pr_original = new_state_original->phase_result;
  new_pr_original->step         = pr->step+1;
  new_pr_original->phase        = IntervalPhase;
  new_pr_original->current_time = pr->current_time;
  new_pr_original->expanded_always = ea;
  new_pr_original->module_set = ms;

  simulation_phases_t phases;

  for(unsigned int create_it = 0; create_it < create_result.result_maps.size() && (opts_->nd_mode||create_it==0); create_it++)
  {
    simulation_phase_sptr_t new_state(create_new_simulation_phase(new_state_original));
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(state));
    phase_result_sptr_t& npr = new_state->phase_result, &bpr = branch_state->phase_result;

    bpr->variable_map = range_map_to_value_map(bpr, create_result.result_maps[create_it], bpr->parameter_map);
    npr->parameter_map = bpr->parameter_map;
    
    bpr->parent->children.push_back(bpr);
    npr->parent = bpr;

    if(is_safe_){
      phases.push_back(new_state);
    }else{
      bpr->cause_of_termination = simulator::ASSERTION;
    }
  }

  HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_point***\n");
  consistent = true;
  return phases;
}

SymbolicSimulator::simulation_phases_t SymbolicSimulator::simulate_ms_interval(const module_set_sptr& ms, 
    simulation_phase_sptr_t& state,
    bool& consistent)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::simulate_ms_interval***");
  //前準備
  phase_result_sptr_t& pr = state->phase_result;
  solver_->change_mode(ContinuousMode, opts_->approx_precision);
  negative_asks_t negative_asks;
  positive_asks_t positive_asks(pr->positive_asks);
  solver_->reset(pr->parent->variable_map, pr->parameter_map);
  expanded_always_t ea(pr->expanded_always);

  timer::Timer cc_timer;

  //閉包計算
  CalculateClosureResult result = calculate_closure(state, ms, ea,positive_asks,negative_asks);

  state->profile["CalculateClosure"] += cc_timer.get_elapsed_us();
  
  if(result.size() != 1){
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_interval(result.size() != 1 && consistent = false)***\n");
    consistent = false;
    return result;
  }

  timer::Timer create_timer;
  SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
  state->profile["CreateMap"] += create_timer.get_elapsed_us();
  SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;

  if(results.size() != 1){
    // 区分的に連続で無い解軌道を含む．中断．
    simulation_phase_sptr_t phase(create_new_simulation_phase(state));
    phase->phase_result->cause_of_termination = simulator::NOT_UNIQUE_IN_INTERVAL;
    phase->phase_result->parent->children.push_back(phase->phase_result);
    consistent = true;
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_interval(result.size() != 1 && consistent = true)***\n");
    return simulation_phases_t();
  }

  simulation_phase_sptr_t new_state_original(create_new_simulation_phase());
  phase_result_sptr_t &npr_original = new_state_original->phase_result;

  npr_original->step         = pr->step+1;
  npr_original->phase        = PointPhase;
  npr_original->expanded_always = ea;
  
  pr->variable_map = range_map_to_value_map(pr, results[0], pr->parameter_map);
  pr->variable_map = shift_variable_map_time(pr->variable_map, pr->current_time);

  pr->module_set = ms;
    
  simulation_phases_t phases;
  
  if(is_safe_){

    /*
    // MaxModuleの導出
    module_set_sptr max_module_set = (*msc_no_init_).get_max_module_set();
    HYDLA_LOGGER_DEBUG("#** interval_phase: ms: **\n",
     *ms,
     "\n#** interval_phase: max_module_set: ##\n",
     *max_module_set);


    // 採用していないモジュールのリスト導出
    hydla::ch::ModuleSet::module_list_t diff_module_list(max_module_set->size() - ms->size());

    std::set_difference(
    max_module_set->begin(),
    max_module_set->end(),
    ms->begin(),
    ms->end(),
    diff_module_list.begin());


    // それぞれのモジュールをsingletonなモジュール集合とする
    std::vector<module_set_sptr> diff_module_set_list;

    hydla::ch::ModuleSet::module_list_const_iterator diff_it = diff_module_list.begin();
    hydla::ch::ModuleSet::module_list_const_iterator diff_end = diff_module_list.end();
    for(; diff_it!=diff_end; ++diff_it){
    module_set_sptr diff_ms(new ModuleSet((*diff_it).first, (*diff_it).second));
    diff_module_set_list.push_back(diff_ms);
    }

    assert(diff_module_list.size() == diff_module_set_list.size());


    // diff_module_set_list内の各モジュール集合内にある条件なし制約をそれぞれ得る
    not_adopted_tells_list_t not_adopted_tells_list;

    std::vector<module_set_sptr>::const_iterator diff_ms_list_it = diff_module_set_list.begin();
    std::vector<module_set_sptr>::const_iterator diff_ms_list_end = diff_module_set_list.end();
    for(; diff_ms_list_it!=diff_ms_list_end; ++diff_ms_list_it){
    TellCollector not_adopted_tells_collector(*diff_ms_list_it);
    tells_t       not_adopted_tells;
    not_adopted_tells_collector.collect_all_tells(&not_adopted_tells,
    &expanded_always, 
    &positive_asks);
    not_adopted_tells_list.push_back(not_adopted_tells);
    }


    //現在採用されていない制約を離散変化条件として追加
    for(not_adopted_tells_list_t::const_iterator it = not_adopted_tells_list.begin(); it != not_adopted_tells_list.end(); it++){
    tells_t::const_iterator na_it = it -> begin();
    tells_t::const_iterator na_end = it -> end();
    for(; na_it != na_end; na_it++){
    disc_cause.push_back((*na_it)->get_child());
    }
    }
    */
    timer::Timer next_pp_timer;
    constraints_t disc_cause;
    //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
    for(positive_asks_t::const_iterator it = positive_asks.begin(); it != positive_asks.end(); it++){
      disc_cause.push_back(node_sptr(new Not((*it)->get_guard() ) ) );
    }
    //現在導出されていないガード条件を離散変化条件として追加
    for(negative_asks_t::const_iterator it = negative_asks.begin(); it != negative_asks.end(); it++){
      disc_cause.push_back((*it)->get_guard());
    }

    //assertionの否定を追加
    if(opts_->assertion){
      disc_cause.push_back(node_sptr(new Not(opts_->assertion)));
    }
    
    time_t max_time(new SymbolicValue(node_sptr(new hydla::parse_tree::Number(opts_->max_time))));

    SymbolicVirtualConstraintSolver::PPTimeResult 
      time_result = solver_->calculate_next_PP_time(disc_cause, pr->current_time, max_time);

    for(unsigned int time_it=0; time_it<time_result.candidates.size() && (opts_->nd_mode||time_it==0); time_it++){
      simulation_phase_sptr_t branch_state(create_new_simulation_phase(state));
      phase_result_sptr_t &bpr = branch_state->phase_result;
      SymbolicVirtualConstraintSolver::PPTimeResult::NextPhaseResult &candidate = time_result.candidates[time_it];
      
      // 直接代入すると，値の上限も下限もない記号定数についての枠が無くなってしまうので，追加のみを行う．
      for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
        bpr->parameter_map[it->first] = it->second;
      }
      bpr->parent->children.push_back(bpr);
      simulation_phase_sptr_t new_state(create_new_simulation_phase(new_state_original));
      phase_result_sptr_t& npr = new_state->phase_result;
      
      if(!candidate.is_max_time ) {
        bpr->end_time = candidate.time;
        npr->current_time = candidate.time;
        solver_->simplify(npr->current_time);
        npr->parameter_map = bpr->parameter_map;
        npr->parent = bpr;
        phases.push_back(new_state);
      }else{
        bpr->cause_of_termination = simulator::TIME_LIMIT;
        bpr->end_time = max_time;
      }
    }
    state->profile["NextPP"] += next_pp_timer.get_elapsed_us();
  }else{
    pr->parent->children.push_back(pr);
    pr->cause_of_termination = simulator::ASSERTION;
  }

  HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_interval***");
  consistent = true;
  return phases;
}

SymbolicSimulator::variable_map_t SymbolicSimulator::range_map_to_value_map(
  phase_result_sptr_t& state,
  const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t& rm,
  parameter_map_t &parameter_map){
variable_map_t ret = *variable_map_;
for(vcs::SymbolicVirtualConstraintSolver::variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
  variable_t* variable = get_variable(r_it->first->get_name(), r_it->first->get_derivative_count());
  if(r_it->second.is_unique()){
    ret[variable] = r_it->second.get_lower_bound().value;
  }else{
    parameter_t param(r_it->first, state);
    parameter_set_->push_front(param);
    parameter_map[&(parameter_set_->front())] = r_it->second;
    ret[variable] = value_t(new SymbolicValue(node_sptr(
      new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
    // TODO:記号定数導入後，各変数の数式に出現する変数を記号定数に置き換える
  }
}
return ret;
}


SymbolicSimulator::variable_map_t SymbolicSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
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
