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
class RPTime
{
public:
  RPTime(int digits=10);

  /**
   * 与えられた文字列を元に作成
   */
  RPTime(std::string str, int digits=10);

  /* ダンプ */
  std::ostream& dump(std::ostream& s) const;

  friend std::ostream& operator<<(std::ostream& s, 
                                  const RPTime & v)
  {
    return v.dump(s);
  }

  RPTime& operator+=(const RPTime& t);

  RPTime& operator-=(const RPTime& t);

  double width() const
  {
    return sup_ - inf_;
  }

  // TODO: 面倒なのでpublicにしている
  double inf_, sup_;

private:
  int display_digits_;
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_TIME_H_
