#ifndef _SIMULATOR_VALUE_H_
#define _SIMULATOR_VALUE_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "ValueVisitor.h"

#include <boost/operators.hpp>

namespace hydla {
namespace simulator {

class ValueVisitor;

class Value:
  public boost::additive<Value>
{  
  public:
  /**
   * 未定義値かどうか
   */
  virtual bool is_undefined() const = 0;

  /**
   * 文字列表現を取得する
   */
  virtual std::string get_string() const = 0;
  
  virtual void accept(ValueVisitor &) = 0;
  
  /**
   * Value同士の加算
   */
  Value& operator+=(const Value& rhs);

  /**
   * Value同士の減算
   */
  Value& operator-=(const Value& rhs);
  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const{
    if(is_undefined()) s << "UNDEF";
    else s << get_string();
    return s;
  }
};

bool operator<(const Value& lhs, const Value& rhs);

std::ostream& operator<<(std::ostream& s, const Value & v);


} // namespace simulator
} // namespace hydla

#endif // _SIMULATOR_VALUE_H_