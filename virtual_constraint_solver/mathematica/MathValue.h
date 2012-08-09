#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_

#include <string>

namespace hydla {
namespace vcs {
namespace mathematica {

struct MathValue {

  /**
   * 未定義値かどうか
   */
  bool is_undefined() const;

  /**
   * 文字列表現を取得する
   */
  std::string get_string() const;
  
  /**
   * とりあえず文字列をセット
   */
  void set(std::string);

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const;

  private:
  std::string str_; // 文字列（任意の式を扱える）
};

bool operator<(const MathValue& lhs, const MathValue& rhs);

std::ostream& operator<<(std::ostream& s, const MathValue & v);


} // namespace mathematica
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
