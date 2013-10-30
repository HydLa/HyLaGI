#ifndef _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VALUE_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VALUE_H_

#include <string>

namespace hydla {
namespace vcs {
namespace reduce {

struct REDUCEValue {

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

bool operator<(const REDUCEValue& lhs, const REDUCEValue& rhs);

std::ostream& operator<<(std::ostream& s, const REDUCEValue & v);


} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VALUE_H_
