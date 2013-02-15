#ifndef _HYDLA_OUTPUT_TRAJ_PRINTER_H_
#define _HYDLA_OUTPUT_TRAJ_PRINTER_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "PhaseResult.h"
#include "Simulator.h"

namespace hydla{
namespace output{


typedef hydla::simulator::PhaseResult                                       phase_result_t;
typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;
using hydla::simulator::phase_result_sptr_t;
typedef std::vector<phase_result_sptr_t >                                   phase_result_sptrs_t;
using hydla::simulator::variable_map_t; 
using hydla::simulator::parameter_map_t;
using hydla::simulator::parameter_set_t;
using hydla::simulator::value_t;

/**
 * ���O���̏o�͂�S������N���X
 */

class TrajPrinter{
public:
  typedef hydla::simulator::PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;

  /**
   * ���O���ؑS�̂��o�͂���֐�
   */
  virtual void output_result_tree(const phase_result_const_sptr_t&) const = 0;
  /**
   * �����1�t�F�[�Y���o�͂���֐�
   */
  virtual void output_one_phase(const phase_result_const_sptr_t&) const = 0;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_TRAJECTORY_PRINTER_H_