#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {

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
  

  std::vector<std::vector< std::string > > value_; //文字列の列の列（文字列のDNF形式になっている．∧でつながれたものを∨でつなぐ）
  std::string str_; // 文字列（任意の式を扱える）
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
