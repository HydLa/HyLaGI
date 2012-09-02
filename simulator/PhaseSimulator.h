#ifndef _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
#define _INCLUDED_HYDLA_PHASE_SIMULATOR_H_

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>


#include "Logger.h"
#include "VariableMap.h"
#include "PhaseResult.h"
#include "TreeInfixPrinter.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

template<typename PhaseResultType>
class PhaseSimulator{

public:  
  typedef PhaseResultType                                   phase_result_t; 
  typedef typename boost::shared_ptr<phase_result_t>        phase_result_sptr; 
  typedef typename boost::shared_ptr<const phase_result_t>  phase_result_const_sptr; 
  typedef std::vector<phase_result_sptr>                    phase_result_sptrs_t;
  typedef PhaseSimulator<PhaseResultType>                   phase_simulator_t;

  typedef typename phase_result_t::variable_map_t variable_map_t;
  typedef typename phase_result_t::variable_t     variable_t;
  typedef typename phase_result_t::parameter_t    parameter_t;
  typedef typename phase_result_t::value_t        value_t;
  typedef typename phase_result_t::time_t         time_t;
  typedef typename phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef std::vector<phase_result_sptr>                   Phases;
  
  PhaseSimulator(const Opts& opts):opts_(&opts){
  }
  
  virtual ~PhaseSimulator(){}

  virtual phase_result_sptrs_t simulate_phase(phase_result_sptr& state, bool &consistent)
  {
    HYDLA_LOGGER_PHASE("#*** Begin Simulator::simulate_phase ***");
    HYDLA_LOGGER_PHASE("%% current time:", state->current_time);
    HYDLA_LOGGER_PHASE("--- parent variable map ---\n", state->parent->variable_map);
    HYDLA_LOGGER_PHASE("--- parameter map ---\n", state->parameter_map);
    
    phase_result_sptrs_t phases; 
    bool has_next = false;
    is_safe_ = true;
    variable_map_t time_applied_map;
    if(state->phase == PointPhase){
      time_applied_map = apply_time_to_vm(state->parent->variable_map, state->current_time);
    }
    //TODO:exclude_errorが無効になってる
    while(state->module_set_container->go_next()){
      module_set_sptr ms = state->module_set_container->get_module_set();
 
      HYDLA_LOGGER_PHASE("--- next module set ---\n",
            ms->get_name(),
            "\n",
            ms->get_infix_string() );
      switch(state->phase) 
      {
        case PointPhase:
        { 
          HYDLA_LOGGER_PHASE("%% begin simulate_ms_point");
          phase_result_sptrs_t tmp = simulate_ms_point(ms, state,time_applied_map, consistent);
          phases.insert(phases.begin(), tmp.begin(), tmp.end());
          break;
        }

        case IntervalPhase: 
        {
          HYDLA_LOGGER_PHASE("%% begin simulate_ms_interval");
          phase_result_sptrs_t tmp = simulate_ms_interval(ms, state, consistent);
          phases.insert(phases.begin(), tmp.begin(), tmp.end());
          break;            
        }
        default:
          assert(0);
          break;
      }
      
      if(consistent){
        state->module_set_container->mark_nodes();
        has_next = true;
      }else{
        state->module_set_container->mark_current_node();
      }
      if(phases.size() > 1){
        return phases;
      }
      
      state->positive_asks.clear();
    }
    
    
    //無矛盾な解候補モジュール集合が存在しない場合
    if(!has_next){
      state->cause_of_termination = simulator::INCONSISTENCY;
      state->parent->children.push_back(state);
    }
    return phases;
  }
  
  /**
   * 新たなPhaseResultの作成
   */
  phase_result_sptr create_new_phase_result() const
  {
    phase_result_sptr ph(new phase_result_t());
    ph->cause_of_termination = NONE;
    return ph;
  }

  /**
   * 与えられたPhaseResultの情報をを引き継いだ，
   * 新たなPhaseResultの作成
   */
  phase_result_sptr create_new_phase_result(const phase_result_const_sptr& old) const
  {
    phase_result_sptr ph(new phase_result_t(*old));
    ph->cause_of_termination = NONE;
    return ph;
  }
  
  virtual variable_map_t apply_time_to_vm(const variable_map_t &, const time_t &) = 0;

  
  /**
   * モジュール集合の無矛盾性を確かめる関数．Point Phase版
   * @return 次にシミュレーションするべきフェーズの集合
   */
  virtual Phases simulate_ms_point(const module_set_sptr& ms, 
                           phase_result_sptr& state, variable_map_t &, bool& consistent) = 0;

  /**
   * モジュール集合の無矛盾性を確かめる関数．Interval Phase版
   * @return 次にシミュレーションするべきフェーズの集合
   */
  virtual Phases simulate_ms_interval(const module_set_sptr& ms, 
                              phase_result_sptr& state, bool& consistent) = 0;

  virtual void initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c)
  {
    variable_set_ = &v;
    parameter_set_ = &p;
    variable_map_ = &m;
  }
  
  
protected:

  const Opts *opts_;

  bool is_safe_;
  
  variable_set_t *variable_set_;
  parameter_set_t *parameter_set_;
  variable_map_t *variable_map_;
};


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
