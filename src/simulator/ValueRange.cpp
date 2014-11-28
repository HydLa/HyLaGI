#include "ValueRange.h"
#include "ValueNumerizer.h"

namespace hydla {
namespace simulator {

std::ostream& operator<<(std::ostream& s, const ValueRange & val)
{
  return val.dump(s);
}
ValueRange::ValueRange(const value_t& lower, bool low_include,
                       const value_t& upper, bool up_include)
{
  set_lower_bound(lower, low_include);
  set_upper_bound(upper, up_include);
}

ValueRange::ValueRange(const value_t& lower, const value_t& upper)
{
  set_lower_bound(lower, true);
  set_upper_bound(upper, true);
}

ValueRange ValueRange::get_numerized_range()const
{
  ValueRange numerized_range = *this;
  ValueNumerizer numerizer;
  if(unique())
  {
    numerizer.numerize(numerized_range.unique_value_);
  }
  else
  {
    for(auto &bound : numerized_range.lower_)
    {
      numerizer.numerize(bound.value);
    }
    for(auto &bound : numerized_range.upper_)
    {
      numerizer.numerize(bound.value);
    }
  }
  return numerized_range;
}

std::string ValueRange::get_string()const
{
  std::string tmp_str;
  if(unique()){
    tmp_str += unique_value_.get_string();
  }else{
    if(lower_.size() > 0)
    {
      for(uint i = 0; i < lower_.size(); i++)
      {
        const bound_t& l = lower_[i];
        tmp_str += l.include_bound?"[":"(";
        tmp_str += l.value.get_string();
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
        tmp_str += u.value.get_string();
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
