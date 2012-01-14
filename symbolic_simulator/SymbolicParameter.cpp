#include "SymbolicParameter.h"

#include <cassert>
#include <sstream>
#include <iostream>

#include "Logger.h"


namespace hydla {
namespace symbolic_simulator{

  std::map<const simulator::DefaultVariable, int> SymbolicParameter::id_map_;

  std::string SymbolicParameter::get_name() const
  {
    std::string ret(original_variable_.name);
    for(int i=0; i<original_variable_.derivative_count; i++){
      ret += "d";
    }
    for(int i=0; i<id_; i++){
      ret += "i";
    }
    return ret;
  }
 
  
  SymbolicParameter::SymbolicParameter(const simulator::DefaultVariable variable, const SymbolicValue time): original_variable_(variable), introduced_time_(time){
    id_ = id_map_[variable];
  }
  
  
  simulator::DefaultVariable SymbolicParameter::get_variable(const std::string &name){
    // 現状だとID無視されるから修正が必要
    std::string ret_name;
    int derivative_count;
    std::string::size_type pos = name.find_last_of('d');
    if(pos == std::string::npos)
    {
      derivative_count = 0;
      pos = name.length();
    }
    else
    {
      std::string::size_type prev_pos = pos;
      while(pos > 0 && name[pos] == 'd'){
        pos--;
      }
      derivative_count = prev_pos - pos;
    }
    ret_name = name.substr(0, pos + 1);
    
    
    return simulator::DefaultVariable(ret_name, derivative_count);
  }
  
  void SymbolicParameter::set_variable(simulator::DefaultVariable variable){
    original_variable_ = variable;
  }
  
  void SymbolicParameter::increment_id(const simulator::DefaultVariable &variable){
    // 間に合わせ感が否めない関数
    if(id_map_.find(variable) == id_map_.end()){
      id_map_[variable] = 0;
    }else{
      id_map_[variable]++;
    }
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