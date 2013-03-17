#ifndef _HYDLA_OUTPUT_SYMBOLIC_TRAJ_PRINTER_H_
#define _HYDLA_OUTPUT_SYMBOLIC_TRAJ_PRINTER_H_

#include "TrajPrinter.h"

namespace hydla{
namespace output{

/**
 * ”®‚É‚æ‚éo—Í‚ğs‚¤
 */

class SymbolicTrajPrinter: public TrajPrinter{
public:

  SymbolicTrajPrinter(const std::set<std::string>& output_variables);
  SymbolicTrajPrinter();
  
  void output_result_tree(const phase_result_const_sptr_t&)const;
  void output_one_phase(const phase_result_const_sptr_t&)const;
  
  std::string get_state_output(const phase_result_t& result) const;
  
  void output_parameter_map(const parameter_map_t& pm) const;
  void output_parameter_set(const parameter_set_t& ps) const;

  void output_variable_map(std::ostream &stream, const variable_map_t& vm) const;

  void output_result_node(const phase_result_const_sptr_t &node,
    std::vector<std::string> &result, int &case_num, int &phase_num) const;
  
  void set_output_variables(const std::set<std::string>& ovs){output_variables_ = ovs;}
  
private:
  
  std::set<std::string> output_variables_;
};

}// output
}// hydla

#endif // _HYDLA_OUTPUT_SYMBOLIC_TRAJ_PRINTER_H_
