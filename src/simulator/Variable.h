#ifndef _INCLUDED_HYDLA_SIMULATOR_VARIABLE_H_
#define _INCLUDED_HYDLA_SIMULATOR_VARIABLE_H_

#include <ostream>
#include <string>

namespace hydla {
namespace simulator {

class Variable {
public:
  std::string  name;
  int differential_count;
  
  Variable(const std::string& n, const int& d):name(n), differential_count(d){}
  Variable():name(""), differential_count(0){}

  std::string get_name() const
  {
    return name;
  }
  
  int get_differential_count() const
  {
    return differential_count;
  }

  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << name;
    for(int i=0; i<differential_count; i++) s << "'";
    return s;
  }
  
  std::string get_string() const
  {
    std::string ret = name;
    ret.append(differential_count, '\'');
    return ret;
  }

  friend bool operator<(const Variable& lhs, 
                        const Variable& rhs)
  {
    if(lhs.differential_count == rhs.differential_count) {
      return lhs.name < rhs.name;
    }
    return lhs.differential_count < rhs.differential_count;
  }
  
  friend bool operator==(const Variable& lhs, 
                        const Variable& rhs)
  {
    return (lhs.name == rhs.name) && (lhs.differential_count == rhs.differential_count);
  }


  friend bool operator!=(const Variable& lhs, 
                        const Variable& rhs)
  {
    return !(lhs == rhs);
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const Variable& v) 
  {
    return v.dump(s);
  }
};



class VariableComparator { // simple comparison class
   public:
   bool operator()(const Variable x,const Variable y) const { return x < y; }
};

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_DEFAULT_VARIABLE_H_
