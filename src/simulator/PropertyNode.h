#pragma once
#include "../symbolic_expression/Node.h"
#include "Automaton.h"
#include <iostream>
#include <vector>
#include <string>

class PropertyNode;

typedef enum {
  ZERO,
  NOMAL,
  ACCEPTANCE_STATE,
  ACCEPTANCE_CYCLE
} PropertyNodeType;
typedef std::pair<hydla::symbolic_expression::node_sptr,PropertyNode*> property_link_t;
typedef std::vector<property_link_t> property_link_list_t;
typedef hydla::symbolic_expression::node_sptr                     node_sptr;

class PropertyNode : public Automaton
{
 public:
  PropertyNodeType type;
  property_link_list_t link;
  PropertyNode(int set_id, PropertyNodeType set_type);
  ~PropertyNode();
  void add_next_link(node_sptr guard,PropertyNode* child);
};
