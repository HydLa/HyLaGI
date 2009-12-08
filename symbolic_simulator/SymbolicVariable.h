#ifndef _INCLUDED_HYDLA_SYMBOLIC_VARIABLE_H_
#define _INCLUDED_HYDLA_SYMBOLIC_VARIABLE_H_

#include "DefaultVariable.h"

namespace hydla {
namespace symbolic_simulator {

struct SymbolicVariable //: public DefaultVariable
{
  std::string  name;
  unsigned int derivative_count;
  bool previous;

  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    for(size_t i=0; i<derivative_count; i++) s << "'";
    if(previous)
    return s;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicVariable& v) 
  {
    return v.dump(s);
  }
};

} //namespace symbolic_simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_SYMBOLIC_VARIABLE_H_
