#ifndef _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_
#define _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_


#include "ProfilePrinter.h"
#include <iostream>

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
  CsvProfilePrinter(std::ostream &stream = std::cout):output_stream_(stream){}
private:
  std::ostream& output_stream_;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_