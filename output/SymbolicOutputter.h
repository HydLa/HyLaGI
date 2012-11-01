#ifndef _HYDLA_OUTPUT_SYMBOLIC_OUTPUTTER_H_
#define _HYDLA_OUTPUT_SYMBOLIC_OUTPUTTER_H_

#include "Outputter.h"

namespace hydla{
namespace output{

/**
 * 数式による出力を行う
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
  SymbolicOutputter();
  
  void output_result_tree(const phase_result_const_sptr_t&)const;
  void output_one_phase(const phase_result_const_sptr_t&)const;
  
  private:
  
  std::string get_state_output(const phase_result_t& result) const;
  
  void output_parameter_map(const parameter_map_t& pm) const;

  void output_variable_map(std::ostream &stream, const variable_map_t& vm) const;

  void output_result_node(const phase_result_const_sptr_t &node,
    std::vector<std::string> &result, int &case_num, int &phase_num) const;
  
  std::set<std::string> output_variables_;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_SYMBOLIC_OUTPUTTER_H_
