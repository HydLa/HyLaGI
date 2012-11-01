#ifndef _HYDLA_OUTPUT_OUTPUTTER_H_
#define _HYDLA_OUTPUT_OUTPUTTER_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"

namespace hydla{
namespace output{

/**
 * 処理系の結果の出力を担当するクラス
 */

class Outputter{
public:
  typedef hydla::simulator::PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;

  /**
   * 解軌道木全体を出力する関数
   */
  virtual void output_result_tree(const phase_result_const_sptr_t&) const = 0;
  /**
   * 特定の1フェーズを出力する関数
   */
  virtual void output_one_phase(const phase_result_const_sptr_t&) const = 0;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_OUTPUTTER_H_