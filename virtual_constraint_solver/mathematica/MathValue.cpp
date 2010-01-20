#include "MathValue.h"

#include <cassert>

#include "mathlink_helper.h"
#include "Logger.h"

namespace hydla {
namespace vcs {
namespace mathematica {

// MathValue::MathValue() :
//  str("")
// {}

// MathValue::~MathValue()
// {}

std::string MathValue::get_real_val(MathLink& ml, int precision) const
{
  ml.put_function("ToString", 1);  
  ml.put_function("N", 2);  
  ml.put_function("ToExpression", 1);
  ml.put_string(str);
  ml.put_integer(precision);
  
  ml.skip_pkt_until(RETURNPKT);
  return  ml.get_string();
}

std::ostream& MathValue::dump(std::ostream& s) const
{
  s << str;
  return s;
}

bool operator<(const MathValue& lhs, 
               const MathValue& rhs)
{
  return lhs.str < rhs.str;
}


std::ostream& operator<<(std::ostream& s, 
                         const MathValue & v)
{
  return v.dump(s);
}


} // namespace mathematica
} // namespace simulator
} // namespace hydla

