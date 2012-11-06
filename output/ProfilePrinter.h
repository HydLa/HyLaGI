#ifndef _HYDLA_OUTPUT_PROFILE_PRINTER_H_
#define _HYDLA_OUTPUT_PROFILE_PRINTER_H_

#include "Types.h"

namespace hydla{
namespace output{

/**
 * �v���t�@�C�����O���ʂ̏o�͂�S������N���X
 * �Ƃ肠�������Ԃ���
 */

class ProfilePrinter{
public:
  /**
   * ���O���ؑS�̂��o�͂���֐�
   */
  virtual void print_profile(const entire_profile_t&) const = 0;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_PROFILE_PRINTER_H_