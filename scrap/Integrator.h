#ifndef _INCLUDED_HYDLA_INTEGRATOR_H_
#define _INCLUDED_HYDLA_INTEGRATOR_H_

#include "Node.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include "Types.h"
#include "SymbolicTime.h"
#include "ConstraintStoreBuilderInterval.h"
#include "PacketSender.h"

#include <boost/shared_ptr.hpp>

namespace hydla {
namespace symbolic_simulator {

struct NextPointPhaseState {
  hydla::symbolic_simulator::SymbolicTime next_point_phase_time;
  hydla::symbolic_simulator::variable_map_t variable_map;
  bool is_max_time;
};

struct IntegrateResult {
  typedef std::vector<std::pair<hydla::simulator::AskState, int> > ask_list_t;
  
  std::vector<NextPointPhaseState> states;
  ask_list_t ask_list;

  std::ostream& dump(std::ostream& s) const;

  friend std::ostream& operator<<(std::ostream& s, 
                                  const IntegrateResult & t)
  {
    return t.dump(s);
  }
};

class Integrator
{
public:
  typedef hydla::symbolic_simulator::SymbolicTime time_t;

  Integrator(MathLink& ml);

  virtual ~Integrator();

  void integrate(
    IntegrateResult& integrate_result,
    hydla::symbolic_simulator::ConstraintStoreInterval& constraint_store,
    const hydla::simulator::positive_asks_t& positive_asks,
    const hydla::simulator::negative_asks_t& negative_asks,
    const time_t& current_time,
    const time_t& max_time);
//  outFunc

private:
  MathLink& ml_;
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_INTEGRATOR_H_

