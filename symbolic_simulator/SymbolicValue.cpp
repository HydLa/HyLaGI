#include "SymbolicValue.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{


bool SymbolicValue::is_undefined() const
{
  return str.empty();
}


std::ostream& SymbolicValue::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << str;
  return s;
}

bool operator<(const SymbolicValue& lhs, 
               const SymbolicValue& rhs)
{
  return lhs.str < rhs.str;
}


std::ostream& operator<<(std::ostream& s, 
                         const SymbolicValue & v)
{
  return v.dump(s);
}


} // namespace symbolic_simulator
} // namespace hydla

