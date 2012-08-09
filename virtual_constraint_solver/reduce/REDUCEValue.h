#ifndef _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VALUE_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VALUE_H_

#include <string>

namespace hydla {
namespace vcs {
namespace reduce {

struct REDUCEValue {

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

bool operator<(const REDUCEValue& lhs, const REDUCEValue& rhs);

std::ostream& operator<<(std::ostream& s, const REDUCEValue & v);


} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_REDUCE_VALUE_H_
