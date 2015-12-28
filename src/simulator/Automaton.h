#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../symbolic_expression/Node.h"
#include "PhaseResult.h"


namespace hydla{
namespace simulator{

class AutomatonNode;
typedef hydla::symbolic_expression::node_sptr          node_sptr;
typedef std::vector<AutomatonNode*>            automaton_node_list_t;
typedef std::pair<AutomatonNode*,node_sptr>         automaton_edge_t;
typedef std::vector<automaton_edge_t>      automaton_edge_list_t;

//TODO: fix memory leak (memory leak is caused by never freed AutomatonNode)
class AutomatonNode
{
public:
  int id;
  std::string name;
  std::string color;
  phase_result_sptr_t phase;
  automaton_edge_list_t edges;
  automaton_node_list_t reversed_edges;

  AutomatonNode(phase_result_sptr_t phase = phase_result_sptr_t(), std::string name = "no_name",int id = 0);

  void add_edge(AutomatonNode* child,
                node_sptr guard = node_sptr(new hydla::symbolic_expression::True()));
  void remove();
  void set_id(int id);
  void set_name(std::string name);
  void set_color(std::string color);
  void dump(std::ostream &ost = std::cout);
};


class Automaton
{
public:
  Automaton clone();
  AutomatonNode* initial_node = nullptr;
  void dump(std::ostream &ost = std::cout);
  std::list<AutomatonNode *> get_all_nodes();
  
/*
  void trace();
  void dump_node_and_edge();
  void output_dot();*/
private:
  std::set<AutomatonNode *> visited_nodes;
  void clone_node(AutomatonNode *node, std::map<int, AutomatonNode*> &cloned_nodes);
  void dump_node(AutomatonNode * node, std::ostream &ost);
  void get_nodes(AutomatonNode *node, std::list<AutomatonNode *> &result_list);
};

}
}
