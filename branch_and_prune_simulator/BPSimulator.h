#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_H_

#include "BPTypes.h"

// simulator
#include "TellCollector.h"
#include "AskCollector.h"

namespace hydla {
namespace bp_simulator {

class ConstraintStore;
class ConstraintStoreInterval;

class BPSimulator : public simulator_t
{
public:

  typedef struct Opts_ {
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
    ConstraintStore& constraint_store,
    hydla::simulator::TellCollector& tell_collector,
    hydla::simulator::positive_asks_t& positive_asks,
    hydla::simulator::negative_asks_t& negative_asks);

  bool do_interval_phase(const module_set_sptr& ms,
    const phase_state_const_sptr& state,
    ConstraintStoreInterval& constraitn_store,
    hydla::simulator::TellCollector& tell_collector,
    hydla::simulator::positive_asks_t& positive_asks,
    hydla::simulator::negative_asks_t& negative_asks);

  module_set_container_sptr msc_original_;
  module_set_container_sptr msc_no_init_;
  module_set_container_sptr msc_no_init_discreteask_;
  Opts opts_;
};

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_H_