#include "ValueRange.h"
#include "ConstraintStore.h"

namespace hydla {
namespace simulator {

using namespace symbolic_expression;

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

ConstraintStore ValueRange::create_range_constraint(node_sptr to_be_compared)
{
  ConstraintStore ret;
  if(unique()){
    ret.add_constraint(constraint_t(new Equal(to_be_compared, unique_value_.get_node())));
  }else{
    if(lower_.size() > 0)
    {
      for(uint i = 0; i < lower_.size(); i++)
      {
        const bound_t& l = lower_[i];
        if(l.include_bound)ret.add_constraint(constraint_t(new LessEqual(l.value.get_node(), to_be_compared)));
        else ret.add_constraint(constraint_t(new Less(l.value.get_node(), to_be_compared)));
      }
    }

    if(upper_.size() > 0)
    {
      for(uint i = 0; i < upper_.size(); i++)
      {
        const bound_t& u = upper_[i];
        if(u.include_bound)ret.add_constraint(constraint_t(new LessEqual(to_be_compared, u.value.get_node())));
        else ret.add_constraint(constraint_t(new Less(to_be_compared, u.value.get_node())));        
      }
    }
  }
  return ret;
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
