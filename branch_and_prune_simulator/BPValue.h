#ifndef _INCLUDED_HYDLA_BP_SIMULATOR_BP_VALUE_H_
#define _INCLUDED_HYDLA_BP_SIMULATOR_BP_VALUE_H_

#include <ostream>

#include "rp_float.h"
#include "rp_interval.h"

namespace hydla {
namespace bp_simulator {

/**
 * •Ï”’l
 */
class BPValue
{
public:
  BPValue(int digits = 10) : display_digits_(digits) {
    rp_interval_set_real_line(this->val_);
  }

  BPValue(rp_interval i, int digits = 10) : display_digits_(digits) {
    rp_interval_copy(this->val_, i);
  }

  /* ƒ_ƒ“ƒv */
  std::ostream& dump(std::ostream& s) const
  {
    char tmp[255];
    rp_interval i;
    rp_interval_copy(i, this->val_);
    rp_interval_print(tmp, i,
      this->display_digits_, RP_INTERVAL_MODE_BOUND);
    s << tmp;
    return s;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const BPValue& v)
  {
    return v.dump(s);
  }

  void get(rp_interval i) const {
    rp_interval_copy(i, this->val_);
  }

  void set(rp_interval i) {
    rp_interval_copy(this->val_, i);
  }

  const int get_display_digits() const {
    return this->display_digits_;
  }

  void set_display_digits(int i) {
    if(i>0) this->display_digits_ = i;
  }

private:
  rp_interval val_;
  int display_digits_;
};

} // namespace bp_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_BP_SIMULATOR_BP_VALUE_H_
