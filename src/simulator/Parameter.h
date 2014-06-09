#pragma once

#include <boost/shared_ptr.hpp>
#include <sstream>
#include "PhaseResult.h"
#include "Variable.h"

namespace hydla {
namespace simulator {

class Parameter{
  public:

  Parameter(const variable_t &variable, const phase_result_sptr_t &phase)
  :variable_name_(variable.get_name()), differential_count_(variable.get_differential_count()), phase_id_(phase->id)
  {
  }

  Parameter(const std::string &name, int diff_cnt, int id)
  :variable_name_(name), differential_count_(diff_cnt), phase_id_(id)
  {
  }
  
  std::string get_name()const {return variable_name_;}
  int get_differential_count()const {return differential_count_;}
  int get_phase_id()const {return phase_id_;}
 
 
  void set_name(const std::string &str) {variable_name_ = str;}
  void set_differential_count(int d) {differential_count_ = d;}
  void set_phase_id(int p) {phase_id_ = p;}
   
  std::string to_string() const
  {
    std::stringstream strstr;
    std::string ret("parameter[" + variable_name_);
    strstr << ", " << differential_count_ << ", " << phase_id_ << "]";
    ret += strstr.str();
    return ret;
  }
  
  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << to_string();
    return s;
  }
  
    
  friend bool operator<(const Parameter& lhs, 
                        const Parameter& rhs)
  {
    return lhs.to_string() < rhs.to_string();
  }
  
  private:
  std::string variable_name_;
  int differential_count_;
  int phase_id_;
};

std::ostream& operator<<(std::ostream& s, const Parameter& p);

} // namespace simulator
} // namespace hydla 

