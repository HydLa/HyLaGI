#include "AskRelationGraph.h"
#include <iostream>
#include "VariableFinder.h"
#include "Logger.h"
#include "SimulateError.h"

using namespace std;

namespace hydla {
namespace simulator {


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


void AskRelationGraph::get_adjacent_asks(const string &var, asks_t &asks){
  asks.clear();
  if(!variable_node_map.count(var))return;
  VariableNode *var_node = variable_node_map[var];
  if(var_node == nullptr)throw HYDLA_SIMULATE_ERROR("VariableNode is not found");
  for(auto ask_node : var_node->edges)
  {
    if(ask_node->module_adopted)
    {
      asks.push_back(ask_node->ask);
    }
  }
}


void AskRelationGraph::get_adjacent_variables(const ask_t &ask, set<string> &vars){
  vars.clear();
  if(!ask_node_map.count(ask))return;
  AskNode *ask_node = ask_node_map[ask];
  if(ask_node == nullptr)throw HYDLA_SIMULATE_ERROR("AskNode is not found");
  if(!ask_node->module_adopted)return;
  for(auto var_node: ask_node->edges)
  {
    vars.insert(var_node->variable);
  }
}

AskRelationGraph::AskRelationGraph(const module_set_t &ms)
{
  for(auto module : ms)
  {
    add(module);
  }
}

AskRelationGraph::asks_t AskRelationGraph::get_asks()
{
  asks_t asks;
  for(auto ask_node : ask_nodes)
  {
    if(ask_node->module_adopted)asks.push_back(ask_node->ask);
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
  if(ask_node_map.count(ask))
  {
    ask_node = ask_node_map[ask];
  }
  else
  {
    ask_node = new AskNode(ask, current_module);
    ask_nodes.push_back(ask_node);
    ask_node_map[ask] = ask_node; 
  }

  variables = finder.get_all_variable_set();
  for(auto variable : variables)
  {
    VariableNode* var_node = add_variable_node(variable.get_name());
    ask_node->edges.push_back(var_node);
    var_node->edges.push_back(ask_node);
  }

  accept(ask->get_rhs());
}


} //namespace simulator
} //namespace hydla 
