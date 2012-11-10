#ifndef _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_
#define _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_


#include "ProfilePrinter.h"
#include <iostream>

namespace hydla{
namespace output{

/**
 * プロファイリング結果の出力を担当するクラス
 * とりあえず時間だけ
 */

class CsvProfilePrinter{
public:
  /**
   * 解軌道木全体を出力する関数
   */
  virtual void print_profile(const entire_profile_t&) const;
  CsvProfilePrinter(std::ostream &stream = std::cout):output_stream_(stream){}
private:
  std::ostream& output_stream_;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_CSV_PROFILE_PRINTER_H_