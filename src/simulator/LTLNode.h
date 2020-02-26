#pragma once
#include "Automaton.h"
#include "Backend.h"
#include "ConstraintStore.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "Parameter.h"
#include "PhaseResult.h"
#include "PropertyNode.h"
#include "ValueRange.h"
#include "Variable.h"
#include <iostream>
#include <string>
#include <vector>

namespace hydla {
namespace simulator {

class LTLNode;

typedef std::vector<LTLNode *> ltl_node_list_t;
typedef std::vector<automaton_node_list_t> path_list_t;
typedef std::pair<LTLNode *, node_sptr> ltl_edge_t;
typedef std::vector<ltl_edge_t> ltl_edge_list_t;

class LTLNode : public AutomatonNode {
public:
  PropertyNode *property;
  /* path_list_t acceptance_pathes; */
  bool checked_next_link;
  /* LTLNode(hydla::simulator::phase_result_sptr_t set_phase,PropertyNode*
   * set_property); */
  LTLNode(hydla::simulator::phase_result_sptr_t set_phase,
          PropertyNode *set_property, int id);
  LTLNode(std::string name, hydla::simulator::phase_result_sptr_t set_phase,
          PropertyNode *set_property, int id);
  /* ~LTLNode(); */
  /* void set_color_to_trace_path(std::string color); */
  bool acceptanceState();
  bool acceptanceCycle();
  /* bool will_include(LTLNode* check,hydla::simulator::backend_sptr_t backend);
   */
  /* LTLNode* detectLoop(LTLNode* parent_node,hydla::simulator::backend_sptr_t
   * backend); */
  /* LTLNode* detectAcceptanceCycle(LTLNode*
   * parent_node,hydla::simulator::backend_sptr_t backend); */
  /* bool search_parameter(value_t var); */
};

} // namespace simulator
} // namespace hydla
