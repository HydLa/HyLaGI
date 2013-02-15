#ifndef _DEFAULT_PARAMETER_H_
#define _DEFAULT_PARAMETER_H_

#include <boost/shared_ptr.hpp>
#include <sstream>
#include "PhaseResult.h"
#include "DefaultVariable.h"

namespace hydla {
namespace simulator {

struct DefaultParameter{
  public:
  variable_t* original_variable_;
  phase_result_sptr_t introduced_phase_;

  DefaultParameter(variable_t* variable, phase_result_sptr_t& phase)
    :original_variable_(variable), introduced_phase_(phase)
  {
  }
  
  /**
   * 元となる変数を設定する
   */
  void set_variable(variable_t* const variable)
  {
    original_variable_ = variable;
  }
  
  /**
   * 元となった変数のポインタを取得する
   */
  variable_t* get_variable() const {return original_variable_;}

  
  std::string get_name(){return original_variable_->get_name();}
  int get_derivative_count(){return original_variable_->get_derivative_count();}
  int get_phase_id(){return introduced_phase_->id;}
  
  /**
   * 導入されたフェーズを設定する
   */
  void set_phase(const phase_result_sptr_t &phase)
  {
    introduced_phase_ = phase;
  }
  
  /**
   * 導入されたフェーズを取得する
   */
  phase_result_sptr_t get_phase() const {return introduced_phase_;}
  
  std::string get_name() const
  {
    std::stringstream strstr;
    std::string ret("parameter[" + original_variable_->name);
    strstr << ", " << original_variable_->derivative_count << ", " << introduced_phase_->id<< "]";
    ret += strstr.str();
    return ret;
  }
  
  /**
   * 構造体の値をダンプする
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << get_name();
    return s;
  }
  
  
  bool operator==(const DefaultParameter& rhs)
  {
    return (get_variable() == rhs.get_variable()) && (get_phase() == rhs.get_phase());
  }
  
  
  friend bool operator<(const DefaultParameter& lhs, 
                        const DefaultParameter& rhs)
  {
    return lhs.get_name() < rhs.get_name();
  }
};

std::ostream& operator<<(std::ostream& s, const DefaultParameter& p);

} // namespace simulator
} // namespace hydla 

#endif // _DEFAULT_PARAMETER_H_
