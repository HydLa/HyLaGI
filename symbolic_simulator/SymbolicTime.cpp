#include "SymbolicTime.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{

SymbolicTime::SymbolicTime() :
  time_("0")
{}

SymbolicTime::SymbolicTime(const std::string& str)
{
  time_ = str;
}

SymbolicTime::~SymbolicTime()
{}

std::string SymbolicTime::get_string() const
{
  return time_;
}

void SymbolicTime::set(const std::string &str)
{
  time_ = str;
}


SymbolicTime& SymbolicTime::operator+=(const SymbolicTime& rhs)
{
  time_ += " + (" + rhs.time_ + ")";
  return *this;
}


SymbolicTime& SymbolicTime::operator-=(const SymbolicTime& rhs)
{
  time_ += " - (" + rhs.time_ + ")";
  return *this;
}



std::ostream& SymbolicTime::dump(std::ostream& s) const
{
  s << time_;
  return s;
}

std::ostream& operator<<(std::ostream& s, const SymbolicTime & t)
{
  return t.dump(s);
}


} // namespace symbolic_simulator
} // namespace hydla

