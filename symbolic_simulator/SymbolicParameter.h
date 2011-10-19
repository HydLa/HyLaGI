#ifndef _SYMBOLIC_PARAMETER_H_
#define _SYMBOLIC_PARAMETER_H_

#include <ostream>
#include <string>
#include "DefaultVariable.h"
#include "SymbolicValue.h"

namespace hydla {
namespace symbolic_simulator {

struct SymbolicParameter {
  std::string name;
  /*
  boost::shared_ptr<simulator::DefaultVariable> original_variable_;
  boost::shared_ptr<SymbolicValue> introduced_time_;
  int id;*/

  SymbolicParameter(const std::string &n):name(n){}
  SymbolicParameter(){}
  std::string get_name() const;
  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const;
  
  friend bool operator<(const SymbolicParameter& lhs, 
                        const SymbolicParameter& rhs);

  friend std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicParameter& p);
};

} // namespace symbolic_simulator
} // namespace hydla 

#endif // _SYMBOLIC_PARAMETER_H_