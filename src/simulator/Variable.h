#pragma once

#include <ostream>
#include <string>
#include <stdexcept>

#include "Node.h"

namespace hydla {
namespace simulator {

class Variable {
public:
  std::string  name;
  symbolic_expression::node_sptr index;
  int differential_count;
  
  Variable(const std::string& n, const int& d, const symbolic_expression::node_sptr i = symbolic_expression::node_sptr()):name(n), index(i), differential_count(d){}
  /**
   * create Variable from string (like x')
   */
  Variable(const std::string& n)
  {
    std::string::size_type index = n.find_first_of("'");
    if(index != std::string::npos)
    {
      name = n.substr(0, index);
      differential_count = n.length() - index;
      // check the correctness of given string
      for(;index < n.length(); index++)
      {
        if(n[index] != '\'') throw std::runtime_error("invalid name of variable: " + n);
      }
    }
    else
    {
      name = n;
      differential_count = 0;
    }
  }
  Variable():name(""), differential_count(0){}

  std::string get_name() const
  {
    std::string str = name;
    if(index != symbolic_expression::node_sptr())
    {
      str += "[" + symbolic_expression::get_infix_string(index) + "]";
    }
    return str;
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
    if(index != symbolic_expression::node_sptr()) s << "[" << symbolic_expression::get_infix_string(index) << "]";
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
      return lhs.get_name() < rhs.get_name();
    }
    return lhs.differential_count < rhs.differential_count;
  }
  
  friend bool operator==(const Variable& lhs, 
                        const Variable& rhs)
  {
    if(lhs.index == symbolic_expression::node_sptr() && rhs.index == symbolic_expression::node_sptr())
    {
      return (lhs.name == rhs.name) && (lhs.differential_count == rhs.differential_count);
    }
    else if(lhs.index == symbolic_expression::node_sptr() || rhs.index == symbolic_expression::node_sptr())
    {
      return false;
    }
    else
    {
      return (lhs.name == rhs.name) && (lhs.differential_count == rhs.differential_count) && lhs.index->is_same_struct(*rhs.index, true);
    }
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

