#pragma once
#include "../symbolic_expression/Node.h"
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

typedef std::vector< std::pair<hydla::symbolic_expression::node_sptr,PropertyNode*> > Property_link_t;

class PropertyNode
{
 public:
  int id;
  PropertyNodeType type;
  int write;
  Property_link_t link;
  PropertyNode(int set_id, PropertyNodeType set_type);
  ~PropertyNode();
  void addLink(hydla::symbolic_expression::node_sptr guard,PropertyNode* child);
  void dump();
  void dot();
  void write_reset();
};
