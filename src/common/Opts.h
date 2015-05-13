#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

namespace hydla{

namespace symbolic_expression
{
class Node;
typedef boost::shared_ptr<Node> node_sptr;
}

typedef enum{
  DFS,
  BFS
}SearchMethod;

struct Opts {
  std::string mathlink;
  bool debug_mode;
  std::string max_time;
  bool approx;
  bool cheby;
  int epsilon_mode;
  bool nd_mode;
  bool static_generation_of_module_sets;
  bool interactive_mode;
  bool use_unsat_core;
  bool ha_convert_mode;
  bool ha_simulator_mode;
  bool dump_relation;
  bool profile_mode;
  bool parallel_mode;
  int parallel_number;
  bool reuse;
  bool dump_in_progress;
  bool stop_at_failure;
  bool ignore_warnings;
  int output_precision;
  std::string solver;
  symbolic_expression::node_sptr assertion;
  std::set<std::string> output_variables;
  int timeout;
  int timeout_phase;
  int timeout_case;
  int timeout_calc;
  int max_phase;
  SearchMethod search_method;
};

}
