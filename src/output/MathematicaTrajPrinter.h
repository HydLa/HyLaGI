#pragma once

#include "TrajPrinter.h"

namespace hydla{
namespace output{

/**
 * Mathematica形式での出力を行う
 */

class MathematicaTrajPrinter: public TrajPrinter{
public:
  MathematicaTrajPrinter(
            const std::string& max_time,
            const std::set<std::string> output_variables_);
            
  
  void output_result_tree(const phase_result_const_sptr_t&)const;

  
  private:
  
  std::string max_time_;
  std::set<std::string> output_variables_;
};

}// output
}// hydla

