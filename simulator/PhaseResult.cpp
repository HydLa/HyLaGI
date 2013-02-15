#include "PhaseResult.h"
#include "DefaultParameter.h"

namespace hydla {
namespace simulator {


std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm){
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  for(; it!=end; ++it) {
    s << *(it->first) << "(" << it->first << ") " << " <=> " << it->second << "\n";
  }
  return s;
}

std::ostream& operator<<(std::ostream& s, const variable_map_t& vm){
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  
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
