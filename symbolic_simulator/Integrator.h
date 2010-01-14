#ifndef _INCLUDED_HYDLA_INTEGRATOR_H_
#define _INCLUDED_HYDLA_INTEGRATOR_H_

#include "Node.h"
#include "PacketSenderInterval.h"
#include "mathlink_helper.h"
#include "ParseTree.h"
#include "Types.h"
#include <boost/shared_ptr.hpp>
#include "ConstraintStoreBuilderInterval.h"
#include "SymbolicTime.h"

namespace hydla {
namespace symbolic_simulator {

struct NextPointPhaseState {
//    hydla::symbolic_simulator::SymbolicTime next_point_phase_time;
  std::string next_point_phase_time;
  hydla::symbolic_simulator::variable_map_t variable_map;
  bool is_max_time;
};

typedef std::vector<std::pair<std::string, int> > ask_list_t;

struct IntegrateResult {
  std::vector<NextPointPhaseState> states;
  ask_list_t ask_list;

  std::ostream& dump(std::ostream& s) const
  {
    s << "states: " << std::endl;
    std::vector<NextPointPhaseState>::const_iterator state_it = states.begin();
    while((state_it)!=states.end())
    {
      s << "  next_point_phase_time=" << (*state_it).next_point_phase_time << ", ";
      s << "  variable_map=" << (*state_it).variable_map;
      state_it++;
    }
    s << std::endl << "ask_list: ";
    ask_list_t::const_iterator ask_list_it = ask_list.begin();
    while((ask_list_it)!=ask_list.end())
    {
      s << "ask_type= " << (*ask_list_it).first << ", ";
      s << "ask_id= " << (*ask_list_it).second;
      ask_list_it++;
    }
    s << std::endl;
    return s;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const IntegrateResult & t)
  {
    return t.dump(s);
  }

};

class Integrator
{
public:
  Integrator(MathLink& ml, bool debug_mode = true);

  virtual ~Integrator();

  IntegrateResult integrate(
    hydla::symbolic_simulator::ConstraintStoreInterval& constraint_store,
    hydla::simulator::positive_asks_t& positive_asks,
    hydla::simulator::negative_asks_t& negative_asks,
    const hydla::symbolic_simulator::SymbolicTime& current_time,
    std::string max_time);
//  outFunc

private:
  MathLink& ml_;
  /// デバッグ出力をするかどうか
  bool               debug_mode_;

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_INTEGRATOR_H_

