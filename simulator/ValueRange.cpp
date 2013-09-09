#include "ValueRange.h"

namespace hydla {
namespace simulator {

std::ostream& operator<<(std::ostream& s, const ValueRange & val)
{
  return val.dump(s);
}


std::string ValueRange::get_string()const
{
  std::string tmp_str;
  if(is_unique()){
    tmp_str += lower_[0].value->get_string();
  }else{
    if(lower_.size() > 0)
    {
      for(uint i = 0; i < lower_.size(); i++)
      {
        const bound_t& l = lower_[i];
        tmp_str += l.include_bound?"[":"(";
        tmp_str += l.value->get_string();
        tmp_str += ", ";
      }
    }
    else
    {
      tmp_str += "(-inf, ";
    }

    if(upper_.size() > 0)
    {
      for(uint i = 0; i < upper_.size(); i++)
      {
        const bound_t& u = upper_[i];
        tmp_str += u.value->get_string();
        tmp_str += u.include_bound?"]":")";
        if(i != upper_.size() - 1)
        {
          tmp_str += ", ";
        }
      }
    }
    else
    {
      tmp_str += "inf)";
    }
  }
  return tmp_str;
}

} // namespace simulator
} // namespace hydla
