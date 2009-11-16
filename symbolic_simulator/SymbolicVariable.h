#ifndef _INCLUDED_HYDLA_SYMBOLIC_VARIABLE_H_
#define _INCLUDED_HYDLA_SYMBOLIC_VARIABLE_H_

namespace hydla {
namespace symbolic_simulator {

typedef struct SymbolicVariable_ {
  std::string name;
  unsigned int derivative_count;
  bool previous;
  
  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    for(size_t i=0; i<derivative_count; i++) s << "'";
    if(previous) s << "-";
    return s;
  }
} SymbolicVariable;

} //namespace symbolic_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_SYMBOLIC_VARIABLE_H_
