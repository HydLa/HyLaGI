#include "RPTime.h"

#include "rp_interval.h"

namespace hydla {
namespace vcs {
namespace realpaver {

RPTime::RPTime(int digits) :
  inf_(0.0), sup_(0.0), display_digits_(digits)
{}

/**
 * 与えられた文字列を元に作成
 */
RPTime::RPTime(std::string str, int digits) :
  display_digits_(digits)
{
  rp_interval i;
  rp_interval_from_str(const_cast<char *>(str.c_str()), i);
  this->inf_ = rp_binf(i);
  this->sup_ = rp_bsup(i);
}

/* ダンプ */
std::ostream& RPTime::dump(std::ostream& s) const
{
  char tmp[255];
  rp_interval i;
  rp_interval_set(i, this->inf_, this->sup_);
  rp_interval_print(tmp, i,
    this->display_digits_, RP_INTERVAL_MODE_BOUND);
  s << tmp;
  return s;
}

/**
 * 本来の区間演算とは異なるので気をつけよう
 */
RPTime& RPTime::operator +=(const RPTime& t)
{
  //// now += next_timeのみを想定
  //// 相手のinf_を足す
  //rp_interval i, j, r;
  //rp_interval_set_point(i, this->inf_);
  //rp_interval_set(j, t.inf_, t.sup_);
  //rp_interval_add(r, i, j);
  //this->inf_ = rp_binf(r);
  //this->sup_ = rp_bsup(r);
  return *this;
}

/**
 * 本来の区間演算とは異なるので気をつけよう
 */
RPTime& RPTime::operator -=(const RPTime& t)
{
  // maxtime -= now のみを想定
  // 結果のinfにほしい値が入る
  return *this;
}

} // namespace realpaver
} // namespace vcs
} // namespace hydla