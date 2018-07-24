#include "ValueModifier.h"

using namespace std;

namespace hydla{
namespace simulator{

using uint = unsigned;

ValueModifier::ValueModifier(backend::Backend &b):backend(b)
{
}

variable_map_t ValueModifier::apply_function(const std::string& function, const Value& time, const variable_map_t& map)
{
  variable_map_t result;
  for(auto var_entry : map)
  {
    result[var_entry.first] = apply_function(function, time, var_entry.second);
  }
  return result;
}


ValueRange ValueModifier::apply_function(const std::string& function, const Value& time, const ValueRange& range)
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

Value ValueModifier::apply_function(const std::string& function, const Value& time, const Value& value)
{
  Value result_value;
  backend.call(function.c_str(), false, 2, "vltvlt", "vl", &value, &time, &result_value);
  return result_value;
}


variable_map_t ValueModifier::apply_function(const std::string& function, const variable_map_t& map, const string &fmt)
{
  variable_map_t result;
  for(auto var_entry : map)
  {
    result[var_entry.first] = apply_function(function, var_entry.second, fmt);
  }
  return result;
}


ValueRange ValueModifier::apply_function(const std::string& function, const ValueRange& range, const string &fmt)
{
  ValueRange result_range;
  if(!range.undefined())
  {
    if(range.unique())
    {
      result_range.set_unique_value(
        apply_function(function, range.get_unique_value(), fmt));
    }
    else
    {
      for(uint i = 0; i < range.get_lower_cnt(); i++)
      {
        ValueRange::bound_t bd = range.get_lower_bound(i);
        result_range.add_lower_bound(
          apply_function(function, bd.value, fmt), bd.include_bound);
      }
      for(uint i = 0; i < range.get_upper_cnt(); i++)
      {

        ValueRange::bound_t bd = range.get_upper_bound(i);
        result_range.add_upper_bound(
          apply_function(function, bd.value, fmt), bd.include_bound);
      }
    }
  }
  return result_range;
}

Value ValueModifier::apply_function(const std::string& function, const Value& value, const string &fmt)
{
  Value result_value;
  backend.call(function.c_str(), false, 1, fmt.empty()?"vlt":fmt.c_str(), "vl", &value, &result_value);
  return result_value;
}


variable_map_t ValueModifier::substitute_time(const Value& time, const variable_map_t& map){
  return apply_function("applyTime2Expr", time, map); 
}

ValueRange ValueModifier::substitute_time(const Value& time, const ValueRange& range){
  return apply_function("applyTime2Expr", time, range); 
}

Value ValueModifier::substitute_time(const Value& time, const Value& value){
  return apply_function("applyTime2Expr", time, value);
}


Value ValueModifier::shift_time(const Value& time, const Value& value){
  return apply_function("exprTimeShift", time, value);
}

ValueRange ValueModifier::shift_time(const Value& time, const ValueRange& range){
  return apply_function("exprTimeShift", time, range);
}

variable_map_t ValueModifier::shift_time(const Value& time, const variable_map_t &map){
  return apply_function("exprTimeShift", time, map);
}


}
}
