#ifndef _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_
#define _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_

#include <ostream>
#include <string>

namespace hydla {
namespace simulator {

struct DefaultVariable {
  std::string  name;
  unsigned int derivative_count;

  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    for(size_t i=0; i<derivative_count; i++) s << "'";
    return s;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const DefaultVariable& v) 
  {
    return v.dump(s);
  }
};

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_