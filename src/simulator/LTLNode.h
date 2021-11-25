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
  LTLNode(hydla::simulator::phase_result_sptr_t set_phase,
          PropertyNode *set_property, int id);
  LTLNode(std::string name, hydla::simulator::phase_result_sptr_t set_phase,
          PropertyNode *set_property, int id);
  /* ~LTLNode(); */
  bool acceptanceState();
  bool acceptanceCycle();
};

} // namespace simulator
} // namespace hydla
