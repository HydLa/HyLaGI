#include "TimeModifier.h"


namespace hydla{
namespace simulator{

TimeModifier::TimeModifier(backend::Backend &b):backend(b)
{
}

variable_map_t TimeModifier::apply_function(const std::string& function, const Value& time, const variable_map_t& map)
{
  variable_map_t result;
  for(auto var_entry : map)
  {
    result[var_entry.first] = apply_function(function, time, var_entry.second);
  }
  return result;
}


ValueRange TimeModifier::apply_function(const std::string& function, const Value& time, const ValueRange& range)
{
  ValueRange result_range;
  if(!range.undefined())
  {
    if(range.unique())
    {
      result_range.set_unique_value(
        apply_function(function, time, range.get_unique_value()));
    }
    else
    {
      for(uint i = 0; i < range.get_lower_cnt(); i++)
      {
        ValueRange::bound_t bd = range.get_lower_bound(i);
        result_range.add_lower_bound(
          apply_function(function, time, bd.value), bd.include_bound);
      }
      for(uint i = 0; i < range.get_upper_cnt(); i++)
      {

        ValueRange::bound_t bd = range.get_upper_bound(i);
        result_range.add_upper_bound(
          apply_function(function, time, bd.value), bd.include_bound);
      }
    }
  }
  return result_range;
}

Value TimeModifier::apply_function(const std::string& function, const Value& time, const Value& value)
{
  Value result_value;
  backend.call(function.c_str(), 2, "vltvlt", "vl", &value, &time, &result_value);
  return result_value;
}

variable_map_t TimeModifier::substitute_time(const Value& time, const variable_map_t& map){
  return apply_function("applyTime2Expr", time, map); 
}

ValueRange TimeModifier::substitute_time(const Value& time, const ValueRange& range){
  return apply_function("applyTime2Expr", time, range); 
}

Value TimeModifier::substitute_time(const Value& time, const Value& value){
  return apply_function("applyTime2Expr", time, value);
}


Value TimeModifier::shift_time(const Value& time, const Value& value){
  return apply_function("exprTimeShift", time, value);
}

ValueRange TimeModifier::shift_time(const Value& time, const ValueRange& range){
  return apply_function("exprTimeShift", time, range);
}

variable_map_t TimeModifier::shift_time(const Value& time, const variable_map_t &map){
  return apply_function("exprTimeShift", time, map);
}


}
}
