#ifndef _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_
#define _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_

#include <ostream>
#include <string>

namespace hydla {
namespace simulator {

struct DefaultVariable {
  std::string  name;
  int derivative_count;
  
  DefaultVariable(const std::string& n, const int& d):name(n), derivative_count(d){}
  DefaultVariable():name(""), derivative_count(0){}

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
    return lhs.derivative_count < rhs.derivative_count;
  }
  
  friend bool operator==(const DefaultVariable& lhs, 
                        const DefaultVariable& rhs)
  {
    return (lhs.name == rhs.name) && (lhs.derivative_count == rhs.derivative_count);
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const DefaultVariable& v) 
  {
    return v.dump(s);
  }
};



class VariableComparator { // simple comparison class
   public:
   bool operator()(const DefaultVariable* x,const DefaultVariable* y) const { return *x < *y; }
};

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_
