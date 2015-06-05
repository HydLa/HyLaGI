#pragma once
#include "PropertyNode.h"
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

class LTLNode;

typedef std::vector<LTLNode*> ltl_node_list_t;
typedef std::vector<ltl_node_list_t> pass_list_t;
typedef boost::shared_ptr<hydla::simulator::ConsistencyChecker> consistency_checker_t;


typedef boost::shared_ptr<hydla::symbolic_expression::Ask>                ask_t;
typedef std::set<ask_t >                                           asks_t;
typedef std::set<boost::shared_ptr<hydla::symbolic_expression::Always> >  always_set_t;

typedef hydla::hierarchy::ModuleSet                              module_set_t;

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

class LTLNode
{
 public:
  std::string id;
  hydla::simulator::phase_result_sptr_t phase;
  PropertyNode* property;
  LTLNode* parent;
  ltl_node_list_t link;
  ltl_node_list_t pass;
  pass_list_t acceptance_passes;
  int write;
  int red;
  bool checked_next_link;
  LTLNode(hydla::simulator::phase_result_sptr_t set_phase,PropertyNode* set_property);
  /* ~LTLNode(); */
  bool acceptanceState();
  bool acceptanceCycle();
  /* bool will_include(LTLNode* check,hydla::simulator::backend_sptr_t backend); */
  /* LTLNode* detectLoop(LTLNode* parent_node,hydla::simulator::backend_sptr_t backend); */
  /* LTLNode* detectAcceptanceCycle(LTLNode* parent_node,hydla::simulator::backend_sptr_t backend); */
  void addLink(LTLNode* child);
  void setRed();
  void trace();
  void dump();
  void dot();
  void write_reset();
  bool search_parameter(value_t var);
};
/* bool check_edge_guard(hydla::simulator::phase_result_sptr_t phase,hydla::symbolic_expression::node_sptr guard,hydla::simulator::backend_sptr_t backend,consistency_checker_t consistency_checker); */
/* ltl_node_list_t transition(ltl_node_list_t current,hydla::simulator::phase_result_sptr_t phase,hydla::simulator::backend_sptr_t backend,consistency_checker_t consistency_checker); */
/* Phase* simulate(Phase* phase); */
