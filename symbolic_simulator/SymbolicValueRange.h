#ifndef _SYMBOLIC_VALUE_RANGE_H_
#define _SYMBOLIC_VALUE_RANGE_H_

#include <string>
#include <vector>

#include "SymbolicValue.h"

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValueRange {
  
  typedef struct Bound{
    bool include_bound;
    SymbolicValue value;
    Bound():include_bound(false){}
  }bound_t;
  

  SymbolicValueRange();

  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;
  
  /**
   * �l���i��ʓI�Ɂj��ӂɒ�܂邩
   */
  bool is_unique() const;

  /**
   * ������\�����擾����
   */
  std::string get_string() const;
  
  const bound_t& get_lower_bound() const;
  const bound_t& get_upper_bound() const;

  /**
   * �V���Ȃ��̂��Z�b�g
   */
  void set_upper_bound(const SymbolicValue& val, const bool& include);
  void set_lower_bound(const SymbolicValue& val, const bool& include);
  
  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
  

  private:
  bound_t lower_, upper_;  
};

std::ostream& operator<<(std::ostream& s, const SymbolicValueRange & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
