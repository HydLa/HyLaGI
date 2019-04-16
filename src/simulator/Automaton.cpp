#include "Automaton.h"
#include "PropertyNode.h"
#include "LTLNode.h"
#include "../symbolic_expression/Node.h"
#include <iostream>
#include <vector>
#include <string>
#include "HydLaError.h"
#include "Logger.h"

using namespace std;
using namespace hydla;
using namespace simulator;
using namespace symbolic_expression;

AutomatonNode::AutomatonNode(phase_result_sptr_t phase, std::string name,int id){
  this->id = id;
  this->name = name;
  this->phase = phase;
  this->color =  "#000000";
  this->peripheries = 0;
	this->node_time_write = true;
	this->node_vm_write = true;
	this->node_pm_write = true;
  this->edge_guard_write = false;
}

void AutomatonNode::add_edge(AutomatonNode* child,node_sptr guard){
  this->edges.push_back(std::pair<AutomatonNode*,node_sptr>(child,guard));
  child->reversed_edges.push_back(this);
}

void AutomatonNode::set_id(int id){
  this->id = id;
}

void AutomatonNode::set_name(std::string name){
  this->name = name;
}

void AutomatonNode::set_color(std::string color){
  this->color = color;
}

void AutomatonNode::set_peripheries(int num){
  this->peripheries = num;
}

void AutomatonNode::dump(ostream& ost)
{
	string name = this->name;
	if(this->node_time_write) name += "\n" + (*this->phase).get_time_string();
	if(this->node_vm_write) name += "\n" + (*this->phase).get_vm_string();
	if(this->node_pm_write) name += "\nprev map\n" + (*this->phase).get_pm_string();

  if(this->peripheries > 0){
    ost << "\"" << name << "\"" << " " << "[color=\"" << this->color << "\",peripheries=" << this->peripheries << "];" << endl;
  }else{
    ost << "\"" << name << "\"" << " " << "[color=\"" << this->color << "\"];" << endl;
  }
  if(this->edge_guard_write){
    for(auto it = edges.begin();it != edges.end();it++){
			string nextname = (*it).first->name;
			if((*it).first->node_time_write) nextname += "\n" + (*(*it).first->phase).get_time_string();
			if((*it).first->node_vm_write) nextname += "\n" + (*(*it).first->phase).get_vm_string();
			if((*it).first->node_pm_write) nextname += "\nprev map\n" + (*(*it).first->phase).get_pm_string();

      if(this->color == (*it).first->color){
        ost << "\"" << name << "\"" << " -> " << "\"" << nextname << "\"" << "[color=\"" << this->color << "\",label=\"" << get_infix_string((*it).second) << "\"];" << endl;
      }else{
        ost << "\"" << name << "\"" << " -> " << "\"" << nextname << "\"" << "[label=\"" << get_infix_string((*it).second) << "\"];" << endl;
      }
    }
  }else{
    for(auto it = edges.begin();it != edges.end();it++){
			string nextname = (*it).first->name;
			if((*it).first->node_time_write) nextname += "\n" + (*(*it).first->phase).get_time_string();
			if((*it).first->node_vm_write) nextname += "\n" + (*(*it).first->phase).get_vm_string();
			if((*it).first->node_pm_write) nextname += "\nprev map\n" + (*(*it).first->phase).get_pm_string();

      if(this->color == (*it).first->color){
        ost << "\"" << name << "\"" << " -> " << "\"" << nextname << "\"" << "[color=\"" << this->color << "\"];" << endl;
      }else{
        ost << "\"" << name << "\"" << " -> " << "\"" << nextname << "\"" << ";" << endl;
      }
    }
  }
}

void AutomatonNode::remove()
{
  for(auto node : reversed_edges)
  {
    for(auto edge_it = node->edges.begin(); edge_it != node->edges.end(); edge_it++)
    {
      if(edge_it->first == this)
      {
        node->edges.erase(edge_it);
        break;
      }
    }
  }
}


void Automaton::dump_node(AutomatonNode *node, ostream &ost){
  node->dump(ost);
  visited_nodes.insert(node);
  for(auto edge : node->edges)
  {
    if(!visited_nodes.count(edge.first))dump_node(edge.first, ost);
  }
}


void Automaton::dump(ostream& ost){
  visited_nodes.clear();
  ost << "digraph g{" << endl;
  dump_node(initial_node, ost);
  ost << "}" << endl;
}

Automaton Automaton::clone()
{
  Automaton cloned_automaton;
  map<int, AutomatonNode *> cloned_nodes;
  cloned_automaton.clone_node(initial_node, cloned_nodes);
  return cloned_automaton;
}

void Automaton::clone_node(AutomatonNode *original_node, map<int, AutomatonNode*> &cloned_nodes)
{
  AutomatonNode *cloned_node = new AutomatonNode(*original_node);
  cloned_nodes.insert(make_pair(original_node->id, cloned_node));
  cloned_node->edges.clear();
  cloned_node->reversed_edges.clear();
  if(initial_node == nullptr)initial_node = cloned_node;

  for(auto node : original_node->reversed_edges)
  {
    if(cloned_nodes.count(node->id) == 0)
    {
      clone_node(node, cloned_nodes);
      HYDLA_ASSERT(cloned_nodes.count(node->id) > 0);
    }
    cloned_node->reversed_edges.push_back(cloned_nodes[node->id]);
  }
  for(auto edge : original_node->edges)
  {
    AutomatonNode* node = edge.first;
    if(cloned_nodes.count(node->id) == 0)
    {
      clone_node(node, cloned_nodes);
      HYDLA_ASSERT(cloned_nodes.count(node->id) > 0);
    }
    cloned_node->edges.push_back(make_pair(cloned_nodes[node->id], edge.second));
  }
}

list<AutomatonNode *> Automaton::get_all_nodes()
{
  visited_nodes.clear();
  list<AutomatonNode *> all_nodes;
  get_nodes(initial_node, all_nodes);
  all_nodes.push_back(initial_node);
  return all_nodes;
}

void Automaton::get_nodes(AutomatonNode *node, list<AutomatonNode *> &result_list)
{
  visited_nodes.insert(node);
  result_list.push_back(node);
  for(auto edge: node->edges)
  {
    if(!visited_nodes.count(edge.first))get_nodes(edge.first, result_list);
  }
}

AutomatonNode* Automaton::exist_node(std::string name){
  std::list<AutomatonNode *> exist_nodes = this->get_all_nodes();
  for(auto exist_node : exist_nodes){
    if(exist_node->name == name) return exist_node;
  }
  return nullptr;
}

bool Automaton::exist_edge(AutomatonNode *base, AutomatonNode *end){
  for(auto exist_edge : base->edges){
    if(exist_edge.first == end) return true;
  }
  return false;
}

void Automaton::exec(const Opts& opts, ostream& ost){
	AutomatonNode* current_node = initial_node;
	//current_node->dump(ost);
		value_t current_time, end_time;
		current_time = end_time = current_node->phase->current_time;
		variable_map_t variable_map = current_node->phase->variable_map;
	
	for(int i = 1; i <= opts.max_phase; i++){
		current_node = current_node->edges[0].first;

		if(i&1){
			ost << "---------" << (i+1)/2 << "---------" << endl;
			ost << "---------PP " << i << "---------" << endl;
		}else{
			ost << "---------IP " << i << "---------" << endl;
		}

		ost << current_node->phase->get_mod_string() << endl;
	}
}
/*
void AutomatonNode::trace(){
  std::cout << "// this is trace of " << this->name << "." << std::endl;
  for(typename std::vector<AutomatonNode *>::iterator it = this->trace_path.begin();it + 1 != this->trace_path.end();it++){
    if(it == this->trace_path.begin()){
      std::cout << "\"init\"[shape=\"point\"];" << std::endl;
      std::cout << "\"init\"" << " -> " << "\"" << (*it)->name << "\"" << "[color=\"" << (*it)->color << "\"];" << std::endl;
    }
    if((it + 1) != trace_path.end()){
      std::cout << "\"" << (*it)->name << "\"" << " " << "[color=\"" << (*it)->color << "\"];" << std::endl;
      if((*it)->color == (*(it + 1))->color){
        std::cout << "\"" << (*it)->name << "\"" << " -> " << "\"" << (*(it + 1))->name << "\"" << "[color=\"" << (*it)->color << "\"];" << std::endl;
      }else{
        std::cout << "\"" << (*it)->name << "\"" << " -> " << "\"" << (*(it + 1))->name << "\"" << ";" << std::endl;
      }
    }
  }
  std::cout << "// " << this->name << " is traced." << std::endl;
}

void Automaton::dump(){
  std::cout << "~~~~~~~~ dump : " << this->name << " start. ~~~~~~~~" << std::endl;
  std::cout << "digraph g{" << std::endl;
  this->trace();
  this->write_reset();
  std::cout << "// nodes and links after " << this->name << "." << std::endl;
  this->dump_node_and_edge();
  this->write_reset();
  std::cout << "}" << std::endl;
  std::cout << "~~~~~~~~ dump : " << this->name << " end.  ~~~~~~~~" << std::endl;
}

void Automaton::output_dot(){
  std::cout << "======== Out put dot file ========" << std::endl;
  this->write_reset();
  std::cout << "digraph g{" << std::endl;
  std::cout << "\"init\"[shape=\"point\"];" << std::endl;
  std::cout << "\"init\"" << " -> " << "\"" << this->name << "\"" << "[color=\"" << this->color << "\"];" << std::endl;
  this->dump_node_and_edge();
  std::cout << "}" << std::endl;
  this->write_reset();
  std::cout << "==================================" << std::endl;
}

void AutomatonNode::dump_node_and_edge(){
  if(write == 0){
    write++;
    std::cout << "\"" << this->name << "\"" << " " << "[color=\"" << this->color << "\"];" << std::endl;
    for(typename std::vector<std::pair<AutomatonNode *,node_sptr>>::iterator it = next_edge.begin();it != next_edge.end();it++){
      if(this->name == (*it).first->name){
        std::cout << "\"" << this->name << "\"" << " -> " << "\"" << this->name << "\"" << " " <<  "[color=\"" << this->color << "\"];" << std::endl;
      }else{
        if(this->color == (*it).first->color){
          std::cout << "\"" << this->name << "\"" << " -> " << "\"" << (*it).first->name << "\"" << "[color=\"" << this->color << "\"];" << std::endl;
        }else{
          std::cout << "\"" << this->name << "\"" << " -> " << "\"" << (*it).first->name << "\"" << ";" << std::endl;
        }
        (*it).first->dump_node_and_edge();
      }
    }
  }
}
*/
