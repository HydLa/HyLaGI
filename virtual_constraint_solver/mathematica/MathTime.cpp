#include "MathTime.h"

#include "mathlink_helper.h"

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


void MathTime::send_time(MathLink* ml)
{
  ml.put_function("ToExpression", 1);
  ml.put_string(time_);
}

void MathTime::receive_time(MathLink* ml)
{
  time_ = ml.get_string();
}

  MathTime& MathTime::operator+=(const SymbolicTime& rhs)
  {
    time_ += " + " + rhs.time_;
    return *this;
  }

  MathTime& MathTime::operator-=(const SymbolicTime& rhs)
  {
    time_ += " - " + rhs.time_;
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

