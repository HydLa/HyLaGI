#include "AskRelationGraph.h"
#include <iostream>
#include "VariableFinder.h"
#include "Logger.h"
#include "SimulateError.h"

using namespace std;

namespace hydla {
namespace simulator {


AskRelationGraph::AskRelationGraph(const module_set_t &ms)
{
  for(auto module : ms)
  {
    add(module);
  }
}


AskRelationGraph::~AskRelationGraph()
{
  for(auto var : variable_nodes){
    delete var;
  }

  for(auto ask : ask_nodes){
    delete ask;
  }
}

void AskRelationGraph::add(module_t &mod)
{
  current_module = mod;
  parent_ask = nullptr;
  accept(mod.second);
}

void AskRelationGraph::dump_graph(ostream & os) const
{
  os << "graph g {\n";
  os << "graph [ranksep = 2.0 ,rankdir = LR];\n";
  for(auto ask_node : ask_nodes) {
    string ask_name = ask_node->get_name();
    os << "  \"" << ask_name << "\" [shape = box]\n";
    for(auto edge : ask_node->edges){
      string variable_name = edge->get_name();
      os << "  \"" 
        << ask_name 
        << "\" -- \"" 
        << variable_name 
        << "\";\n";
    }
  }
  os << "}" << endl;
}


string AskRelationGraph::VariableNode::get_name() const
{
  return variable;
}

string AskRelationGraph::AskNode::get_name() const
{
  string ret = symbolic_expression::get_infix_string(ask);
  // if too long, cut latter part
  const string::size_type max_length = 10;
  if(ret.length() > max_length)
  {
    ret = ret.substr(0, max_length) + ("...");
  }
  return ret + " (" + module.first + ")";
}


void AskRelationGraph::set_adopted(const module_t &mod, bool adopted)
{
  if(!module_ask_nodes_map.count(mod))return;
  for(auto ask_node : module_ask_nodes_map[mod])
  {
    ask_node->module_adopted = adopted;
  }
}

void AskRelationGraph::set_adopted(const module_set_t &ms, bool adopted)
{
  for(auto module : ms)
  {
    set_adopted(module, adopted);
  }
}


void AskRelationGraph::set_entailed(const ask_t &ask, bool entailed)
{
  auto node_it = ask_node_map.find(ask);
  if(node_it == ask_node_map.end())return;
  node_it->second->entailed = entailed;
}


bool AskRelationGraph::set_entailed_if_prev(const ask_t &ask, bool entailed)
{
  auto node_it = ask_node_map.find(ask);
  if(node_it == ask_node_map.end())return false;
  if(node_it->second->prev)
  {
    node_it->second->entailed = entailed;
    return true;
  }
  return false;
}

AskRelationGraph::asks_t AskRelationGraph::get_adjacent_asks(const string &var, bool ignore_prev_asks){
  asks_t asks;
  if(!variable_node_map.count(var))return asks;
  VariableNode *var_node = variable_node_map[var];
  if(var_node == nullptr)throw HYDLA_SIMULATE_ERROR("VariableNode is not found");
  for(auto ask_node : var_node->edges)
  {
    if(active(ask_node))
    {
      asks.push_back(ask_node->ask);
    }
  }
  return asks;
}


set<string> AskRelationGraph::get_adjacent_variables(const ask_t &ask){
  set<string> vars;
  if(!ask_node_map.count(ask))return vars;
  AskNode *ask_node = ask_node_map[ask];
  for(auto var_node: ask_node->edges)
  {
    vars.insert(var_node->variable);
  }
  return vars;
}

AskRelationGraph::asks_t AskRelationGraph::get_active_asks(bool ignore_prev_asks)
{
  asks_t asks;
  for(auto ask_node : ask_nodes)
  {
    if(active(ask_node))asks.push_back(ask_node->ask);
  }
  return asks;
}


AskRelationGraph::VariableNode* AskRelationGraph::add_variable_node(const string &var)
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

void AskRelationGraph::visit(boost::shared_ptr<symbolic_expression::Ask> ask)
{
  VariableFinder finder;
  finder.visit_node(ask->get_guard());
  variable_set_t variables;
    
  AskNode* ask_node;
  assert(!ask_node_map.count(ask)); /// assume that same ask node doesn't exist

  ask_node = new AskNode(ask, current_module);
  ask_nodes.push_back(ask_node);
  ask_node_map[ask] = ask_node;
  ask_node->parent_ask = parent_ask;

  variables = finder.get_all_variable_set();
  for(auto variable : variables)
  {
    VariableNode* var_node = add_variable_node(variable.get_name());
    ask_node->edges.push_back(var_node);
    var_node->edges.push_back(ask_node);
  }

  parent_ask = ask_node;
  accept(ask->get_rhs());
  parent_ask = ask_node->parent_ask;
}

AskRelationGraph::AskNode::AskNode(const ask_t &a, const module_t &mod):ask(a), module(mod), module_adopted(true), entailed(false), parent_ask(nullptr)
{
  VariableFinder finder;
  finder.visit_node(ask->get_guard());
  prev = finder.get_variable_set().empty();
}

bool AskRelationGraph::active(const AskNode *ask) const
{
  return ask->module_adopted && ask->prev && (ask->parent_ask == nullptr || ask->parent_ask->entailed);
}

} //namespace simulator
} //namespace hydla 
