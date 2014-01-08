#include "PhaseSimulator.h"
#include "AskCollector.h"
#include "NonPrevSearcher.h"
#include "VariableFinder.h"
#include "SymbolicValue.h"
#include "Exceptions.h"
#include "Backend.h"

using namespace hydla::simulator;
using namespace hydla::backend;

PhaseSimulator::PhaseSimulator(Simulator* simulator,const Opts& opts): breaking(false), simulator_(simulator), opts_(&opts), select_phase_(NULL), break_condition_(node_sptr()) {
}

PhaseSimulator::~PhaseSimulator(){}

void PhaseSimulator::set_backend(backend_sptr_t back)
{
  backend_ = back;
}

PhaseSimulator::result_list_t PhaseSimulator::calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont)
{
  HYDLA_LOGGER_PHASE("%% current time:", *todo->current_time);
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
  variable_map_t time_applied_map;
  boost::shared_ptr<RelationGraph> graph;
  
  if(todo->phase == PointPhase)
  {
    time_applied_map = apply_time_to_vm(todo->parent->variable_map, todo->current_time);
    graph = pp_relation_graph_;
    /*
    if(todo->current_time->get_string() != "0" && opts_->analysis_mode == "cmmap"){
      module_set_list_t mms = calculate_mms(todo,time_applied_map);
      set_simulation_mode(PointPhase);
      for(module_set_list_t::iterator it = mms.begin(); it != mms.end(); it++){
	timer::Timer ms_timer;
        result_list_t tmp_result = simulate_ms((*it), graph, time_applied_map, todo);
	if(!tmp_result.empty())
	  {
	    has_next = true;
	    result.insert(result.begin(), tmp_result.begin(), tmp_result.end());
	  }
	std::string module_sim_string = "\"ModuleSet" + (*it)->get_name() + "\"";
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
    */
    set_simulation_mode(PointPhase);
  }else{
    time_applied_map = todo->parent->variable_map;
    graph = ip_relation_graph_;
    set_simulation_mode(IntervalPhase);
  }
  
  while(todo->module_set_container->go_next())
  {
    module_set_sptr ms = todo->module_set_container->get_module_set();
    
    std::string module_sim_string = "\"ModuleSet" + ms->get_name() + "\"";
    timer::Timer ms_timer;
    result_list_t tmp_result = simulate_ms(ms, graph, time_applied_map, todo);
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


PhaseSimulator::result_list_t PhaseSimulator::simulate_ms(const hydla::ch::module_set_sptr& ms,
  boost::shared_ptr<RelationGraph>& graph, const variable_map_t& time_applied_map, simulation_todo_sptr_t& todo)
{
  HYDLA_LOGGER_MS("--- next module set ---\n", ms->get_infix_string());
  graph->set_valid(ms.get());
  result_list_t result;
  // TODO:変数の値による分岐も無視している？

  variable_maps_t vms;
  vms.push_back(*variable_map_);

  if(todo->module_set_container != msc_no_init_)
  {
    variable_maps_t tmp_vms;
    CalculateVariableMapResult cvm_res =
      calculate_variable_map(ms, todo, time_applied_map, tmp_vms);
    switch(cvm_res)
    {
      case CVM_CONSISTENT:
      HYDLA_LOGGER_LOCATION(MS);
      merge_variable_maps(vms, tmp_vms);
      break;
      
      case CVM_INCONSISTENT:
      HYDLA_LOGGER_LOCATION(MS);
      if(opts_->use_unsat_core) mark_nodes_by_unsat_core(ms, todo, time_applied_map);
      else todo->module_set_container->mark_nodes(todo->maximal_mss, *ms);
      if(opts_->find_unsat_core_mode)
      {
        find_unsat_core(ms, todo, time_applied_map);
      }
      return result;
      break;
      
      case CVM_ERROR:
      HYDLA_LOGGER_LOCATION(MS);
      throw SimulateError("CalculateVariableMap for " + ms->get_name());
      break;
    }
  }
  else
  {
    CalculateVariableMapResult cvm_res;
    /*
    if(todo->phase == PointPhase && opts_->analysis_mode == "simulate"){
      cvm_res = check_conditions(ms, todo, time_applied_map, true);
      switch(cvm_res){
        case CVM_CONSISTENT:
          break;
        case CVM_INCONSISTENT:
          todo->module_set_container->mark_nodes(*ms);
          return result;
        case CVM_ERROR:
          throw SimulateError("CalculateVariableMap for " + ms->get_name());
          break;
        default:
          assert(0);
          break;
      }
    }
    */
    HYDLA_LOGGER_LOCATION(MS);
    HYDLA_LOGGER_MS("%% connected module set size:", graph->get_connected_count());
    for(int i = 0; i < graph->get_connected_count(); i++){
      module_set_sptr connected_ms = graph->get_component(i);
      HYDLA_LOGGER_MS("--- next connected module set ---\n", connected_ms->get_infix_string());
      SimulationTodo::ms_cache_t::iterator ms_it = todo->ms_cache.find(*connected_ms);
      if(ms_it != todo->ms_cache.end())
      {
        merge_variable_maps(vms, ms_it->second);
      }
      else
      {
        variable_maps_t tmp_vms;
        cvm_res =
          calculate_variable_map(connected_ms, todo, time_applied_map, tmp_vms);
        switch(cvm_res)
        {
          case CVM_CONSISTENT:
          HYDLA_LOGGER_LOCATION(MS);
          HYDLA_LOGGER_MS("--- CVM_CONSISTENT ---\n");
          todo->ms_cache.insert(std::make_pair(*connected_ms, tmp_vms) );
          merge_variable_maps(vms, tmp_vms);
          break;
          
          case CVM_INCONSISTENT:
          HYDLA_LOGGER_LOCATION(MS);
          if(opts_->use_unsat_core) mark_nodes_by_unsat_core(ms, todo, time_applied_map);
          else todo->module_set_container->mark_nodes(todo->maximal_mss, *connected_ms);
          if(opts_->find_unsat_core_mode)
          {
            find_unsat_core(ms, todo, time_applied_map);
          }
          return result;
          break;

          case CVM_ERROR:
          throw SimulateError("CalculateVariableMap for " + connected_ms->get_name());
          break;
        }
      }
    }
  }
  todo->module_set_container->mark_nodes();
  if(!(opts_->nd_mode || opts_->interactive_mode)) todo->module_set_container->reset(module_set_list_t());
  todo->maximal_mss.push_back(ms);
  for(unsigned int i=0; i < vms.size(); i++)
  {
    variable_map_t& vm = vms[i];
    
    phase_result_sptr_t phase = make_new_phase(todo, vm);
    phase->module_set = ms;
    
    
    
    if(opts_->assertion || break_condition_.get() != NULL){
      timer::Timer entailment_timer;
      
      backend_->call("resetConstraintForVariable", 0, "","");
      std::string fmt = "mv0";
      fmt += (phase->phase==PointPhase)?"n":"t";
      backend_->call("addConstraint", 1, fmt.c_str(), "", &phase->variable_map);
      backend_->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);
     
      if(opts_->assertion)
      {
        HYDLA_LOGGER_MS("%% check_assertion");
        CheckConsistencyResult cc_result;
        switch(check_entailment(cc_result, node_sptr(new parse_tree::Not(opts_->assertion)), continuity_map_t(), todo->phase)){
        case ENTAILED:
        case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
          std::cout << "Assertion Failed!" << std::endl;
          HYDLA_LOGGER_CLOSURE("%% Assertion Failed!");
          phase->cause_of_termination = ASSERTION;
          break;
        case CONFLICTING:
          break;
        case BRANCH_PAR:
          HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of parameters");
          push_branch_states(todo, cc_result);
          std::cout << "Assertion Failed!" << std::endl;
          phase->parameter_map = todo->parameter_map;
          HYDLA_LOGGER_CLOSURE("%% Assertion Failed!");
          phase->cause_of_termination = ASSERTION;
          break;
        }
        todo->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
      }
      if(break_condition_.get() != NULL)
      {
        HYDLA_LOGGER(MS, "%% check_break_condition");
        CheckConsistencyResult cc_result;
        switch(check_entailment(cc_result, break_condition_, continuity_map_t(), todo->phase)){
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
  
    result.push_back(phase);
  }
  return result;
}


void PhaseSimulator::push_branch_states(simulation_todo_sptr_t &original, CheckConsistencyResult &result){
  assert(result.size() == 2);
  for(int i=1; i<(int)result[0].size();i++){
    simulation_todo_sptr_t branch_state_true(create_new_simulation_phase(original));
    branch_state_true->parameter_map = result[0][i];
    HYDLA_LOGGER_VAR(PHASE, branch_state_true);
    todo_container_->push_todo(branch_state_true);
  }
  for(int i=0; i<(int)result[1].size();i++){
    simulation_todo_sptr_t branch_state_false(create_new_simulation_phase(original));
    branch_state_false->parameter_map = result[1][i];
    HYDLA_LOGGER_VAR(PHASE, branch_state_false);
    todo_container_->push_todo(branch_state_false);
  }
  original->parameter_map = result[0][0];
  backend_->call("resetConstraintForParameter", 1, "mp", "", &original->parameter_map);
}


phase_result_sptr_t PhaseSimulator::make_new_phase(simulation_todo_sptr_t& todo, const variable_map_t& vm)
{
  phase_result_sptr_t phase(new PhaseResult(*todo));
  phase->id = ++phase_sum_;
  phase->variable_map = vm;
  todo->parent->children.push_back(phase);
  return phase;
}


phase_result_sptr_t PhaseSimulator::make_new_phase(const phase_result_sptr_t& original)
{
  phase_result_sptr_t phase(new PhaseResult(*original));
  phase->id = ++phase_sum_;
  phase->parent->children.push_back(phase);
  phase->cause_of_termination = simulator::NONE;
  return phase;
}


void PhaseSimulator::merge_variable_maps(variable_maps_t& lhs, const variable_maps_t& rhs)
{
  unsigned int original_l_size = lhs.size();
  for(unsigned int r_it = 0; r_it < rhs.size() - 1; r_it++)
  {
    for(unsigned int l_it = 0; l_it < original_l_size; l_it++)
    {
      variable_map_t tmp_map = lhs[l_it];
      merge_variable_map(tmp_map, rhs[r_it]);
      lhs.push_back(tmp_map);
    }
  }
  if(rhs.size() > 0)
  {
    for(unsigned int l_it = 0; l_it < original_l_size; l_it++)
    {
      merge_variable_map(lhs[l_it], rhs[rhs.size() - 1]);
    }
  }
}


void PhaseSimulator::merge_variable_map(variable_map_t& lhs, const variable_map_t& rhs)
{
  variable_map_t::const_iterator it = rhs.begin();
  for(;it != rhs.end(); it++)
  {
    if(!it->second.undefined())
    {
      lhs[it->first] = it->second;
    }
  }
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
  
  // TODO:RelationGraph上では，ASSERT文を無視しているため，制約モジュール中では独立でもASSERT文の中で関係している変数があった場合に正しく動作しない
  // RelationGraphを作る上では，ASSERTを特殊な「モジュールのようなもの」として扱う必要がある．
  pp_relation_graph_ = RelationGraph::new_graph(*ms, *variable_set_, false);
  ip_relation_graph_ = RelationGraph::new_graph(*ms, *variable_set_, true);
  
  if(opts_->dump_relation){
    pp_relation_graph_->dump_graph(std::cout);
    ip_relation_graph_->dump_graph(std::cout);
    exit(EXIT_SUCCESS);
  }
  
  AskCollector ac(ms);
  expanded_always_t eat;
  positive_asks_t pat;
  negative_asks_t nat;

  ac.collect_ask(&eat, &pat, &nat, &prev_guards_);
  NonPrevSearcher searcher;
  for(negative_asks_t::iterator it = prev_guards_.begin(); it != prev_guards_.end();){
    if(searcher.judge_non_prev((*it)->get_guard())){
      prev_guards_.erase(it++);
    }else{
      it++;
    }
  }
  backend_->set_variable_set(*variable_set_);
}


simulation_todo_sptr_t PhaseSimulator::create_new_simulation_phase(const simulation_todo_sptr_t& old) const
{
  simulation_todo_sptr_t sim(new SimulationTodo(*old));
  return sim;
}

bool PhaseSimulator::check_include_bound(value_t tmp_variable_phase, value_t tmp_variable_past, parameter_map_t pm1, parameter_map_t pm2)
{
  HYDLA_LOGGER_LOCATION(HA);
  bool ret;
  backend_->call("checkIncludeBound", 4, "vlnvlnmpmp", "b", &tmp_variable_phase, &tmp_variable_past, &pm1, &pm2, &ret);
  return ret;
}

void PhaseSimulator::substitute_values_for_vm(phase_result_sptr_t pr, std::map<parameter_t, value_t> vm)
{
/*
  HYDLA_LOGGER_LOCATION(HAS);
	// 変数に代入
	variable_map_t ret;
  backend_->call("SubstituteValue",);
	solver_->substitute_values_for_vm(pr->variable_map, ret, vm);
	pr->variable_map = ret;
	// 時刻にも代入
  HYDLA_LOGGER_LOCATION(HAS);
	time_t ret_time;
	solver_->substitute_values_for_time(pr->current_time, ret_time, vm);
	pr->current_time = ret_time;
	if(pr->phase == IntervalPhase){
		solver_->substitute_values_for_time(pr->end_time, ret_time, vm);
		pr->end_time = ret_time;
	}
*/
}

void PhaseSimulator::substitute_current_time_for_vm(phase_result_sptr_t pr, time_t current_time)
{
/*
  HYDLA_LOGGER_LOCATION(HAS);
	variable_map_t ret;
  solver_->substitute_current_time_for_vm(pr->variable_map, ret, current_time);
  pr->variable_map = ret;
4	// 時刻にも適用
  HYDLA_LOGGER_LOCATION(HAS);
	time_t ret_time;
	solver_->substitute_current_time_for_time(pr->current_time, ret_time, current_time);
	pr->current_time = ret_time;
	if(pr->phase == IntervalPhase){
		solver_->substitute_current_time_for_time(pr->end_time, ret_time, current_time);
		pr->end_time = ret_time;
	}
*/
}

void PhaseSimulator::replace_prev2parameter(variable_map_t &vm,
                                            phase_result_sptr_t &phase)
{
  assert(phase->parent.get() != NULL);
}

void PhaseSimulator::set_break_condition(node_sptr break_cond)
{
  break_condition_ = break_cond;
}

PhaseSimulator::node_sptr PhaseSimulator::get_break_condition()
{
  return break_condition_;
}
