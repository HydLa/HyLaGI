#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_BP_VALUE_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_BP_VALUE_H_

#include <ostream>

namespace hydla {
namespace bp_simulator {

/**
 * ïœêîíl
 */
struct BPValue
{
  // rp_interval?

  /* É_ÉìÉv */
  std::ostream& dump(std::ostream& s) const
  {
    s << "value";
    return s;
  }
    
  friend std::ostream& operator<<(std::ostream& s, 
                                  const BPValue & v)
  {
    return v.dump(s);
  }
};

} // namespace bp_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_BP_VALUE_H_
