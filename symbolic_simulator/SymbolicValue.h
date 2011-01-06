#ifndef _SYMBOLIC_VALUE_H_
#define _SYMBOLIC_VALUE_H_

#include <string>
#include <vector>

namespace hydla {
namespace symbolic_simulator {

struct SymbolicValue {

  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;

  /**
   * ������\�����擾����
   */
  std::string get_string() const;
  
  /**
   * �Ƃ肠������������Z�b�g
   */
  void set(std::string);

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;

  private:
  

  std::vector<std::vector< std::string > > value_; //������̗�̗�i�������DNF�`���ɂȂ��Ă���D�ȂłȂ��ꂽ���̂��ɂłȂ��j
  std::string str_; // ������i�C�ӂ̎���������j
};

bool operator<(const SymbolicValue& lhs, const SymbolicValue& rhs);

std::ostream& operator<<(std::ostream& s, const SymbolicValue & v);


} // namespace simulator
} // namespace hydla

#endif // _SYMBOLIC_VALUE_H_
