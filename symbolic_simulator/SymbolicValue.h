#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {
  std::string str; // 文字列（任意の式を扱える）


  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;

  /**
   * 浮動小数点形式の値を取得する
   */
  //std::string get_real_val(MathLink& ml, int precision) const;

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
