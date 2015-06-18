#pragma once

#include <string>
#include <boost/shared_ptr.hpp>

namespace hydla{

namespace symbolic_expression
{
class Node;
typedef boost::shared_ptr<Node> node_sptr;
}

struct Opts {
  std::string mathlink;
  bool debug_mode;
  std::string max_time;
  int epsilon_mode;
  bool interval;
  bool ltl_model_check_mode;

  bool nd_mode;
  bool static_generation_of_module_sets;
  bool ha_convert_mode;
  bool ha_simulator_mode;
  bool dump_relation;
  bool dump_in_progress;
  bool stop_at_failure;
  bool ignore_warnings;
  symbolic_expression::node_sptr assertion;
  int max_phase;
};

}
