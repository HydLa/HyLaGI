#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_BP_VARIABLE_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_BP_VARIABLE_H_

#include "DefaultVariable.h"

namespace hydla {
namespace bp_simulator {

/*
struct BPVariable {
  std::string name;
  unsigned int derivative_count;

  bool previous;
  bool initial;

  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    if(previous) s << "-";
    if(initial) s << "_0";
    return s;
  }


  friend std::ostream& operator<<(std::ostream& s, 
                                  const BPVariable & v)
  {
    return v.dump(s);
  }
};
*/

/**
 * •Ï”–¼E‘®«
 */
typedef simulator::DefaultVariable BPVariable;

} //namespace bp_simulator
} // namespace hydla

#endif //_INCLUDED_HYDLA_BP_SIMULATOR_BP_VARIABLE_H_
