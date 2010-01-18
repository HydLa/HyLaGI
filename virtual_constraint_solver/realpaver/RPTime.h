#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_BP_TIME_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_BP_TIME_H_

#include <ostream>

namespace hydla {
namespace bp_simulator {
/**
 * ����
 * �����ƌo�ߎ��ԁH
 */
struct BPTime
{
  /* �_���v */
  std::ostream& dump(std::ostream& s) const
  {
    s << "time";
    return s;
  }
    
  friend std::ostream& operator<<(std::ostream& s, 
                                  const BPTime & v)
  {
    return v.dump(s);
  }
};

} // namespace bp_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_BP_TIME_H_
