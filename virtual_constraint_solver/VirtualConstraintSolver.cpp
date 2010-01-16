#include "VirtualConstraintSolver.h

#include <boost/foreach.hpp>

namespace hydla {
namespace vcs {

VirtualConstraintSolver::VirtualConstraintSolver()
{}

virtual VirtualConstraintSolver::~VirtualConstraintSolver()
{}

std::ostream& operator<<(std::ostream& s, 
                         const VirtualConstraintSolver::integrate_result_t & t)
{
  s << "#*** integrate result ***\n";

  BOOST_FOREACH(next_phase_state_list_t::value_type& i, t.states)
  {
    s << "---- next_phase_state ----"
    s << "- time -\n" 
      << i.time 
      << "\n"
      << "- variable_map -" 
      << i.variable_map 
      << "\n";
  }

  s << std::endl << "---- changed asks ----\n";
  BOOST_FOREACH(changed_asks_t::value_type& i, t.changed_asks)
  {
    s << "ask_type : "
      << ask_list_it->first 
      << ", "
      << "ask_id : " << ask_list_it->second
      << "\n";
  }
  return s;

}

} //namespace simulator
} //namespace hydla 
"
