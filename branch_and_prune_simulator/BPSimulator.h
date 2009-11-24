#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_H_

#include "Simulator.h"

namespace hydla {
namespace bp_simulator {

  struct bp_variable {};
  struct bp_value {};
  struct bp_time {};

typedef simulator::VariableMap<bp_variable, bp_value> variable_map_t;
typedef simulator::PhaseState<variable_map_t, bp_time> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef simulator::Simulator<phase_state_t> simulator_t;

class BPSimulator : public simulator_t
{
public:

  BPSimulator();
  virtual ~BPSimulator();

  bool simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc);
//                Opts& opts);

//  virtual bool test_module_set(hydla::ch::module_set_sptr ms);


  /**
   * Point PhaseÇÃèàóù
   */
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);
  
  /**
   * Interval PhaseÇÃèàóù
   */
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);

private:
};

} // bp_simulator
} // hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_H_