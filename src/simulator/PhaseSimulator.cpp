#include <iostream>
#include <fstream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "PhaseSimulator.h"
#include "Exceptions.h"
#include "Backend.h"

#include "Logger.h"
#include "Timer.h"

#include "PrevReplacer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "VariableFinder.h"

#include "InitNodeRemover.h"
#include "MathematicaLink.h"
#include "REDUCELinkFactory.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"

#include "Backend.h"
#include "Exceptions.h"
#include "AnalysisResultChecker.h"
#include "UnsatCoreFinder.h"
#include "AlwaysFinder.h"
#include "EpsilonMode.h"


using namespace std;
using namespace boost;
using namespace hydla::simulator;
using namespace hydla::backend;

using namespace hydla::backend::mathematica;
using namespace hydla::backend::reduce;
using namespace hydla::hierarchy;
using namespace hydla::symbolic_expression;
using namespace hydla::logger;
using namespace hydla::timer;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;
using hydla::simulator::VariableFinder;


PhaseSimulator::PhaseSimulator(Simulator* simulator,const Opts& opts): breaking(false), simulator_(simulator), opts_(&opts), select_phase_(NULL), break_condition_(symbolic_expression::node_sptr()), unsat_core_finder_(new UnsatCoreFinder()) {
}

PhaseSimulator::~PhaseSimulator(){}

PhaseSimulator::result_list_t PhaseSimulator::calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont)
{
  HYDLA_LOGGER_DEBUG("%% current time:", todo->current_time);
  timer::Timer phase_timer;
  result_list_t result;

  todo->module_set_container->reset(todo->ms_to_visit);

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
      tmp_todo->module_set_container->reset(tmp_todo->ms_to_visit);
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
  if(todo->phase_type == PointPhase)
  {
    set_simulation_mode(PointPhase);
  }else{
    set_simulation_mode(IntervalPhase);
  }

 while(todo->module_set_container->go_next())
  {
    module_set_sptr ms = todo->module_set_container->get_module_set();

    std::string module_sim_string = "\"ModuleSet" + ms->get_name() + "\"";
    //aho
    if(opts_->epsilon_mode>0 && false ){
      // todo
      // PhaseType                 phase_type;
      // int                       id;
      // value_t                   current_time;
      // /// 左極限値のマップ
      // variable_map_t            prev_map;
      // parameter_map_t           parameter_map;
      // positive_asks_t           positive_asks;
      // negative_asks_t           negative_asks;
      // ask_set_t                 discrete_causes;
      // always_set_t              expanded_always;
      // entailed_prev_map_t       judged_prev_map;
      // /// 前のフェーズ
      // phase_result_sptr_t parent;
      // /// このフェーズの制約ストアの初期値（離散変化条件など，特に重要な条件を入れる）
      // ConstraintStore initial_constraint_store;
      // /// 使用する制約モジュール集合．（フェーズごとに，非always制約を含むか否かの差がある）
      // module_set_container_sptr module_set_container;
      // /// 未判定のモジュール集合を保持しておく．分岐処理時，同じ集合を複数回調べることが無いように
      // /// TODO:現状，これがまともに使われていない気がする．つまり，何か間違っている可能性があるし，無駄は確実にある
      // module_set_list_t ms_to_visit;
      // /// 無矛盾極大なモジュール集合の集合
      // module_set_list_t maximal_mss;
      // /// プロファイリング結果
      // profile_t profile;
      // /// 所属するケースの計算時間
      // int elapsed_time;
      // /// map to cache result of calculation for each module_set
      // ms_cache_t ms_cache;
      // /// changing variables from previous phase
      // change_variables_t changing_variables;

      std::cout << "todo id " <<  todo->id << std::endl;
      std::cout << "parent phase " << todo->parent->id << std::endl;
      std::cout << "current time " <<  todo->current_time << std::endl;
      if(todo->phase_type == PointPhase)
        std::cout << "phase type " <<  todo->phase_type << ": PointPhase" << std::endl;
      else
        std::cout << "phase type " <<  todo->phase_type << ": IntervalPhase" << std::endl;
      std::cout << "module set == " << ms->get_name() << std::endl;

      std::cout << "parameter map" << std::endl;
      for(parameter_map_t::iterator p_it = todo->parameter_map.begin(); p_it != todo->parameter_map.end(); p_it++)
        {
          std::cout << "\t " << p_it->first << "\t: " << p_it->second << std::endl;
        }

      std::cout << "variable map" << std::endl;
      for(variable_map_t::iterator v_it = todo->prev_map.begin(); v_it != todo->prev_map.end(); v_it++)
        {
          std::cout << "\t " << v_it->first << "\t: " << v_it->second << std::endl;
        }
      std::cout << "judged_prev_map \t: " <<  std::endl;
      for(entailed_prev_map_t::iterator it = todo->judged_prev_map.begin(); it != todo->judged_prev_map.end();it++){
        std::cout << "\t " << *(*it).first << "\t: ";
        if((*it).second) std::cout << "true" << std::endl;
        else std::cout << "false" << std::endl;
      }
      std::cout << std::endl;
    }

    timer::Timer ms_timer;
    result_list_t tmp_result = simulate_ms(ms, todo->prev_map, todo);
    if(!tmp_result.empty())
    {
      has_next = true;
      result.insert(result.begin(), tmp_result.begin(), tmp_result.end());
    }
    todo->profile[module_sim_string] += ms_timer.get_elapsed_us();
    todo->positive_asks.clear();
    todo->negative_asks.clear();
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



PhaseSimulator::result_list_t PhaseSimulator::simulate_ms(const hydla::hierarchy::module_set_sptr& ms, const variable_map_t &time_applied_map, simulation_todo_sptr_t& todo)
{
  HYDLA_LOGGER_DEBUG("--- next module set ---\n", ms->get_infix_string());
  relation_graph_->set_valid(ms.get());
  result_list_t result;
  // TODO:変数の値による分岐も無視している？
  ConstraintStore store;

  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &todo->parameter_map);
  backend_->call("addPrevConstraint", 1, "mvp", "", &time_applied_map);

  simulation_todo_sptr_t tes = todo;

  if(todo->module_set_container != msc_no_init_)
  {
    ConstraintStore sub_store =
      calculate_constraint_store(ms, todo);
    if(sub_store.consistent())
    {
      HYDLA_LOGGER_DEBUG("CONSISTENT");
      store.add_constraint_store(sub_store);
    }
    else
    {
      HYDLA_LOGGER_DEBUG("INCONSISTENT");
      todo->module_set_container->mark_nodes(todo->maximal_mss, *ms);
      return result;
    }
  }
  else
  {
    int connected_count = relation_graph_->get_connected_count();
    for(int i = 0; i < connected_count; i++){
      module_set_sptr connected_ms = relation_graph_->get_component(i);
      HYDLA_LOGGER_DEBUG("\n--- connected module set", i + 1, "/", connected_count, " ---\n", connected_ms->get_infix_string());
      HYDLA_LOGGER_DEBUG_VAR(store);
      SimulationTodo::ms_cache_t::iterator ms_it = todo->ms_cache.find(*connected_ms);
      if(ms_it != todo->ms_cache.end())
      {
        store.add_constraint_store(ms_it->second);
      }
      else
      {
        ConstraintStore sub_store =
          calculate_constraint_store(connected_ms, todo);

        if(sub_store.consistent())
        {
          HYDLA_LOGGER_DEBUG("CONSISTENT");
          todo->ms_cache.insert(std::make_pair(*connected_ms, sub_store) );
          store.add_constraint_store(sub_store);
        }
        else
        {

          HYDLA_LOGGER_DEBUG("INCONSISTENT");
          //if(opts_->use_unsat_core) mark_nodes_by_unsat_core(ms, todo, time_applied_map);
          //else
	  if(opts_->reuse){
	    module_set_sptr changed_ms(new ModuleSet());
	    for( auto it : *connected_ms ){
	      if(has_variables(it.second,todo->changing_variables,false)) changed_ms->add_module(it);
	    }
	    todo->module_set_container->mark_nodes(todo->maximal_mss, *changed_ms);
	  }
	  else todo->module_set_container->mark_nodes(todo->maximal_mss, *connected_ms);
          return result;
        }
      }
    }
  }
  todo->module_set_container->mark_nodes();
  if(!(opts_->nd_mode || opts_->interactive_mode)) todo->module_set_container->reset(module_set_list_t());
  todo->maximal_mss.push_back(ms);

  phase_result_sptr_t phase = make_new_phase(todo, store);
  phase->module_set = ms;

  // 変数表はここで作成する
  backend_->call("resetConstraint", 0, "", "");
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
  if(opts_->epsilon_mode>0)
  {
    //aho
    // std::cout << "create reslt : variable map" << std::endl;
    // // phase->variable_map = cut_high_order_epsilon(backend_.get(),phase,opts_->epsilon_mode);
    // for(vector<variable_map_t>::iterator c_it = create_result.begin(); c_it != create_result.end();c_it++){
    //   std::cout << "test" << std::endl;
    //   for(variable_map_t::iterator v_it = c_it->begin(); v_it != c_it->end(); v_it++)
    //     {
    //       std::cout << "\t " << v_it->first << "\t: " << v_it->second << std::endl;
    //     }
    // }
    //aho
  }
  phase->variable_map = create_result[0];

  if(opts_->reuse && todo->module_set_container == msc_no_init_){
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
      switch(check_entailment(cc_result, symbolic_expression::node_sptr(new symbolic_expression::Not(opts_->assertion)), continuity_map_t(), todo->phase_type)){
      case ENTAILED:
      case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
        std::cout << "Assertion Failed!" << std::endl;
        HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
        phase->cause_for_termination = ASSERTION;
        break;
      case CONFLICTING:
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
      switch(check_entailment(cc_result, break_condition_, continuity_map_t(), todo->phase_type)){
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

  if(opts_->epsilon_mode>0){
    phase->variable_map = cut_high_order_epsilon(backend_.get(),phase,opts_->epsilon_mode);
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
                                parse_tree_sptr pt,
                                const module_set_container_sptr &msc_no_init)
{
  variable_set_ = &v;
  parameter_map_ = &p;
  variable_map_ = &m;
  phase_sum_ = 0;
  parse_tree_ = pt;
  msc_no_init_ = msc_no_init;
  const hydla::simulator::module_set_sptr ms = msc_no_init->get_max_module_set();

  relation_graph_.reset(new RelationGraph(*ms, *variable_set_));

  if(opts_->dump_relation){
    relation_graph_->dump_graph(std::cout);
    exit(EXIT_SUCCESS);
  }

  AskCollector ac(ms);
  always_set_t eat;
  positive_asks_t pat;
  negative_asks_t nat;

  ac.collect_ask(&eat, &pat, &nat, &prev_guards_);
  PrevSearcher searcher;
  for(negative_asks_t::iterator it = prev_guards_.begin(); it != prev_guards_.end();){
    if(!searcher.search_prev((*it)->get_guard())){
      prev_guards_.erase(it++);
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
    value_t tmp_val = it->second.get_unique();
    backend_->call("substituteParameterCondition",
                   2, "vlnmp", "vl", &tmp_val, &pm, &tmp_val);
    it->second.set_unique(tmp_val);
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

module_set_list_t PhaseSimulator::calculate_mms(
  simulation_todo_sptr_t& state,
  const variable_map_t& vm)
{
  timer::Timer cmms_timer;
  module_set_list_t ret = analysis_result_checker_->calculate_mms(state,vm,todo_container_);
  state->profile["CalculateMMS"] += cmms_timer.get_elapsed_us();
  return ret;
}


variable_map_t shift_variable_map_time(const variable_map_t& vm,
                                       Backend* backend_, const value_t &time){
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


/*
CalculateConstraintStoreResult
PhaseSimulator::check_conditions
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
*/
 /*
void PhaseSimulator::mark_nodes_by_unsat_core
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
 */
  /*
void
PhaseSimulator::find_unsat_core
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
  */

void PhaseSimulator::set_backend(backend_sptr_t back)
{
  backend_ = back;
  unsat_core_finder_->set_backend(back);
}


void PhaseSimulator::set_simulation_mode(const PhaseType& phase)
{
  current_phase_ = phase;
}


PhaseSimulator::CheckEntailmentResult PhaseSimulator::check_entailment(
  CheckConsistencyResult &cc_result,
  const symbolic_expression::node_sptr& guard,
  const continuity_map_t& cont_map,
  const PhaseType& phase
  )
{
  CheckEntailmentResult ce_result;
  ConsistencyChecker consistency_checker(backend_);
  HYDLA_LOGGER_DEBUG(get_infix_string(guard) );
  backend_->call("startTemporary", 0, "", "");
  consistency_checker.add_continuity(cont_map, phase);
  const char* fmt = (phase == PointPhase)?"en":"et";
  backend_->call("addConstraint", 1, fmt, "", &guard);
  cc_result = consistency_checker.call_backend_check_consistency(phase);
  if(cc_result.consistent_store.consistent()){
    HYDLA_LOGGER_DEBUG("%% entailable");
    if(cc_result.inconsistent_store.consistent()){
      HYDLA_LOGGER_DEBUG("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }
    else
    {
      backend_->call("endTemporary", 0, "", "");
      backend_->call("startTemporary", 0, "", "");
      consistency_checker.add_continuity(cont_map, phase);
      symbolic_expression::node_sptr not_node = symbolic_expression::node_sptr(new Not(guard));
      const char* fmt = (phase == PointPhase)?"en":"et";
      backend_->call("addConstraint", 1, fmt, "", &not_node);
      cc_result = consistency_checker.call_backend_check_consistency(phase);
      if(cc_result.consistent_store.consistent()){
        HYDLA_LOGGER_DEBUG("%% entailablity branches");
        if(cc_result.inconsistent_store.consistent()){
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

bool PhaseSimulator::calculate_closure(simulation_todo_sptr_t& state,
    const module_set_sptr& ms)
{
  // preparation
  positive_asks_t& positive_asks = state->positive_asks;
  negative_asks_t& negative_asks = state->negative_asks;

  ask_set_t unknown_asks;
  always_set_t& expanded_always = state->expanded_always;
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  tells_t         tell_list;
  ConstraintStore   constraint_list;
  ConsistencyChecker consistency_checker(backend_);

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  AlwaysFinder always_finder;

  bool expanded;

  if(opts_->reuse && state->in_following_step() ){
    if(state->phase_type == PointPhase){
      ask_collector.collect_ask(&expanded_always,
          &positive_asks,
          &negative_asks,
          &unknown_asks);
      apply_discrete_causes_to_guard_judgement( state->discrete_causes, positive_asks, negative_asks, unknown_asks );
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
    HYDLA_LOGGER_DEBUG_VAR(expanded_always.size());
    tell_collector.collect_new_tells(&tell_list,
        &expanded_always,
        &positive_asks);

    timer::Timer consistency_timer;

    constraint_list.clear();

    maker.reset();

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
      }
      constraint_list.add_constraint(tell);
//      maker.visit_node(tell, state->phase_type == IntervalPhase, false);
    }

//    continuity_map = maker.get_continuity_map();

    for(auto constraint : state->initial_constraint_store){
      constraint_list.add_constraint(constraint);
    }

    {
      CheckConsistencyResult cc_result;
      cc_result = consistency_checker.check_consistency(constraint_list, state->phase_type);
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
        if(opts_->reuse)
        {
          VariableFinder variable_finder;
          variable_finder.visit_node(*it);
          if(!variable_finder.include_variables(state->parent->changed_variables))
          {
            if(state->parent->positive_asks.find(*it) != state->parent->positive_asks.end())  positive_asks.insert(*it);
            else negative_asks.insert(*it);
            unknown_asks.erase(it);
            it++;
            continue;
          }
        }
        if(state->phase_type == PointPhase){
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
        maker.visit_node((*it)->get_child(), state->phase_type == IntervalPhase, true);
        CheckConsistencyResult check_consistency_result;
        switch(check_entailment(check_consistency_result, (*it)->get_guard(), maker.get_continuity_map(), state->phase_type)){
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
            if(prev_guards_.find(*it) != prev_guards_.end()){
              state->judged_prev_map.insert(std::make_pair(*it, true));
            }
            always_finder.find_always((*it)->get_child(), expanded_always);
            unknown_asks.erase(it++);
            expanded = true;
            if(opts_->reuse && !state->parent->changed_variables.empty()){
              VariableFinder variable_finder;
              variable_finder.visit_node(*it);
              VariableFinder::variable_set_t tmp_vars = variable_finder.get_all_variable_set();
              for(auto var : tmp_vars){
                state->parent->changed_variables.insert(var.get_name());
              }
            }
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
//        maker.set_continuity_map(continuity_map);
      }
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
    if(entailment_changed) continue;
  }while(expanded);

  //add_continuity(continuity_map, state->phase_type);

  if(!unknown_asks.empty()){
    boost::shared_ptr<hydla::symbolic_expression::Ask> branched_ask = *unknown_asks.begin();
    // TODO: 極大性に対して厳密なものになっていない（実行アルゴリズムを実装しきれてない）
    HYDLA_LOGGER_DEBUG("%% branched_ask:", get_infix_string(branched_ask));
    {
      // 分岐先を生成（導出される方）
      simulation_todo_sptr_t new_todo(create_new_simulation_phase(state));
      new_todo->initial_constraint_store.add_constraint((branched_ask)->get_guard());
      todo_container_->push_todo(new_todo);
    }
    {
      // 分岐先を生成（導出されない方）
      state->initial_constraint_store.add_constraint(symbolic_expression::node_sptr(new Not((branched_ask)->get_guard())));
      negative_asks.insert(branched_ask);
      return calculate_closure(state, ms);
    }
  }

  return true;
}

ConstraintStore
PhaseSimulator::calculate_constraint_store(
  const module_set_sptr& ms,
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

  if(todo->phase_type == PointPhase)
  {
    backend_->call("getConstraintStorePoint", 0, "", "cs", &result_store);
  }
  else
  {
    backend_->call("getConstraintStoreInterval", 0, "", "cs", &result_store);
    replace_prev2parameter(todo->parent, result_store, todo->parameter_map);
  }

  return result_store;
}

void PhaseSimulator::apply_discrete_causes_to_guard_judgement( ask_set_t& discrete_causes,
                                                                       positive_asks_t& positive_asks,
                                                                       negative_asks_t& negative_asks,
                                                                       ask_set_t& unknown_asks ){
  /*
  std::cout << "before" << std::endl;
  std::cout << "A+: " << positive_asks << std::endl;
  std::cout << "A-: " << negative_asks << std::endl;
  std::cout << "Au: " << unknown_asks << std::endl;
  */

  PrevSearcher searcher;
  ask_set_t prev_asks = unknown_asks;

  for( auto ask : unknown_asks ){
    if( !searcher.search_prev(ask) ){
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

void PhaseSimulator::set_changing_variables( const phase_result_sptr_t& parent_phase,
                                                           const module_set_sptr& present_ms,
                                                           const positive_asks_t& positive_asks,
                                                           const negative_asks_t& negative_asks,
                                                           change_variables_t& changing_variables ){
  //条件なし制約の差分取得
  module_set_sptr parent_ms = parent_phase->module_set;
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
}

void PhaseSimulator::set_changed_variables(phase_result_sptr_t& phase)
{
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

bool PhaseSimulator::apply_entailment_change( const ask_set_t::iterator it,
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

void PhaseSimulator::apply_previous_solution(const change_variables_t& variables,
    const bool in_IP,
    const phase_result_sptr_t parent,
    continuity_map_t& continuity_map,
    const value_t& current_time ){
  ConsistencyChecker consistency_checker(backend_);
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
        value_t val = parent->parent->variable_map.find(pair.first)->second.get_unique();
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
  PhaseType phase;
  if(in_IP) phase = IntervalPhase;
  else phase = PointPhase;
  consistency_checker.add_continuity(continuity_map, phase);
}

PhaseSimulator::todo_list_t
  PhaseSimulator::make_next_todo(phase_result_sptr_t& phase, simulation_todo_sptr_t& current_todo)
{
  todo_list_t ret;

  simulation_todo_sptr_t next_todo(new SimulationTodo());
  next_todo->module_set_container = msc_no_init_;
  next_todo->parent = phase;
  next_todo->ms_to_visit = next_todo->module_set_container->get_full_ms_list();
  next_todo->expanded_always = phase->expanded_always;
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
    //aho
    // std::cout<<"makenexttodo come here 1\n";
    // std::cout<<"phase->constraint_store"<<phase->constraint_store<<"\n";
    // std::cout << "variable map" << std::endl;
    // for(variable_map_t::iterator v_it = phase->variable_map.begin(); v_it != phase->variable_map.end(); v_it++)
    //   {
    //     std::cout << "\t " << v_it->first << "\t: " << v_it->second << std::endl;
    //   }
    //aho

    PhaseSimulator::replace_prev2parameter(phase->parent, phase->variable_map, phase->parameter_map);
    variable_map_t vm_before_time_shift = phase->variable_map;
    phase->variable_map = shift_variable_map_time(phase->variable_map, backend_.get(), phase->current_time);
    next_todo->phase_type = PointPhase;

    timer::Timer next_pp_timer;
    dc_causes_t dc_causes;

    //TODO: どこかで事前にmapを作って、毎回作らないようにする。
    std::map<int, boost::shared_ptr<Ask> > ask_map;

    //現在導出されているガード条件にNotをつけたものを離散変化条件として追加
    for(positive_asks_t::const_iterator it = phase->positive_asks.begin(); it != phase->positive_asks.end(); it++){
      symbolic_expression::node_sptr negated_node(new Not((*it)->get_guard()));
      int id = (*it)->get_id();
      ask_map[id] = *it;
      dc_causes.push_back(dc_cause_t(negated_node, id) );
    }
    //現在導出されていないガード条件を離散変化条件として追加
    for(negative_asks_t::const_iterator it = phase->negative_asks.begin(); it != phase->negative_asks.end(); it++){
      symbolic_expression::node_sptr node((*it)->get_guard() );
      int id = (*it)->get_id();
      ask_map[id] = *it;
      dc_causes.push_back(dc_cause_t(node, id));
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
      max_time = symbolic_expression::node_sptr(new hydla::symbolic_expression::Number(opts_->max_time));
    }else{
      max_time = symbolic_expression::node_sptr(new hydla::symbolic_expression::Infinity());
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

    else if(opts_->epsilon_mode>0){
      time_result = pass_specific_case(time_result,backend_.get(), phase,vm_before_time_shift,dc_causes,time_limit,current_todo);
      time_result = reduce_unsuitable_case(time_result,backend_.get(),phase);
    }

    else
      backend_->call("calculateNextPointPhaseTime", 2, "vltdc", "cp", &(time_limit), &dc_causes, &time_result);

    unsigned int time_it = 0;
    result_list_t results;
    phase_result_sptr_t pr = next_todo->parent;


    // まずインタラクティブ実行のために最小限の情報だけ整理する
    while(true)
    {
      NextPhaseResult &candidate = time_result[time_it];
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

      NextPhaseResult &candidate = time_result[result_it];
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
