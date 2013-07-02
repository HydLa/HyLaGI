#ifndef _VALUE_RANGE_H_
#define _VALUE_RANGE_H_

#include <string>
#include <vector>
#include "Value.h"

namespace hydla {
namespace simulator {

class ValueRange {
public:
  typedef boost::shared_ptr<Value> value_t;
  typedef struct Bound{
    value_t value;
    bool include_bound;
    Bound():include_bound(false){}
    Bound(const value_t& val, const bool& in):value(val), include_bound(in){}
  }bound_t;
  

  ValueRange(const value_t& val){set_unique(val);}
  ValueRange(){}

  /**
   * 完全な未定義値かどうか
   */
  bool undefined() const
  {
    return (lower_.size() == 0 && upper_.size() == 0);
  }
  
  /**
   * 一意に値が定まるかどうか
   * 数式的な比較はしておらず，
   * 1. 下限と上限がそれぞれ1つずつしかなく，
   * 2. 下限と上限の元となったvalueが同じものである
   * 場合のみtrueが返る．
   */
  bool is_unique() const
  {
    return lower_.size() == 1 && upper_.size() == 1 &&
      lower_[0].value.get() == upper_[0].value.get() &&
      lower_[0].include_bound && upper_[0].include_bound ;
  }
  
  /**
   * 一意に値が定まるものとする
   */
  void set_unique(const value_t& val)
  {
    set_upper_bound(val, true);
    set_lower_bound(val, true);
  }


  std::string get_string() const;

  uint get_lower_cnt()const{return lower_.size();}
  const bound_t& get_lower_bound() const{return get_lower_bound(0);}
  const bound_t& get_lower_bound(const uint& idx) const
    {assert(idx<lower_.size()); return lower_[idx];}

  uint get_upper_cnt()const{return upper_.size();}
  const bound_t& get_upper_bound() const{return get_upper_bound(0);}
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

    if(val && !val->undefined())
    { 
      lower_.push_back(bound_t(val, include));
    }
  }

  void add_upper_bound(const value_t& val, const bool& include)
  {
    if(val && !val->undefined())
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
};

std::ostream& operator<<(std::ostream& s, const ValueRange & val);


} // namespace simulator
} // namespace hydla

#endif // _VALUE_RANGE_H_
