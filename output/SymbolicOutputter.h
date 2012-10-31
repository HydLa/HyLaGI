#ifndef _HYDLA_OUTPUT_SYMBOLIC_OUTPUTTER_H_
#define _HYDLA_OUTPUT_SYMBOLIC_OUTPUTTER_H_

#include "Outputter.h"

namespace hydla{
namespace output{

/**
 * êîéÆÇ…ÇÊÇÈèoóÕÇçsÇ§
 */

class SymbolicOutputter: public Outputter{
public:
  typedef hydla::simulator::PhaseResult                                       phase_result_t;
  typedef boost::shared_ptr<const phase_result_t>                             phase_result_const_sptr_t;
  typedef phase_result_t::phase_result_sptr_t                                 phase_result_sptr_t;
  typedef std::vector<phase_result_sptr_t >                                   phase_result_sptrs_t;
  typedef phase_result_t::variable_map_t variable_map_t;  
  typedef phase_result_t::parameter_map_t parameter_map_t;
  typedef hydla::simulator::PhaseResult::value_t value_t;

  SymbolicOutputter(const std::set<std::string>& output_variables);
  
  void output_result_tree(const phase_result_const_sptr_t&)const;
  
  std::string get_state_output(const phase_result_t& result, const bool& numeric, const bool& is_in_progress) const;

  private:
  
  void output_variable_labels(std::ostream &stream, const variable_map_t variable_map) const;
  
  //std::string get_state_output(const phase_result_t& result, const bool& numeric, const bool& is_in_progress) const;
  
  void output_parameter_map(const parameter_map_t& pm) const;

  void output_variable_map(std::ostream &stream, const variable_map_t& vm, const value_t& time, const bool& numeric) const;

  void output_result_node(const phase_result_const_sptr_t &node,
    std::vector<std::string> &result, int &case_num, int &phase_num) const;
  
  std::set<std::string> output_variables_;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_SYMBOLIC_OUTPUTTER_H_
