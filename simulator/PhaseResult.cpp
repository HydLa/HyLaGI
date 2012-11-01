#include "PhaseResult.h"
#include "DefaultParameter.h"

namespace hydla {
namespace simulator {


std::ostream& operator<<(std::ostream& s, const PhaseResult::parameter_map_t& pm){
  PhaseResult::parameter_map_t::const_iterator it  = pm.begin();
  PhaseResult::parameter_map_t::const_iterator end = pm.end();
  for(; it!=end; ++it) {
    s << *(it->first) << "(" << it->first << ") " << " <=> " << it->second << "\n";
  }
  return s;
}

std::ostream& operator<<(std::ostream& s, const PhaseResult::variable_map_t& vm){
  PhaseResult::variable_map_t::const_iterator it  = vm.begin();
  PhaseResult::variable_map_t::const_iterator end = vm.end();
  
  for(; it!=end; ++it) {
    if(it->second.get()){
      s << *(it->first) << "(" << it->first << ") " << " <=> " << *(it->second) << "\n";
    }else{
      s << *(it->first) << "(" << it->first << ") " << " <=> UNDEF\n";
    }
  }
  
  return s;
}

} // namespace simulator
} // namespace hydla 
