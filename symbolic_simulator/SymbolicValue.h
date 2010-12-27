#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {
  std::string str; // ������i�C�ӂ̎���������j


  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;

  /**
   * ���������_�`���̒l���擾����
   */
  //std::string get_real_val(MathLink& ml, int precision) const;

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
