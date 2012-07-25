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
  typedef typename phase_result_t::parameter_t     parameter_t;
  typedef typename phase_result_t::value_t        value_t;
  typedef typename phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef std::vector<phase_result_sptr>                    Phases;
  
  PhaseSimulator(const Opts& opts):opts_(&opts){
  }
  
  virtual ~PhaseSimulator(){}

  virtual phase_result_sptrs_t simulate_phase_result(phase_result_sptr& state, bool &consistent)
  {
    HYDLA_LOGGER_PHASE("#*** Begin Simulator::simulate_phase_result ***");
    HYDLA_LOGGER_PHASE("%% current time:", state->current_time);
    HYDLA_LOGGER_PHASE("--- parent variable map ---\n", state->parent->variable_map);
    HYDLA_LOGGER_PHASE("--- parameter map ---\n", state->parameter_map);
    
    phase_result_sptrs_t phases; 
    bool has_next = false;
    is_safe_ = true;
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
          HYDLA_LOGGER_PHASE("%% begin point phase");
          phase_result_sptrs_t tmp = point_phase(ms, state, consistent);
          phases.insert(phases.begin(), tmp.begin(), tmp.end());
          break;
        }

        case IntervalPhase: 
        {
          HYDLA_LOGGER_PHASE("%% begin interval phase");
          phase_result_sptrs_t tmp = interval_phase(ms, state, consistent);
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

  
  /**
   * Point Phase時，モジュール集合の無矛盾性を調べるために使う関数
   * TODO:名前が変
   */
  virtual Phases point_phase(const module_set_sptr& ms, 
                           phase_result_sptr& state, bool& consistent) = 0;

  /**
   * Interval Phase時，モジュール集合の無矛盾性を調べるために使う関数
   * TODO:名前が変
   */
  virtual Phases interval_phase(const module_set_sptr& ms, 
                              phase_result_sptr& state, bool& consistent) = 0;

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
  
  bool is_safe_;
  const Opts     *opts_;
  
  variable_set_t *variable_set_;
  parameter_set_t *parameter_set_;
  variable_map_t *variable_map_;
};


} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_PHASE_SIMULATOR_H_
