#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_TIME_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_TIME_H_

#include <ostream>

namespace hydla {
namespace vcs {
namespace realpaver {
/**
 * 時刻
 * 時刻と経過時間？
 */
struct RPTime
{
  RPTime()
  {}

  /**
   * 与えられた文字列を元に作成
   */
  RPTime(std::string str)
  {}

  /* ダンプ */
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
