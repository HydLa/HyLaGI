#pragma once

#include "TrajPrinter.h"

namespace hydla{
namespace output{

/**
 * 数式による出力を行う
 */

class SymbolicTrajPrinter: public TrajPrinter{
public:

  SymbolicTrajPrinter(const std::set<std::string>& output_variables, std::ostream& ostream);
  SymbolicTrajPrinter(const std::set<std::string>& output_variables);
  SymbolicTrajPrinter();

  void output_result_tree(const phase_result_const_sptr_t&)const;
  void output_one_phase(const phase_result_const_sptr_t&)const;

  std::string get_state_output(const phase_result_t& result) const;

  void output_parameter_map(const parameter_map_t& pm) const;

  void output_variable_map(std::ostream &stream, const variable_map_t& vm) const;

  void output_result_node(const phase_result_const_sptr_t &node,
    std::vector<std::string> &result, int &case_num, int &phase_num) const;

  void set_output_variables(const std::set<std::string>& ovs){output_variables_ = ovs;}

  virtual void set_epsilon_mode(hydla::simulator::backend_sptr_t back, hydla::simulator::Opts *op);
  hydla::simulator::backend_sptr_t backend_;
  hydla::simulator::Opts *opts_;
  void output_limit_of_time(std::ostream &stream, backend::Backend* backend_, const phase_result_t& result) const;
  void output_limits_of_variable_map(std::ostream &stream, backend::Backend* backend_, const phase_result_t& result, const variable_map_t& vm) const;

private:

  std::ostream& ostream_;
  std::set<std::string> output_variables_;
};

}// output
}// hydla
