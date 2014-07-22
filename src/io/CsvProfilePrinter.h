#pragma once

#include "ProfilePrinter.h"
#include <iostream>

namespace hydla{
namespace io{

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
