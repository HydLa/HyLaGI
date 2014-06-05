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


void RelationGraph::check_connected_components(){
  connected_constraints_vector.clear();
  connected_modules_vector.clear();
  for(auto constraint_node : constraint_nodes){
    constraint_node->visited = !constraint_node->valid;
  }

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
    constraints.push_back(node->constraint);
    visit_edges(node, constraints, ms);
  }
}

void RelationGraph::visit_edges(ConstraintNode* node, constraints_t &constraints, module_set_t &ms){
  for(auto var_node : node->edges)
  {
    for(auto constraint_node : var_node->edges)
    {
      visit_node(constraint_node, constraints, ms);
    }
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

void RelationGraph::set_valid(const module_t &mod, bool valid)
{
  for(auto constraint_node : module_constraint_nodes_map[mod])
  {
    constraint_node->valid = valid;
  }
  up_to_date = false;
}

void RelationGraph::set_valid(module_set_t *ms)
{
  for(auto entry : module_constraint_nodes_map)
  {
    bool valid = !(ms->find(entry.first) == ms->end());
    for(auto constraint_node : entry.second)
    {
      constraint_node->valid = valid;
    }
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

RelationGraph::module_set_sptr RelationGraph::get_modules(unsigned int index)
{
  if(!up_to_date){
    check_connected_components();
  }
  assert(index < connected_modules_vector.size());
  return boost::shared_ptr<module_set_t>(new module_set_t(connected_modules_vector[index]));
}


void RelationGraph::add_node(boost::shared_ptr<symbolic_expression::BinaryNode> node)
{
  VariableFinder finder;
  finder.visit_node(node);
  VariableFinder::variable_set_t variables;
  variables = finder.get_all_variable_set();

  ConstraintNode*& cons = constraint_map[node];
  if(cons == NULL){
    cons = new ConstraintNode(node, current_module);
    constraint_nodes.push_back(cons);
    module_constraint_nodes_map[current_module].push_back(cons);
  }
  for(auto variable : variables)
  {
    VariableNode*& var_node = variable_map[variable];
    if(var_node == NULL){
      var_node = new VariableNode(variable);
      variable_nodes.push_back(var_node);
    }
    cons->edges.push_back(var_node);
    var_node->edges.push_back(cons);
  }
}

void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Equal> node)
{
  add_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::UnEqual> node)
{
  add_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Less> node)
{
  add_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::LessEqual> node)
{
  add_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Greater> node)
{
  add_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::GreaterEqual> node)
{
  add_node(node);
}
void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Ask> node)
{
  accept(node->get_rhs());
}


} //namespace simulator
} //namespace hydla 
