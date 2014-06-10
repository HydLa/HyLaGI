#include "RelationGraph.h"
#include <iostream>
#include "VariableFinder.h"
#include "Logger.h"

using namespace std;

namespace hydla {
namespace simulator {


RelationGraph::~RelationGraph()
{
  for(auto var : variable_nodes){
    delete var;
  }

  for(auto constraint : constraint_nodes){
    delete constraint;
  }
}

void RelationGraph::add(module_t &mod)
{
  current_module = mod;
  visit_mode = ADDING;
  accept(mod.second);
  up_to_date = false;
}

ostream& RelationGraph::dump_graph(ostream & os) const
{
  os << "graph g {\n";
  
  for(auto constraint_node : constraint_nodes) {
    os << "  \"" << constraint_node->get_name() << "\" [shape = box]\n";
    for(auto var_node : constraint_node->edges){
      os << "  \"" 
        << constraint_node->get_name() 
        << "\" -- \"" 
        << var_node->get_name() 
        << "\";\n";
    }
  }
  os << "}" << endl;

  return os;
}

string RelationGraph::VariableNode::get_name() const
{
  return variable.get_string();
}

string RelationGraph::ConstraintNode::get_name() const
{
  string ret = symbolic_expression::get_infix_string(constraint);
  // if too long, cut latter part
  const string::size_type max_length = 10;
  if(ret.length() > max_length)
  {
    ret = ret.substr(0, max_length) + ("...");
  }
  return ret + " (" + module.first + ")";
}

void RelationGraph::initialize_node_visited()
{
  for(auto constraint_node : constraint_nodes){
    constraint_node->visited = !constraint_node->module_adopted || !constraint_node->expanded;
  }
}

void RelationGraph::get_related_constraints(constraint_t constraint, constraints_t &constraints, module_set_t &module_set){
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  auto constraint_it = constraint_node_map.find(constraint);
  if(constraint_it == constraint_node_map.end())
  {
    VariableFinder finder;
    finder.visit_node(constraint);
    VariableFinder::variable_set_t variables;
    variables = finder.get_all_variable_set();
    for(auto variable : variables)
    {
      VariableNode *var_node = variable_node_map[variable];
      visit_node(var_node, constraints, module_set);
    }
  }
  else
  {
    ConstraintNode *constraint_node = constraint_it->second;
    visit_node(constraint_node, constraints, module_set);
  }
}


void RelationGraph::get_related_constraints(const Variable &var, constraints_t &constraints, module_set_t &module_set){
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  VariableNode *var_node = variable_node_map[var];
  assert(var_node != nullptr);
  visit_node(var_node, constraints, module_set);
}


void RelationGraph::check_connected_components(){
  connected_constraints_vector.clear();
  connected_modules_vector.clear();
  initialize_node_visited();


  for(auto constraint_node : constraint_nodes){
    module_set_t ms;
    constraints_t constraints;
    if(!constraint_node->visited){
      visit_node(constraint_node, constraints, ms);
      connected_constraints_vector.push_back(constraints);
      connected_modules_vector.push_back(ms);
    }
  }
  up_to_date = true;
}

void RelationGraph::visit_node(ConstraintNode* node, constraints_t &constraints, module_set_t &ms){
  if(!node->visited){
    node->visited = true;
    ms.add_module(node->module);
    constraints.insert(node->constraint);
    for(auto var_node : node->edges)
    {
      visit_node(var_node, constraints, ms);
    }
  }
}

void RelationGraph::visit_node(VariableNode* node, constraints_t &constraints, module_set_t &ms){
  for(auto constraint_node : node->edges)
  {
    visit_node(constraint_node, constraints, ms);
  }
}

RelationGraph::RelationGraph(const module_set_t &ms)
{
  for(auto module : ms)
  {
    add(module);
  }
  up_to_date = false;
}

int RelationGraph::get_connected_count()
{
  if(!up_to_date){
    check_connected_components();
  }
  return connected_constraints_vector.size();
}

void RelationGraph::set_adopted(const module_t &mod, bool adopted)
{
  for(auto constraint_node : module_constraint_nodes_map[mod])
  {
    constraint_node->module_adopted = adopted;
  }
  up_to_date = false;
}

void RelationGraph::set_adopted(module_set_t *ms)
{
  for(auto entry : module_constraint_nodes_map)
  {
    bool adopted = !(ms->find(entry.first) == ms->end());
    set_adopted(entry.first, adopted);
  }
  up_to_date = false;
}


void RelationGraph::set_expanded(constraint_t cons, bool expanded)
{
  visit_mode = expanded?EXPANDING:UNEXPANDING;
  accept(cons);
  up_to_date = false;
}


void RelationGraph::set_expanded_all(bool expanded)
{
  for(auto node : constraint_nodes)
  {
    node->expanded = expanded;
  }
  up_to_date = false;
}


RelationGraph::constraints_t RelationGraph::get_constraints(unsigned int index)
{
  if(!up_to_date){
    check_connected_components();
  }
  assert(index < connected_constraints_vector.size());
  return connected_constraints_vector[index];
}

RelationGraph::constraints_t RelationGraph::get_constraints()
{
  constraints_t constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->expanded && constraint_node->module_adopted)
    {
      constraints.insert(constraint_node->constraint);
    }
  }
  return constraints;
}

RelationGraph::constraints_t RelationGraph::get_expanded_constraints()
{
  constraints_t constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->expanded)
    {
      constraints.insert(constraint_node->constraint);
    }
  }
  return constraints;
}


RelationGraph::module_set_t RelationGraph::get_modules(unsigned int index)
{
  if(!up_to_date){
    check_connected_components();
  }
  assert(index < connected_modules_vector.size());
  return connected_modules_vector[index];
}


void RelationGraph::visit_binary_node(boost::shared_ptr<symbolic_expression::BinaryNode> node)
{
  if(visit_mode == ADDING)
  {
    VariableFinder finder;
    finder.visit_node(node);
    VariableFinder::variable_set_t variables;
    variables = finder.get_all_variable_set();

    ConstraintNode*& cons = constraint_node_map[node];
    if(cons == NULL){
      cons = new ConstraintNode(node, current_module);
      constraint_nodes.push_back(cons);
      module_constraint_nodes_map[current_module].push_back(cons);
    }
    for(auto variable : variables)
    {
      VariableNode*& var_node = variable_node_map[variable];
      if(var_node == NULL){
        var_node = new VariableNode(variable);
        variable_nodes.push_back(var_node);
      }
      cons->edges.push_back(var_node);
      var_node->edges.push_back(cons);
    }
  }
  else if(visit_mode == EXPANDING)
  {
    assert(constraint_node_map[node] != nullptr);
    constraint_node_map[node]->expanded = true;
  }
  else if(visit_mode == UNEXPANDING)
  {
    assert(constraint_node_map[node] != nullptr);
    constraint_node_map[node]->expanded = false;
  }
}

void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Equal> node)
{
  visit_binary_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::UnEqual> node)
{
  visit_binary_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Less> node)
{
  visit_binary_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::LessEqual> node)
{
  visit_binary_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Greater> node)
{
  visit_binary_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::GreaterEqual> node)
{
  visit_binary_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Ask> node)
{
  if(visit_mode == ADDING)
  {
    accept(node->get_rhs());
  }
}


} //namespace simulator
} //namespace hydla 
