#include "REDUCEValue.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace vcs{
namespace reduce{


bool REDUCEValue::is_undefined() const
{
  return str_.empty();
}


std::ostream& REDUCEValue::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << str_;
  return s;
}


std::string REDUCEValue::get_string() const{
  return str_;
};


void REDUCEValue::set(std::string str){
  str_ = str;
};


bool operator<(const REDUCEValue& lhs, 
               const REDUCEValue& rhs)
{
  return lhs.get_string() < rhs.get_string();
}


std::ostream& operator<<(std::ostream& s, 
                         const REDUCEValue & v)
{
  return v.dump(s);
}


} // namespace reduce
} // namespace vcs
} // namespace hydla

