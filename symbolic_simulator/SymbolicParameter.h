#ifndef _SYMBOLIC_PARAMETER_H_
#define _SYMBOLIC_PARAMETER_H_

#include <ostream>
#include <string>
#include <map>
#include "DefaultVariable.h"
#include "SymbolicValue.h"

namespace hydla {
namespace symbolic_simulator {

class SymbolicParameter {
  public:
  
  simulator::DefaultVariable original_variable_;
  SymbolicValue introduced_time_;
  int id_;
  static std::map<const simulator::DefaultVariable, int> id_map_;

  SymbolicParameter(const simulator::DefaultVariable variable = simulator::DefaultVariable(), const SymbolicValue time = SymbolicValue());

  /**
   * ��ӂȕ�����\����Ԃ�
   */
  std::string get_name() const;
  
  /**
   * ������\���ɑΉ�����ϐ���Ԃ�
   */
  static simulator::DefaultVariable get_variable(const std::string &name);
  
  /**
   * �ϐ��ɑΉ�����ID�𑝂₷�D�V�����L���萔�𓱓�����ۂ�
   */
  static void increment_id(const simulator::DefaultVariable &variable);
  
  /**
   * ���ƂȂ�ϐ���ݒ肷��
   */
  void set_variable(simulator::DefaultVariable variable);
  
  
  /**
   * �\���̂̒l���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
  
  friend bool operator<(const SymbolicParameter& lhs, 
                        const SymbolicParameter& rhs);

  friend std::ostream& operator<<(std::ostream& s, 
                                  const SymbolicParameter& p);
};

} // namespace symbolic_simulator
} // namespace hydla 

#endif // _SYMBOLIC_PARAMETER_H_