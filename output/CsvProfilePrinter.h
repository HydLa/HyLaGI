#ifndef _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_
#define _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_


#include "ProfilePrinter.h"

namespace hydla{
namespace output{

/**
 * �v���t�@�C�����O���ʂ̏o�͂�S������N���X
 * �Ƃ肠�������Ԃ���
 */

class CsvProfilePrinter{
public:
  /**
   * ���O���ؑS�̂��o�͂���֐�
   */
  virtual void print_profile(const entire_profile_t&) const;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_