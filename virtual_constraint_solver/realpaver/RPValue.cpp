#include "RPValue.h"

#include "rp_interval.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RPValue::RPValue(int digits) :
display_digits_(digits)
{
  rp_interval i;
  rp_interval_set_real_line(i);
  this->inf_ = rp_binf(i);
  this->sup_ = rp_bsup(i);
}

RPValue::RPValue(double i, double s, int digits) :
display_digits_(digits)
{
  this->inf_ = i;
  this->sup_ = s;
}

  /* ƒ_ƒ“ƒv */
std::ostream& RPValue::dump(std::ostream& s) const
{
  char tmp[255];
  rp_interval i;
  rp_interval_set(i, this->inf_, this->sup_);
  rp_interval_print(tmp, i,
    this->display_digits_, RP_INTERVAL_MODE_BOUND);
  s << tmp;
  return s;
}

void RPValue::get(double& i, double& s) const
{
  i = this->inf_;
  s = this->sup_;
}

void RPValue::set(const double i, const double s)
{
  this->inf_ = i;
  this->sup_ = s;
}

} // namespace realpaver
} // namespace vcs
} // namespace hydla
