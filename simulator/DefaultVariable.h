#ifndef _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_
#define _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_

#include <ostream>
#include <string>

namespace hydla {
namespace simulator {

struct DefaultVariable {
  std::string  name;
  int derivative_count;

  std::string get_name() const
  {
    return name;
  }
  
  int get_derivative_count() const
  {
    return derivative_count;
  }

  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    for(int i=0; i<derivative_count; i++) s << "'";
    return s;
  }
  
  std::string get_string() const
  {
    std::string ret = name;
    ret.append(derivative_count, '\'');
    return ret;
  }

  friend bool operator<(const DefaultVariable& lhs, 
                        const DefaultVariable& rhs)
  {
    if(lhs.derivative_count == rhs.derivative_count) {
      return lhs.name < rhs.name;
    }
    return lhs.derivative_count - rhs.derivative_count < 0;
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
