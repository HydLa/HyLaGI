#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_

#include <string>

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

struct MathValue {
  std::string str; // 文字列（任意の式を扱える）


  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;

  /**
   * 浮動小数点形式の値を取得する
   */
  std::string get_real_val(MathLink& ml, int precision) const;

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;
};

bool operator<(const MathValue& lhs, const MathValue& rhs);

std::ostream& operator<<(std::ostream& s, const MathValue & v);


} // namespace mathematica
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
