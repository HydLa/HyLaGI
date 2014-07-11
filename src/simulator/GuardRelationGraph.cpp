#include "GuardRelationGraph.h"
#include <iostream>
#include "VariableFinder.h"
#include "Logger.h"
#include "SimulateError.h"

using namespace std;

namespace hydla {
namespace simulator {


GuardRelationGraph::~GuardRelationGraph()
{
  for(auto var : variable_nodes){
    delete var;
  }

  for(auto guard : guard_nodes){
    delete guard;
  }
}

void GuardRelationGraph::add(module_t &mod)
{
  current_module = mod;
  accept(mod.second);
}

void GuardRelationGraph::dump_graph(ostream & os) const
{
  os << "graph g {\n";
  os << "graph [ranksep = 2.0 ,rankdir = LR];\n";
  for(auto guard_node : guard_nodes) {
    string guard_name = guard_node->get_name();
    os << "  \"" << guard_name << "\" [shape = box]\n";
    for(auto edge : guard_node->edges){
      string variable_name = edge->get_name();
      os << "  \"" 
        << guard_name 
        << "\" -- \"" 
        << variable_name 
        << "\";\n";
    }
  }
  os << "}" << endl;
}


string GuardRelationGraph::VariableNode::get_name() const
{
  return variable.get_string();
}

string GuardRelationGraph::GuardNode::get_name() const
{
  string ret = symbolic_expression::get_infix_string(guard);
  // if too long, cut latter part
  const string::size_type max_length = 10;
  if(ret.length() > max_length)
  {
    ret = ret.substr(0, max_length) + ("...");
  }
  return ret + " (" + module.first + ")";
}

void GuardRelationGraph::initialize_node_visited()
{
  for(auto guard_node : guard_nodes){
//    guard_node->visited = false;
  }
}


void GuardRelationGraph::get_related_guards(constraint_t guard, ConstraintStore &guards, module_set_t &module_set){
  initialize_node_visited();
  guards.clear();
  module_set.clear();
  auto guard_it = guard_node_map.find(guard);
  if(guard_it == guard_node_map.end())
  {
    VariableFinder finder;
    finder.visit_node(guard);
    variable_set_t variables;
    variables = finder.get_all_variable_set();
    for(auto variable : variables)
    {
      if(!variable_node_map.count(variable))continue;
      VariableNode *var_node = variable_node_map[variable];
      variable_set_t vars;
      visit_node(var_node, guards, module_set, vars);
    }
  }
  else
  {
    variable_set_t vars;
    GuardNode *guard_node = guard_it->second;
    visit_node(guard_node, guards, module_set, vars);
  }
}


void GuardRelationGraph::get_related_guards(const Variable &var, ConstraintStore &guards, module_set_t &module_set){
  initialize_node_visited();
  guards.clear();
  module_set.clear();
  if(!variable_node_map.count(var))return;
  VariableNode *var_node = variable_node_map[var];
  if(var_node == nullptr)throw HYDLA_SIMULATE_ERROR("VariableNode is not found");
  variable_set_t vars;
  visit_node(var_node, guards, module_set, vars);
}

void GuardRelationGraph::visit_node(GuardNode* node, ConstraintStore &guards, module_set_t &ms, variable_set_t &vars){
  ms.add_module(node->module);
  guards.add_constraint(node->guard);
  for(auto var_node : node->edges)
  {
    visit_node(var_node, guards, ms, vars);
  }
}

void GuardRelationGraph::visit_node(VariableNode* node, ConstraintStore &guards, module_set_t &ms, variable_set_t &vars){
  vars.insert(node->variable);
  for(auto guard_node : node->edges)
  {
    visit_node(guard_node, guards, ms, vars);
  }
}

GuardRelationGraph::GuardRelationGraph(const module_set_t &ms)
{
  for(auto module : ms)
  {
    add(module);
  }
}

ConstraintStore GuardRelationGraph::get_guards()
{
  ConstraintStore guards;
  for(auto guard_node : guard_nodes)
  {
    guards.add_constraint(guard_node->guard);
  }
  return guards;
}


GuardRelationGraph::VariableNode* GuardRelationGraph::add_variable_node(Variable &var)
{
  if(variable_node_map.count(var))
  {
    return variable_node_map[var];
  }
  else
  {
    VariableNode* ret = new VariableNode(var);
    variable_nodes.push_back(ret);
    variable_node_map[var] = ret;
    return ret;
  }
}

void GuardRelationGraph::visit(boost::shared_ptr<symbolic_expression::Ask> ask)
{
  constraint_t guard = ask->get_lhs();
  VariableFinder finder;
  finder.visit_node(guard);
  variable_set_t variables;
    
  GuardNode* guard_node;
  if(guard_node_map.count(guard))
  {
    guard_node = guard_node_map[guard];
  }
  else
  {
    guard_node = new GuardNode(guard, current_module);
    guard_nodes.push_back(guard_node);
    guard_node_map[guard] = guard_node; 
  }

  variables = finder.get_all_variable_set();
  for(auto variable : variables)
  {
    VariableNode* var_node = add_variable_node(variable);
    guard_node->edges.push_back(var_node);
    var_node->edges.push_back(guard_node);
  }

  accept(ask->get_rhs());
}


} //namespace simulator
} //namespace hydla 
