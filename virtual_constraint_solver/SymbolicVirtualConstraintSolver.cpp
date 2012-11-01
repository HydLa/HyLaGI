#include "SymbolicVirtualConstraintSolver.h"


namespace hydla{
namespace vcs{

std::ostream& operator<<(std::ostream& s, const SymbolicVirtualConstraintSolver::variable_range_map_t& vm){
  SymbolicVirtualConstraintSolver::variable_range_map_t::const_iterator it  = vm.begin();
  
  for(; it!=vm.end(); ++it) {
    s << *(it->first) << "(" << it->first << ") " << " <=> " << it->second << "\n";
  }
  
  return s;
}


} //namespace vcs
} //namespace hydla 