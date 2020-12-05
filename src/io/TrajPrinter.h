#pragma once

#include "PhaseResult.h"
#include "Simulator.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace hydla {
namespace io {

typedef simulator::PhaseResult phase_result_t;
typedef std::shared_ptr<const phase_result_t> phase_result_const_sptr_t;
using simulator::phase_result_sptr_t;
typedef std::vector<phase_result_sptr_t> phase_result_sptrs_t;
using simulator::parameter_map_t;
using simulator::value_t;
using simulator::variable_map_t;

/**
 * 解軌道の出力を担当するクラス
 */

class TrajPrinter {
public:
  typedef simulator::PhaseResult phase_result_t;
  typedef std::shared_ptr<const phase_result_t> phase_result_const_sptr_t;

  virtual ~TrajPrinter() {}

  /**
   * 解軌道木全体を出力する関数
   */
  virtual void output_result_tree(const phase_result_const_sptr_t &) const = 0;
  /**
   * 特定の1フェーズを出力する関数
   */
  virtual void output_one_phase(const phase_result_const_sptr_t &,
                                const std::string &prefix = "") const = 0;
};

} // namespace io
} // namespace hydla
