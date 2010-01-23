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

bool MathValue::is_undefined() const
{
  return str.empty();
}

std::string MathValue::get_real_val(MathLink& ml, int precision) const
{
  std::string ret;

  if(!is_undefined()) {
    ml.put_function("ToString", 2);  
    ml.put_function("N", 2);  
    ml.put_function("ToExpression", 1);
    ml.put_string(str);
    ml.put_integer(precision);
    ml.put_symbol("CForm");  
  
    ml.skip_pkt_until(RETURNPKT);
    ret = ml.get_string();
  }
  else {
    ret = "NaN";
  }
  return ret;
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

