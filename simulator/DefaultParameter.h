#ifndef _DEFAULT_PARAMETER_H_
#define _DEFAULT_PARAMETER_H_

#include <boost/shared_ptr.hpp>
#include "PhaseState.h"

namespace hydla {
namespace simulator {

template<typename ValueType>
class DefaultParameter{
  public:
  typedef DefaultVariable variable_t;
  typedef PhaseState<ValueType> phase_state_t;
  typedef boost::shared_ptr<phase_state_t> phase_state_sptr_t;
  const variable_t* original_variable_;
  const phase_state_sptr_t introduced_phase_;

  DefaultParameter(const variable_t* variable = NULL, const phase_state_sptr_t& phase = phase_state_sptr_t())
    :original_variable_(variable), introduced_phase_(phase)
  {
  }
  
  /**
   * 元となる変数を設定する
   */
  void set_variable(const variable_t* variable)
  {
    original_variable_ = variable;
  }
  
  /**
   * 元となった変数のポインタを取得する
   */
  const variable_t* get_variable(){return original_variable_;}

  
  std::string get_name(){return original_variable_->get_name();}
  int get_derivative_count(){return original_variable_->get_derivative_count();}
  int get_phase_id(){return introduced_phase_->id;}
  
  /**
   * 導入されたフェーズを設定する
   */
  void set_phase(const phase_state_sptr_t &)
  {
    introduced_phase_ = phase;
  }
  
  /**
   * 導入されたフェーズを取得する
   */
  const phase_state_sptr_t get_phase(){return introduced_phase_;}
  
  std::string get_name() const
  {
    std::stringstream strstr;
    std::string ret("parameter(" + original_variable_->name);
    strstr << ", d:" << original_variable_->derivative_count << ", id:" << introduced_phase_->id<< ")";
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
  
  friend bool operator<(const DefaultParameter& lhs, 
                        const DefaultParameter& rhs)
  {
    return lhs.get_name() < rhs.get_name();
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const DefaultParameter& p)
  {
    return p.dump(s);
  }
};

} // namespace simulator
} // namespace hydla 

#endif // _DEFAULT_PARAMETER_H_