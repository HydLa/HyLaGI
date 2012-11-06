#ifndef _HYDLA_OUTPUT_MATHEMATICA_TRAJ_PRINTER_H_
#define _HYDLA_OUTPUT_MATHEMATICA_TRAJ_PRINTER_H_

#include "TrajPrinter.h"

namespace hydla{
namespace output{

/**
 * Mathematicaå`éÆÇ≈ÇÃèoóÕÇçsÇ§
 */

class MathematicaTrajPrinter: public TrajPrinter{
public:
  typedef hydla::simulator::PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;
  typedef phase_result_t::phase_result_sptr_t                                 phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                                   phase_result_sptrs_t;
  typedef phase_result_t::variable_map_t variable_map_t;  
  typedef phase_result_t::parameter_map_t parameter_map_t;
  typedef hydla::simulator::PhaseResult::value_t value_t;

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

#endif // _HYDLA_OUTPUT_MATHEMATICA_TRAJ_PRINTER_H_