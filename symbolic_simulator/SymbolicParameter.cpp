#include "SymbolicParameter.h"

#include <cassert>
#include <sstream>

#include "Logger.h"


namespace hydla {
namespace symbolic_simulator{

  std::string SymbolicParameter::get_name() const
  {
    return name;//original_variable_->get_string();
  }

  /**
   * 構造体の値をダンプする
   */
  std::ostream& SymbolicParameter::dump(std::ostream& s) const
  {
    s << get_name();
    return s;
  }

  bool operator<(const SymbolicParameter& lhs, 
                        const SymbolicParameter& rhs)
  {
    return lhs.get_name() < rhs.get_name();
  }

  std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicParameter& p) 
  {
    return p.dump(s);
  }


} // namespace symbolic_simulator
} // namespace hydla