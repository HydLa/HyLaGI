#include "MathValue.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace vcs{
namespace mathematica{


bool MathValue::is_undefined() const
{
  return str_.empty();
}


std::ostream& MathValue::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << str_;
  return s;
}


std::string MathValue::get_string() const{
  return str_;
};


void MathValue::set(std::string str){
  str_ = str;
};


bool operator<(const MathValue& lhs, 
               const MathValue& rhs)
{
  return lhs.get_string() < rhs.get_string();
}


std::ostream& operator<<(std::ostream& s, 
                         const MathValue & v)
{
  return v.dump(s);
}


} // namespace mathematica
} // namespace vcs
} // namespace hydla

