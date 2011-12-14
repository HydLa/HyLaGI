#ifndef _SYMBOLIC_VALUE_RANGE_H_
#define _SYMBOLIC_VALUE_RANGE_H_

#include <string>
#include <vector>

#include "SymbolicValue.h"

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValueRange {
  
  typedef struct Bound{
    bool include_bound;
    SymbolicValue value;
    Bound():include_bound(false){}
  }bound_t;
  

  SymbolicValueRange();

  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;
  
  /**
   * 値が（定量的に）一意に定まるか
   */
  bool is_unique() const;

  /**
   * 文字列表現を取得する
   */
  std::string get_string() const;
  
  const bound_t& get_lower_bound() const;
  const bound_t& get_upper_bound() const;

  /**
   * 新たなものをセット
   */
  void set_upper_bound(const SymbolicValue& val, const bool& include);
  void set_lower_bound(const SymbolicValue& val, const bool& include);
  
  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;
  

  private:
  bound_t lower_, upper_;  
};

std::ostream& operator<<(std::ostream& s, const SymbolicValueRange & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
