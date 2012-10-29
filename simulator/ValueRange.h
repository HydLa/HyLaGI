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
    bool include_bound;
    value_t value;
    Bound():include_bound(false){}
  }bound_t;
  

  ValueRange():is_unique_(false){}

  /**
   * 未定義値かどうか
   */
  bool is_undefined() const
  {
    return (lower_.value.get() && upper_.value.get());
  }
  
  /**
   * 一意に値が定まるかどうか
   */
  bool is_unique() const
  {
    return is_unique_;
  }
  
  /**
   * 一意に値が定まるものとする
   */
  void set_unique(const value_t& val)
  {
    lower_.value = upper_.value = val;
    is_unique_ = true;
  }


  /**
   * 文字列表現を取得する
   */
  std::string get_string() const
  {
    std::string tmp_str;
    if(is_unique()){
      tmp_str += lower_.value->get_string();
    }else{
      if(lower_.include_bound){
        tmp_str += "[";
      }
      else{
        tmp_str += "(";
      }
      if(lower_.value.get() && !lower_.value->is_undefined()){
        tmp_str += lower_.value->get_string();
      }else{
        tmp_str += "-inf";
      }
      
      tmp_str += ", ";
      
      if(upper_.value.get() && !upper_.value->is_undefined()){
        tmp_str += upper_.value->get_string();
      }else{
        tmp_str += "inf";
      }
      
      if(upper_.include_bound){
        tmp_str += "]";
      }
      else{
        tmp_str += ")";
      }
    }
    return tmp_str;
  }
  
  /**
   * 下限を取得
   */
  const bound_t& get_lower_bound() const{return lower_;}
  /**
   * 上限を取得
   */
  const bound_t& get_upper_bound() const{return upper_;}

  /**
   * 上限に新たなものをセット
   * この関数を呼び出すと，実際の上限下限に関わらず値が一意に定まらないものとして扱われる
   */
  void set_upper_bound(value_t val, const bool& include)
  {
    upper_.value = val;
    upper_.include_bound = include;
    is_unique_ = false;
  }

  /**
   * 下限に新たなものをセット
   * この関数を呼び出すと，実際の上限下限に関わらず値が一意に定まらないものとして扱われる
   */
  void set_lower_bound(value_t val, const bool& include)
  {
    lower_.value = val;
    lower_.include_bound = include;
    is_unique_ = false;
  }
  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const  
  {
    s << get_string();
    return s;
  }

  private:
  bound_t lower_, upper_;
  bool is_unique_;
};

std::ostream& operator<<(std::ostream& s, const ValueRange & val);


} // namespace simulator
} // namespace hydla

#endif // _VALUE_RANGE_H_
