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
class RPTime
{
public:
  RPTime(int digits=10);

  /**
   * �^����ꂽ����������ɍ쐬
   */
  RPTime(std::string str, int digits=10);

  /* �_���v */
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

  // TODO: �ʓ|�Ȃ̂�public�ɂ��Ă���
  double inf_, sup_;

private:
  int display_digits_;
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_TIME_H_
