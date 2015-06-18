#pragma once

#include "TrajPrinter.h"
#include "LTLNode.h"
#include "PropertyNode.h"

namespace hydla{
namespace io{

class SymbolicTrajPrinter: public TrajPrinter{
public:

  SymbolicTrajPrinter(simulator::backend_sptr_t b, std::ostream& ostream = std::cout, bool numerize_par = false);

  void output_result_tree(const phase_result_const_sptr_t&)const;
  void output_one_phase(const phase_result_const_sptr_t&)const;

  std::string get_state_output(const phase_result_t& result) const;
  

  void output_parameter_map(const parameter_map_t& pm) const;

  void output_variable_map(std::ostream &stream, const variable_map_t& vm) const;

  void output_result_node(const phase_result_const_sptr_t &node,
    std::vector<std::string> &result, int &case_num, int &phase_num) const;


  void output_property_automaton(PropertyNode* node);
  void dump_property_automaton(PropertyNode* node);
  void output_ltl_node(LTLNode* node);
  void dump_ltl_node(LTLNode* node);

  virtual void set_epsilon_mode(simulator::backend_sptr_t back, bool flag);
  void output_limit_of_time(std::ostream &stream, backend::Backend* backend_, const phase_result_t& result) const;
  void output_limits_of_variable_map(std::ostream &stream, backend::Backend* backend_, const phase_result_t& result, const variable_map_t& vm) const;

private:

  void output_inconsistent_constraints(std::ostream &stream, const phase_result_t &phase)const;

  std::ostream& ostream;
  simulator::backend_sptr_t backend;
  bool numerize_parameter;
  bool epsilon_mode_flag = false;
};

}// output
}// hydla
