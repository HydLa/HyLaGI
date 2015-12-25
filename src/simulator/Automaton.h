#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "../symbolic_expression/Node.h"

class Automaton;
typedef hydla::symbolic_expression::node_sptr          node_sptr;
typedef std::vector<Automaton*>            automaton_node_list_t;
typedef std::pair<Automaton*,node_sptr>         automaton_edge_t;
typedef std::vector<automaton_edge_t>      automaton_edge_list_t;
typedef std::vector<automaton_node_list_t> automaton_path_list_t;

class Automaton
{
 public:
  int id;
  std::string name;
  std::string color;
  Automaton* parent_node;
  automaton_edge_list_t next_edge;
  automaton_node_list_t trace_path;
  int write;

  Automaton();
  Automaton(std::string name);
  Automaton(std::string name,int id);
  /* ~Automaton(); */

  void add_next_edge(Automaton* child);
  void add_next_edge(Automaton* child,node_sptr guard);
  void set_id(int id);
  void set_name(std::string name);
  void set_color(std::string color);
  void set_color_to_trace_path(std::string color);
  void trace();
  void dump();
  void dump_node_and_edge();
  void output_dot();
  void write_reset();


};
