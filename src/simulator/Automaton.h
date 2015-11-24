#pragma once
#include "PhaseResult.h"
#include "Backend.h"
#include <iostream>
#include <vector>
#include <string>
#include "Variable.h"
#include "ValueRange.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ConstraintStore.h"
#include "Parameter.h"

class Automaton;

typedef std::vector<Automaton*> automaton_node_list_t;
typedef std::vector<automaton_node_list_t> automaton_path_list_t;

typedef boost::shared_ptr<hydla::simulator::ConsistencyChecker> consistency_checker_t;

typedef boost::shared_ptr<hydla::symbolic_expression::Ask>                ask_t;
typedef std::set<ask_t >                                           asks_t;
typedef std::set<boost::shared_ptr<hydla::symbolic_expression::Always> >  always_set_t;

typedef hydla::hierarchy::ModuleSet                              module_set_t;
typedef hydla::symbolic_expression::node_sptr                              node_sptr;

typedef boost::shared_ptr<hydla::simulator::PhaseResult>                    phase_result_sptr_t;
typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;
typedef std::list<phase_result_sptr_t >                   todo_list_t;

typedef hydla::simulator::Value                                             value_t;
typedef hydla::simulator::ValueRange                                        range_t;
typedef hydla::simulator::Variable                                          variable_t;
typedef hydla::simulator::Parameter                                         parameter_t;

typedef std::map<variable_t, range_t, hydla::simulator::VariableComparator>                    variable_map_t;

typedef std::map<parameter_t, range_t, hydla::simulator::ParameterComparator>                   parameter_map_t;


typedef std::set<std::string> change_variables_t;

typedef std::map<hydla::simulator::constraint_t, bool> constraint_diff_t;
typedef std::map<module_set_t::module_t, bool>     module_diff_t;
typedef hydla::hierarchy::ModuleSet                      module_set_t;
typedef std::set<module_set_t>                    module_set_set_t;

class Automaton
{
 public:
  std::string name;
  Automaton* parent_node;
  automaton_node_list_t next_link;
  automaton_node_list_t trace_path;
  int write;
  std::string color;
  Automaton(std::string name);
  /* ~Automaton(); */
  virtual void add_next_link(Automaton* child);
  void set_color(std::string color);
  void set_color_to_trace_path(std::string color);
  void trace();
  void dump();
  void dump_node_and_link();
  void output_dot();
  void write_reset();
};
