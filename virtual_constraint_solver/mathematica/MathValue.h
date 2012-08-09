#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_

#include <string>

namespace hydla {
namespace vcs {
namespace mathematica {

struct MathValue {

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
  std::string str_; // ������i�C�ӂ̎���������j
};

bool operator<(const MathValue& lhs, const MathValue& rhs);

std::ostream& operator<<(std::ostream& s, const MathValue & v);


} // namespace mathematica
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
