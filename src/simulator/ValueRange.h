#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "Value.h"

namespace hydla {
namespace simulator {

class ValueRange {
public:
  typedef unsigned int uint;
  typedef Value value_t;
  typedef struct Bound{
    value_t value;
    bool include_bound;
    Bound():include_bound(false){}
    Bound(const value_t& val, bool in):value(val), include_bound(in){}
  }bound_t;
  


  ValueRange(const value_t &lower, bool low_include,
             const value_t &upper, bool up_include);
  ValueRange(const value_t &lower, const value_t &upper);
  ValueRange(const value_t &val){set_unique_value(val);}
  ValueRange(){}

  bool undefined() const
  {
    return (!unique() && lower_.size() == 0 && upper_.size() == 0);
  }
  
  bool unique() const
  {
    return !unique_value_.undefined();
  }
  
  /**
   * 一意に値を定める
   */
  void set_unique_value(const value_t& val)
  {
    unique_value_ = val;
    lower_.clear();
    upper_.clear();
  }

  value_t get_unique_value() const
  {
    if(!unique())
    {
      throw std::runtime_error(
        "ValueRange: " + get_string() + 
        "is not unique, but get_unique_value() is called");
    }
    return unique_value_;
  }

  std::string get_string() const;

  uint get_lower_cnt()const{return lower_.size();}
  const bound_t& get_lower_bound() const{assert(lower_.size() > 0); return get_lower_bound(0);}
  const bound_t& get_lower_bound(const uint& idx) const
    {assert(idx<lower_.size()); return lower_[idx];}

  uint get_upper_cnt()const{return upper_.size();}
  const bound_t& get_upper_bound() const{assert(upper_.size() > 0); return get_upper_bound(0);}
  const bound_t& get_upper_bound(const uint& idx) const
    {assert(idx<upper_.size()); return upper_[idx];}

  void set_upper_bound(const value_t& val, const bool& include)
  {
    upper_.clear();
    add_upper_bound(val, include);
  }

  void set_lower_bound(const value_t& val, const bool& include)
  {
    lower_.clear();
    add_lower_bound(val, include);
  }

  void add_lower_bound(const value_t& val, const bool& include)
  {
    if(!val.undefined())
    {
      lower_.push_back(bound_t(val, include));
    }
  }

  void add_upper_bound(const value_t& val, const bool& include)
  {
    if(!val.undefined())
    {
      upper_.push_back(bound_t(val, include));
    }
  }
  
  std::ostream& dump(std::ostream& s) const  
  {
    s << get_string();
    return s;
  }

  private:
  std::vector<bound_t> lower_, upper_;
  value_t unique_value_;
};

std::ostream& operator<<(std::ostream& s, const ValueRange & val);


} // namespace simulator
} // namespace hydla

