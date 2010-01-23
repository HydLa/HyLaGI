#include "MathTime.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"

namespace hydla {
namespace vcs {
namespace mathematica {

MathTime::MathTime() :
  time_("0")
{}

MathTime::MathTime(const std::string& str)
{
  time_ = str;
}

MathTime::~MathTime()
{}


void MathTime::send_time(MathLink& ml) const
{
  HYDLA_LOGGER_DEBUG("MathTime::send_time : ", time_);
  ml.put_function("ToExpression", 1);
  ml.put_string(time_);
}

void MathTime::receive_time(MathLink& ml)
{
  time_ = ml.get_string();
}

std::string MathTime::get_real_val(MathLink& ml, int precision) const
{
  ml.put_function("ToString", 2);  
  ml.put_function("N", 2);  
  ml.put_function("ToExpression", 1);
  ml.put_string(time_);
  ml.put_integer(precision);
  ml.put_symbol("CForm");  
  
  ml.skip_pkt_until(RETURNPKT);
  return  ml.get_string();
}


MathTime& MathTime::operator+=(const MathTime& rhs)
{
  time_ += " + (" + rhs.time_ + ")";
  return *this;
}

MathTime& MathTime::operator-=(const MathTime& rhs)
{
  time_ += " - (" + rhs.time_ + ")";
  return *this;
}


std::ostream& MathTime::dump(std::ostream& s) const
{
  s << time_;
  return s;
}

std::ostream& operator<<(std::ostream& s, const MathTime & t)
{
  return t.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla

