#include "PhaseSimulator.h"
#include "AskCollector.h"
#include "NonPrevSearcher.h"
#include "VariableFinder.h"
#include "SymbolicValue.h"
#include "SimulateError.h"
#include "ConstraintAnalyzer.h"

using namespace hydla::simulator;

PhaseSimulator::PhaseSimulator(const Opts& opts):opts_(&opts){}

PhaseSimulator::~PhaseSimulator(){}

PhaseSimulator::todo_and_results_t PhaseSimulator::simulate_phase(simulation_phase_sptr_t& state, bool &consistent)
{
  HYDLA_LOGGER_PHASE("%% current time:", *state->phase_result->current_time);
  HYDLA_LOGGER_PHASE("--- parent variable map ---\n", state->phase_result->parent->variable_map);
  HYDLA_LOGGER_PHASE("--- parameter map ---\n", state->phase_result->parameter_map);
  timer::Timer phase_timer;
  
  todo_and_results_t phases; 
  bool has_next = false;
  is_safe_ = true;
  variable_map_t time_applied_map;
  
  judged_prev_map_.clear();
  
  boost::shared_ptr<RelationGraph> graph;
  
  if(state->phase_result->phase == PointPhase)
  {
    time_applied_map = apply_time_to_vm(state->phase_result->parent->variable_map, state->phase_result->current_time);
    graph = pp_relation_graph_;
    set_simulation_mode(PointPhase);
  }else{
    time_applied_map = state->phase_result->parent->variable_map;
    graph = ip_relation_graph_;
    set_simulation_mode(IntervalPhase);
  }
  
  while(state->module_set_container->go_next())
  {
    module_set_sptr ms = state->module_set_container->get_module_set();
    
    std::string module_sim_string = "\"ModuleSet" + ms->get_name() + "\"";
    timer::Timer ms_timer;
    todo_and_results_t tmp_phases = simulate_ms(ms, graph, time_applied_map, state, consistent);
    state->profile[module_sim_string] += ms_timer.get_elapsed_us();
    phases.insert(phases.begin(), tmp_phases.begin(), tmp_phases.end());
    if(consistent)
    {
      has_next = true;
    }
    else if(phases.size() > 1)
    {
      state->profile["Phase"] += phase_timer.get_elapsed_us();
      return phases;
    }
    state->phase_result->positive_asks.clear();
    state->phase_result->negative_asks.clear();
  }
  
  //無矛盾な解候補モジュール集合が存在しない場合
  if(!has_next)
  {
    state->phase_result->cause_of_termination = simulator::INCONSISTENCY;
    phases.push_back(TodoAndResult(simulation_phase_sptr_t(), state->phase_result));
  }
  state->profile["Phase"] += phase_timer.get_elapsed_us();
  return phases;
}


PhaseSimulator::todo_and_results_t PhaseSimulator::simulate_ms(const hydla::ch::module_set_sptr& ms,
  boost::shared_ptr<RelationGraph>& graph, const variable_map_t& time_applied_map, simulation_phase_sptr_t& state, bool &consistent)
{
  HYDLA_LOGGER_MS("--- next module set ---\n",
        ms->get_name(),
        "\n",
        ms->get_infix_string() );
  graph->set_valid(ms.get());
  todo_and_results_t phases; 
  
  consistent = false;
  
  // TODO:変数表が複数作られるような分岐を無視している
  
  variable_map_t vm;
  
  if(state->phase_result->current_time->get_string() == "0" && state->phase_result->phase == PointPhase)
  {
    CalculateVariableMapResult cvm_res =
      calculate_variable_map(ms, state, time_applied_map, vm, phases);
    switch(cvm_res)
    {
      case CVM_CONSISTENT:
      HYDLA_LOGGER_MS("--- CVM_CONSISTENT ---\n", vm);
      break;
      
      case CVM_INCONSISTENT:
      HYDLA_LOGGER_MS("%% CVM_INCONSISTENT");
      state->module_set_container->mark_nodes(*ms);
      return phases;
      break;
      
      case CVM_ERROR:
      HYDLA_LOGGER_MS("%% CVM_ERROR");
      throw SimulateError("CalculateVariableMap for " + ms->get_name());
      break;
              
      case CVM_BRANCH:
      HYDLA_LOGGER_MS("%% CVM_BRANCH");
      return phases;
      break;
    }
  }
  else
  {
    if(state->phase_result->phase == PointPhase && opts_->optimization_level >= 2){
      CalculateVariableMapResult cvm_res = check_false_conditions(ms,state,time_applied_map,vm,phases);
      switch(cvm_res){
      case CVM_CONSISTENT:
	break;
      case CVM_INCONSISTENT:
        state->module_set_container->mark_nodes(*ms);
      case CVM_BRANCH:
	return phases;
	break;
      case CVM_ERROR:
        break;
      default:
        break;
      }
    }
    HYDLA_LOGGER_MS("%% connected module set size:", graph->get_connected_count());
    for(int i = 0; i < graph->get_connected_count(); i++){
      module_set_sptr connected_ms = graph->get_component(i);
      HYDLA_LOGGER_MS("--- next connected module set ---\n",
            connected_ms->get_name(),
            "\n",
            connected_ms->get_infix_string() );
      SimulationTodo::ms_cache_t::iterator ms_it = state->ms_cache.find(*connected_ms);
      if(ms_it != state->ms_cache.end())
      {
        merge_variable_map(vm, ms_it->second);
      }
      else
      {
        variable_map_t tmp_vm;
        CalculateVariableMapResult cvm_res =
          calculate_variable_map(connected_ms, state, time_applied_map, tmp_vm, phases);
        switch(cvm_res)
        {
          case CVM_CONSISTENT:
          HYDLA_LOGGER_MS("--- CVM_CONSISTENT ---\n", tmp_vm);
          state->ms_cache.insert(std::make_pair(*connected_ms, tmp_vm) );
          HYDLA_LOGGER_MS("%%merge");
          merge_variable_map(vm, tmp_vm);
          break;
          
          case CVM_INCONSISTENT:
          HYDLA_LOGGER_MS("%% CVM_INCONSISTENT");
          state->module_set_container->mark_nodes(*connected_ms);
          return phases;
          break;
          
          case CVM_ERROR:
          HYDLA_LOGGER_MS("%% CVM_ERROR");
          throw SimulateError("CalculateVariableMap for " + connected_ms->get_name());
          break;
                  
          case CVM_BRANCH:
          HYDLA_LOGGER_MS("%% CVM_BRANCH");
          return phases;
          break;
        }
      }
    }
  }
  state->module_set_container->mark_nodes();
  
  todo_and_results_t tmp_phases = make_next_todo(ms, state, vm);
  phases.insert(phases.begin(), tmp_phases.begin(), tmp_phases.end());
  consistent = true;

  return phases;
}

void PhaseSimulator::merge_variable_map(variable_map_t& lhs, variable_map_t& rhs)
{
  variable_map_t::iterator it = rhs.begin();
  for(;it != rhs.end(); it++)
  {
    if(it->second.get() && !it->second->is_undefined())
    {
      lhs[it->first] = it->second;
    }
  }
}

void PhaseSimulator::initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, const hydla::simulator::module_set_sptr& ms, continuity_map_t& c)
{
  variable_set_ = &v;
  parameter_set_ = &p;
  variable_map_ = &m;
  
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
}


simulation_phase_sptr_t PhaseSimulator::create_new_simulation_phase() const
{
  simulation_phase_sptr_t ph(new simulation_phase_t());
  ph->phase_result.reset(new phase_result_t);
  ph->phase_result->cause_of_termination = NONE;
  return ph;
}

simulation_phase_sptr_t PhaseSimulator::create_new_simulation_phase(const simulation_phase_sptr_t& old) const
{
  simulation_phase_sptr_t sim(new simulation_phase_t(*old));
  sim->phase_result.reset(new phase_result_t(*old->phase_result));
  sim->phase_result->cause_of_termination = NONE;
  return sim;
}
