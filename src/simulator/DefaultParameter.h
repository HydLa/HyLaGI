#ifndef _DEFAULT_PARAMETER_H_
#define _DEFAULT_PARAMETER_H_

#include <boost/shared_ptr.hpp>
#include <sstream>
#include "PhaseResult.h"
#include "DefaultVariable.h"

namespace hydla {
namespace simulator {

class DefaultParameter{
  public:

  DefaultParameter(const variable_t &variable, const phase_result_sptr_t &phase)
  :variable_name_(variable.get_name()), derivative_count_(variable.get_derivative_count()), phase_id_(phase->id)
  {
  }

  DefaultParameter(const std::string &name, int derivative, int id)
  :variable_name_(name), derivative_count_(derivative), phase_id_(id)
  {
  }
  
  std::string get_name()const {return variable_name_;}
  int get_derivative_count()const {return derivative_count_;}
  int get_phase_id()const {return phase_id_;}
 
 
  void set_name(const std::string &str) {variable_name_ = str;}
  void set_derivative_count(int d) {derivative_count_ = d;}
  void set_phase_id(int p) {phase_id_ = p;}
   
  std::string to_string() const
  {
    std::stringstream strstr;
    std::string ret("parameter[" + variable_name_);
    strstr << ", " << derivative_count_ << ", " << phase_id_ << "]";
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
  
    
  friend bool operator<(const DefaultParameter& lhs, 
                        const DefaultParameter& rhs)
  {
    return lhs.to_string() < rhs.to_string();
  }
  
  private:
  std::string variable_name_;
  int derivative_count_;
  int phase_id_;
};

std::ostream& operator<<(std::ostream& s, const DefaultParameter& p);

} // namespace simulator
} // namespace hydla 

#endif // _DEFAULT_PARAMETER_H_
