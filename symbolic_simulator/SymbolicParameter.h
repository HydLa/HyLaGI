#ifndef _SYMBOLIC_PARAMETER_H_
#define _SYMBOLIC_PARAMETER_H_

#include <ostream>
#include <string>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicParameter {
  std::string  name;

  std::string get_name() const
  {
    return name;
  }

  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    return s;
  }

  friend bool operator<(const SymbolicParameter& lhs, 
                        const SymbolicParameter& rhs)
  {
    return lhs.name < rhs.name;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicParameter& p) 
  {
    return p.dump(s);
  }
};

} // namespace symbolic_simulator
} // namespace hydla 

#endif // _SYMBOLIC_PARAMETER_H_
