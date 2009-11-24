#include "BPSimulator.h"

#include <iostream>

#include "ModuleSet.h"

namespace hydla {
namespace bp_simulator {

  BPSimulator::BPSimulator(){}

  BPSimulator::~BPSimulator(){}

  bool BPSimulator::simulate(boost::shared_ptr<hydla::ch::ModuleSetContainer> msc)
  {
    simulator_t::simulate(msc);
    return true;
  }

  /**
   * Point PhaseÇÃèàóù
   */
  bool BPSimulator::point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state)
  {
    bool debug_mode_ = true;
    if(debug_mode_) {
      std::cout << "#***** bagin point phase *****\n";
      std::cout << "#** module set **\n";
      std::cout << ms->get_name() << std::endl;
      std::cout << ms->get_tree_dump() << std::endl;
    }
    return true;
  }
  
  /**
   * Interval PhaseÇÃèàóù
   */
  bool BPSimulator::interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state)
  {
    return true;
  }

} // bp_simulator
} // hydla
