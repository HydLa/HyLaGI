#include "PhaseSimulator.h"

using namespace hydla::simulator;



PhaseSimulator::PhaseSimulator(const Opts& opts):opts_(&opts){
}

PhaseSimulator::~PhaseSimulator(){}

PhaseSimulator::simulation_phases_t PhaseSimulator::simulate_phase(SimulationPhase& state, bool &consistent)
{
  HYDLA_LOGGER_PHASE("%% current time:", state.phase_result->current_time);
  HYDLA_LOGGER_PHASE("--- parent variable map ---\n", state.phase_result->parent->variable_map);
  HYDLA_LOGGER_PHASE("--- parameter map ---\n", state.phase_result->parameter_map);

  state.phase_result->phase_timer.restart();

  simulation_phases_t phases; 
  bool has_next = false;
  is_safe_ = true;
  variable_map_t time_applied_map;
  if(state.phase_result->phase == PointPhase){
    time_applied_map = apply_time_to_vm(state.phase_result->parent->variable_map, state.phase_result->current_time);
  }

  //TODO:exclude_error‚ª–³Œø‚É‚È‚Á‚Ä‚é
  while(state.module_set_container->go_next()){

    module_set_sptr ms = state.module_set_container->get_module_set();

    HYDLA_LOGGER_MS("--- next module set ---\n",
          ms->get_name(),
          "\n",
          ms->get_infix_string() );
    switch(state.phase_result->phase) 
    {
      case PointPhase:
      { 
        simulation_phases_t tmp = simulate_ms_point(ms, state, time_applied_map, consistent);
        phases.insert(phases.begin(), tmp.begin(), tmp.end());
        break;
      }

      case IntervalPhase: 
      {
        simulation_phases_t tmp = simulate_ms_interval(ms, state, consistent);
        phases.insert(phases.begin(), tmp.begin(), tmp.end());
        break;            
      }
      default:
        assert(0);
        break;
    }

    if(consistent){
      state.module_set_container->mark_nodes();
      has_next = true;
    }else{
      state.module_set_container->mark_current_node();
    }
    if(phases.size() > 1){
      for(phase_result_sptrs_t::iterator it = state.phase_result->parent->children.begin(); it != state.phase_result->parent->children.end(); it++){
        if((*it)->id == state.phase_result->id) (*it)->phase_timer.count_time();
      }
      return phases;
    }
    
    state.phase_result->positive_asks.clear();
  }
  
  //–³–µ‚‚È‰ðŒó•âƒ‚ƒWƒ…[ƒ‹W‡‚ª‘¶Ý‚µ‚È‚¢ê‡
  if(!has_next){
    state.phase_result->cause_of_termination = simulator::INCONSISTENCY;
    state.phase_result->parent->children.push_back(state.phase_result);
  }
  for(phase_result_sptrs_t::iterator it = state.phase_result->parent->children.begin(); it != state.phase_result->parent->children.end(); it++){
    if((*it)->id == state.phase_result->id) (*it)->phase_timer.count_time();
  }
  return phases;
}


void PhaseSimulator::initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c)
{
  variable_set_ = &v;
  parameter_set_ = &p;
  variable_map_ = &m;
}


PhaseSimulator::simulation_phase_t PhaseSimulator::create_new_simulation_phase() const
{
  simulation_phase_t ph;
  ph.phase_result.reset(new phase_result_t);
  ph.phase_result->cause_of_termination = NONE;
  return ph;
}

PhaseSimulator::simulation_phase_t PhaseSimulator::create_new_simulation_phase(const simulation_phase_t& old) const
{
  simulation_phase_t sim;
  sim.phase_result.reset(new phase_result_t(*old.phase_result));
  sim.phase_result->cause_of_termination = NONE;
  return sim;
}
