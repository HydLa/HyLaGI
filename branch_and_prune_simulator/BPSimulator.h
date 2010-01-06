#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_H_

#include "BPTypes.h"

// simulator
#include "TellCollector.h"
#include "AskCollector.h"

namespace hydla {
namespace bp_simulator {

class ConstraintStore;

class BPSimulator : public simulator_t
{
public:

  typedef struct Opts_ {
    bool debug_mode;
    std::string max_time;
    bool profile_mode;    // Ç»Ç¢
    bool parallel_mode;   // Ç»Ç¢
  } Opts;

  BPSimulator(const Opts& opts);
  virtual ~BPSimulator();


  /**
   * Point PhaseÇÃèàóù
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           const phase_state_const_sptr& state);
  
  /**
   * Interval PhaseÇÃèàóù
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state);

private:
  /**
   * èâä˙âªèàóù
   */
  virtual void do_initialize();
  bool do_point_phase(const module_set_sptr& ms,
    const phase_state_const_sptr& state,
    ConstraintStore& constraint_store,
    hydla::simulator::TellCollector& tell_collector,
    hydla::simulator::positive_asks_t& positive_asks,
    hydla::simulator::negative_asks_t& negative_asks);

  Opts opts_;
  std::string max_time_;
};

} // namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_H_