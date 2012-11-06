#ifndef _HYDLA_OUTPUT_STD_PROFILE_PRINTER_H_
#define _HYDLA_OUTPUT_STD_PROFILE_PRINTER_H_


#include "ProfilePrinter.h"

namespace hydla{
namespace output{

/**
 * �v���t�@�C�����O���ʂ̏o�͂�S������N���X
 * �Ƃ肠�������Ԃ���
 */

class StdProfilePrinter{
public:
  /**
   * ���O���ؑS�̂��o�͂���֐�
   */
  virtual void print_profile(const entire_profile_t&) const;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_STD_PROFILE_PRINTER_H_