#ifndef _INCLUDED_HYDLA_VCS_REALPAVER_RP_VALUE_H_
#define _INCLUDED_HYDLA_VCS_REALPAVER_RP_VALUE_H_

#include <ostream>

namespace hydla {
namespace vcs {
namespace realpaver {

/**
 * ïœêîíl
 */
class RPValue
{
public:
  RPValue(int digits = 10);

  RPValue(double inf, double sup, int digits = 10);

  /* É_ÉìÉv */
  std::ostream& dump(std::ostream& s) const;

  friend std::ostream& operator<<(std::ostream& s, const RPValue& v)
  {
    return v.dump(s);
  }

  void get(double& i, double& s) const;

  void set(const double i, const double s);

  const int get_display_digits() const
  {
    return this->display_digits_;
  }

  void set_display_digits(int i) 
  {
    if(i>0) this->display_digits_ = i;
  }

private:
  //boost::scoped_ptr<rp_interval> val_;
  double inf_, sup_;
  int display_digits_;
};

} // namespace realpaver
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REALPAVER_RP_VALUE_H_
