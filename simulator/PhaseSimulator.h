#ifndef _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
#define _INCLUDED_HYDLA_PHASE_SIMULATOR_H_

#include <iostream>
#include <string>
#include <cassert>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>


#include "Logger.h"
#include "VariableMap.h"
#include "PhaseState.h"
#include "TreeInfixPrinter.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

template<typename PhaseStateType>
class PhaseSimulator{

public:  
  typedef PhaseStateType                                   phase_state_t; 
  typedef typename boost::shared_ptr<phase_state_t>        phase_state_sptr; 
  typedef typename boost::shared_ptr<const phase_state_t>  phase_state_const_sptr; 
  typedef std::vector<phase_state_sptr>                    phase_state_sptrs_t;
  typedef PhaseSimulator<PhaseStateType>                   phase_simulator_t;

  typedef typename phase_state_t::variable_map_t variable_map_t;
  typedef typename phase_state_t::variable_t     variable_t;
  typedef typename phase_state_t::parameter_t     parameter_t;
  typedef typename phase_state_t::value_t        value_t;
  typedef typename phase_state_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef std::vector<phase_state_sptr>                    Phases;
  
  PhaseSimulator(const Opts& opts):opts_(&opts){
  }
  
  virtual ~PhaseSimulator(){}

  virtual phase_state_sptrs_t simulate_phase_state(const module_set_sptr& ms, 
                                    phase_state_sptr& state)
  {
    HYDLA_LOGGER_PHASE("#*** Begin Simulator::simulate_phase_state ***");
    HYDLA_LOGGER_PHASE("%% current time:", state->current_time);
    HYDLA_LOGGER_PHASE("--- parent variable map ---\n", state->parent->variable_map);
    HYDLA_LOGGER_PHASE("--- parameter map ---\n", state->parameter_map);
    HYDLA_LOGGER_PHASE("--- module set ---\n",
          ms->get_name(),
          "\n",
          ms->get_infix_string() );
    
    
    phase_state_sptrs_t phases; 
    switch(state->phase) 
    {
      case PointPhase:
      { 
      HYDLA_LOGGER_PHASE("%% begin point phase");

        phases = point_phase(ms, state);
        break;
      }

      case IntervalPhase: 
      {
      HYDLA_LOGGER_PHASE("%% begin interval phase");
        phases = interval_phase(ms, state);
        break;            
      }

    default:
      assert(0);
    }
    
    return phases;
  }
    
  /**
   * id形式のexpanded_alwaysをshared_ptr形式に変換する
   */
  void expanded_always_id2sptr(const expanded_always_id_t& ea_id, 
                               expanded_always_t& ea_sptr) const
  {
    ea_sptr.clear();

    expanded_always_id_t::const_iterator it  = ea_id.begin();
    expanded_always_id_t::const_iterator end = ea_id.end();
    for(; it!=end; ++it) {
      assert(
        boost::dynamic_pointer_cast<hydla::parse_tree::Always>(
          parse_tree_->get_node(*it)));

      ea_sptr.insert(
        boost::static_pointer_cast<hydla::parse_tree::Always>(
          parse_tree_->get_node(*it)));
    }
  }

  /**
   * shared_ptr形式のexpanded_alwaysをid形式に変換する
   */
  void expanded_always_sptr2id(const expanded_always_t& ea_sptr, 
                               expanded_always_id_t& ea_id) const
  {
    ea_id.clear();

    expanded_always_t::const_iterator it  = ea_sptr.begin();
    expanded_always_t::const_iterator end = ea_sptr.end();
    for(; it!=end; ++it) {
      //ea_id.push_back(parse_tree_->get_node_id(*it));
      ea_id.push_back((*it)->get_id());
    }
  }  
  
  
  /**
   * 新たなPhaseStateの作成
   */
  phase_state_sptr create_new_phase_state() const
  {
    phase_state_sptr ph(new phase_state_t());
    ph->cause_of_termination = NONE;
    return ph;
  }

  /**
   * 与えられたPhaseStateの情報をを引き継いだ，
   * 新たなPhaseStateの作成
   */
  phase_state_sptr create_new_phase_state(const phase_state_const_sptr& old) const
  {
    phase_state_sptr ph(new phase_state_t(*old));
    ph->cause_of_termination = NONE;
    return ph;
  }

  
  /**
   * Point Phaseの処理
   */
  virtual Phases point_phase(const module_set_sptr& ms, 
                           phase_state_sptr& state) = 0;

  /**
   * Interval Phaseの処理
   */
  virtual Phases interval_phase(const module_set_sptr& ms, 
                              phase_state_sptr& state) = 0;

  virtual void initialize(const parse_tree_sptr& parse_tree, variable_set_t &v, parameter_set_t &p, variable_map_t &m)
  {
    parse_tree_ = parse_tree;
    variable_set_ = &v;
    parameter_set_ = &p;
    variable_map_ = &m;
  }
  
  
protected:

  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;
  
  
  const Opts     *opts_;
  
  variable_set_t *variable_set_;
  parameter_set_t *parameter_set_;
  variable_map_t *variable_map_;
};


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
