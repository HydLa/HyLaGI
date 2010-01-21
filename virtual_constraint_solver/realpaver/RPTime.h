#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_TIME_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_TIME_H_

#include <ostream>

namespace hydla {
namespace vcs {
namespace realpaver {
/**
 * ����
 * �����ƌo�ߎ��ԁH
 */
struct RPTime
{
  RPTime()
  {}

  /**
   * �^����ꂽ����������ɍ쐬
   */
  RPTime(std::string str)
  {}

  /* �_���v */
  std::ostream& dump(std::ostream& s) const
  {
    s << "time";
    return s;
  }
    
  friend std::ostream& operator<<(std::ostream& s, 
                                  const RPTime & v)
  {
    return v.dump(s);
  }
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_TIME_H_
