#ifndef _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_
#define _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_

#include "Simulator.h"
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
using namespace boost;
namespace hydla {
namespace simulator {

class ParallelSimulatorWorker;

class ParallelSimulator: public Simulator{
  public:
  typedef PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>           phase_result_const_sptr_t;
  typedef PhaseSimulator                                    phase_simulator_t;
  typedef phase_result_t::phase_result_sptr_t               phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;
  
  typedef SimulationTodo                                   simulation_phase_t;
  typedef boost::shared_ptr<SimulationTodo>                simulation_phase_sptr_t;
  typedef std::vector<simulation_phase_sptr_t>              simulation_phases_t;

  typedef phase_result_t::variable_map_t variable_map_t;
  typedef phase_result_t::variable_t     variable_t;
  typedef phase_result_t::parameter_t    parameter_t;
  typedef phase_result_t::value_t        value_t;
  typedef phase_result_t::parameter_map_t     parameter_map_t;
  
  typedef std::list<variable_t>                            variable_set_t;
  typedef std::list<parameter_t>                           parameter_set_t;
  typedef value_t                                          time_value_t;


  ParallelSimulator(Opts &opts);
  
  virtual ~ParallelSimulator();

  void push_phase(const simulation_phase_sptr_t& state);
  simulation_phase_sptr_t pop_phase();
  /**
   * スレッドを立ち上げ、シミュレーションを開始する
   */
  virtual phase_result_const_sptr_t simulate();

  /**
   * シミュレータの初期化を行う
   */
  virtual void initialize(const parse_tree_sptr& parse_tree);
  
  phase_result_const_sptr_t get_result_root();

  bool state_stack_is_empty();

  private:
  
  /**
   * シミュレーション中で使用される変数表の原型
   */
  variable_map_t variable_map_;

  
  /*
   * シミュレーション中に使用される変数と記号定数の集合
   */
  variable_set_t variable_set_;
  parameter_set_t parameter_set_;
  
  
  /**
   * シミュレーション対象となるパースツリー
   */
  parse_tree_sptr parse_tree_;

  thread_group thread_group_;

  std::vector<boost::shared_ptr<ParallelSimulatorWorker> > workers_;

};

} // simulator
} // hydla

#endif // _INCLUDED_HYDLA_PARALLEL_SIMULATOR_H_
