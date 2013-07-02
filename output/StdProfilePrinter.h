#ifndef _HYDLA_OUTPUT_STD_PROFILE_PRINTER_H_
#define _HYDLA_OUTPUT_STD_PROFILE_PRINTER_H_


#include "ProfilePrinter.h"

namespace hydla{
namespace output{

/**
 * プロファイリング結果の出力を担当するクラス
 * とりあえず時間だけ
 */

class StdProfilePrinter{
public:
  /**
   * 解軌道木全体を出力する関数
   */
  virtual void print_profile(const entire_profile_t&) const;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_STD_PROFILE_PRINTER_H_