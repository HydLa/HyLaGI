#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_H_

//#include "BPTypes.h"

// simulator
#include "Simulator.h"
#include "TellCollector.h"
#include "AskCollector.h"

// virtual_consraint_solver
#include "../virtual_constraint_solver/realpaver/RPVariable.h"
#include "../virtual_constraint_solver/realpaver/RPValue.h"
#include "../virtual_constraint_solver/realpaver/RPTime.h"
#include "../virtual_constraint_solver/realpaver/RealPaverVCS.h"

#include "mathlink_helper.h"

namespace hydla {
namespace bp_simulator {

typedef hydla::vcs::realpaver::RPVariable bp_variable_t;
typedef hydla::vcs::realpaver::RPValue    bp_value_t;
typedef hydla::vcs::realpaver::RPTime     bp_time_t;

typedef hydla::simulator::VariableMap<bp_variable_t, bp_value_t> variable_map_t;
typedef hydla::simulator::PhaseState<bp_variable_t, bp_value_t, bp_time_t> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef hydla::simulator::Simulator<phase_state_t> simulator_t;

//class ConstraintStore;
//class ConstraintStoreInterval;

class BPSimulator : public simulator_t
{
public:

  typedef struct Opts_ {
    std::string mathlink;
    bool debug_mode;
    std::string max_time;
    bool profile_mode;    // ない
    bool parallel_mode;   // ない
  } Opts;

  BPSimulator(const Opts& opts);
  virtual ~BPSimulator();


  /**
   * Point Phaseの処理
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           const phase_state_const_sptr& state);
  
  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state);

private:
  /**
   * 初期化処理
   */
  virtual void do_initialize(const parse_tree_sptr& parse_tree);

  bool do_point_phase(const module_set_sptr& ms,
    const phase_state_const_sptr& state,
    hydla::vcs::realpaver::RealPaverVCS& vcs,
    hydla::simulator::TellCollector& tell_collector,
    hydla::simulator::expanded_always_t& expanded_always,
    hydla::simulator::positive_asks_t& positive_asks,
    hydla::simulator::negative_asks_t& negative_asks);

  void set_precision(double prec) {
    precision_ = prec;
  }

  double precision_; // デフォルト0.5
  module_set_container_sptr msc_original_, msc_no_init_, msc_no_init_discreteask_;
  Opts opts_;
  MathLink ml_;
};

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_H_
