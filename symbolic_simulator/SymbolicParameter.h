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
   * 一意な文字列表現を返す
   */
  std::string get_name() const;
  
  /**
   * 文字列表現に対応する変数を返す
   */
  static simulator::DefaultVariable get_variable(const std::string &name);
  
  /**
   * 変数に対応するIDを増やす．新しい記号定数を導入する際に
   */
  static void increment_id(const simulator::DefaultVariable &variable);
  
  /**
   * 元となる変数を設定する
   */
  void set_variable(simulator::DefaultVariable variable);
  
  
  /**
   * 構造体の値をダンプする
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