#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_

#include <string>

class MathLink;

namespace hydla {
namespace vcs {
namespace mathematica {

struct MathValue {
  std::string str; // ������i�C�ӂ̎���������j


  /**
   * ����`�l���ǂ���
   */
  bool is_undefined() const;

  /**
   * ���������_�`���̒l���擾����
   */
  std::string get_real_val(MathLink& ml, int precision) const;

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const;
};

bool operator<(const MathValue& lhs, const MathValue& rhs);

std::ostream& operator<<(std::ostream& s, const MathValue & v);


} // namespace mathematica
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
