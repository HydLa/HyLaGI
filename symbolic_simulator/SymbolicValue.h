#ifndef _INCLUDED_HYDLA_SYMBOLIC_VALUE_H_
#define _INCLUDED_HYDLA_SYMBOLIC_VALUE_H_

namespace hydla {
namespace symbolic_simulator {

typedef struct SymbolicValue_ {

  /**
   * データをダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << "value";
    return s;
  }
} SymbolicValue;

} //namespace symbolic_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_SYMBOLIC_VALUE_H_
