#ifndef _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
#define _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_

#include <string>

namespace hydla {
namespace vcs {
namespace mathematica {

struct MathValue {
  std::string str; // ������i�C�ӂ̎���������j

  /**
   * �f�[�^���_���v����
   */
  std::ostream& dump(std::ostream& s) const
  {
    s << str;
    return s;
  }

  friend bool operator<(const MathValue& lhs, 
                        const MathValue& rhs)
  {
    return lhs.str < rhs.str;
  }

  friend std::ostream& operator<<(std::ostream& s, 
                                  const MathValue & v)
  {
    return v.dump(s);
  }
};

} // namespace mathematica
} // namespace simulator
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_MATHEMATICA_MATHVALUE_H_
