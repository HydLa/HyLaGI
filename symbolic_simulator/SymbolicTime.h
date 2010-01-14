#ifndef _INCLUDED_HYDLA_SYMBOLIC_TIME_H_
#define _INCLUDED_HYDLA_SYMBOLIC_TIME_H_

#include <iostream>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicTime {
  std::string str; // 文字列（任意の式を扱える）

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << str;
    return s;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicTime & t)
  {
    return t.dump(s);
  }
};

} //namespace symbolic_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_SYMBOLIC_TIME_H_

