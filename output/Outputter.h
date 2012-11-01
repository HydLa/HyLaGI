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
 * �����n�̌��ʂ̏o�͂�S������N���X
 */

class Outputter{
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

#endif // _HYDLA_OUTPUT_OUTPUTTER_H_