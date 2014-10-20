#include "RelationGraph.h"
#include <iostream>
#include "VariableFinder.h"
#include "Logger.h"
#include "SimulateError.h"

using namespace std;

namespace hydla {
namespace simulator {


RelationGraph::RelationGraph(const module_set_t &ms)
{
  for(auto module : ms)
  {
    add(module);
  }
  ignore_prev = true;
  up_to_date = false;
}


RelationGraph::~RelationGraph()
{
  for(auto var : variable_nodes){
    delete var;
  }

  for(auto constraint : tell_nodes){
    delete constraint;
  }
  for(auto ask : ask_nodes)
  {
    delete ask;
  }
}

void RelationGraph::add(module_t &mod)
{
  current_module = mod;
  visit_mode = ADDING;
  parent_ask = nullptr;
  accept(mod.second);
  up_to_date = false;
}

void dump_tell_node(RelationGraph::TellNode *node, ostream &os)
{
  string constraint_name = node->get_name();
  os << " \"";
  if(node->always)os << "[]";
  os << constraint_name << "\" [shape = box]\n";
  for(auto edge : node->edges){
    string variable_name = edge.variable_node->get_name();
    os << "  \"" 
       << constraint_name 
       << "\" -- \"" 
       << variable_name 
       << "\"";
    if(edge.ref_prev)
    {
      os << " [style = dotted]";
    }
    os <<  ";\n";
  }
}


void dump_ask_node(RelationGraph::AskNode *node, ostream &os)
{
  string constraint_name = node->get_name();
  os << " \"";
  if(node->always)os << "[]";
  os << constraint_name << "\" [shape = hexagon]\n";
  for(auto edge : node->edges){
    string variable_name = edge.variable_node->get_name();
    os << "  \"" 
       << constraint_name 
       << "\" -- \"" 
       << variable_name 
       << "\"";
    if(edge.ref_prev)
    {
      os << " [style = dotted]";
    }
    os <<  ";\n";
  }
}


void RelationGraph::dump_graph(ostream & os) const
{
  os << "graph g {\n";
  os << "graph [ranksep = 1.0 ,rankdir = LR];\n";
  for(auto tell_node : tell_nodes) {
    dump_tell_node(tell_node, os);
  }

  for(auto ask_node : ask_nodes) {
    dump_ask_node(ask_node, os);
  }
  os << "}" << endl;
}


void RelationGraph::dump_active_graph(ostream & os) const
{
  os << "graph g {" << endl;
  os << "graph [ranksep = 2.0 ,rankdir = LR];" << endl;
  for(auto tell_node : tell_nodes) {
    if(tell_node->is_active()) dump_tell_node(tell_node, os);
  }

  for(auto ask_node : ask_nodes) {
    if(ask_node->is_active()) dump_ask_node(ask_node, os);
  }
  os << "}" << endl;
}


RelationGraph::EdgeToConstraint::EdgeToConstraint(TellNode *cons, bool prev)
  : tell_node(cons), ref_prev(prev){}

RelationGraph::EdgeToVariable::EdgeToVariable(VariableNode *var, bool prev)
  : variable_node(var), ref_prev(prev){}

string RelationGraph::VariableNode::get_name() const
{
  return variable.get_string();
}


string get_constraint_name(const constraint_t &constraint, const RelationGraph::module_t &module)
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

string RelationGraph::TellNode::get_name() const
{
  return get_constraint_name(constraint, module);
}

string RelationGraph::AskNode::get_name() const
{
  return get_constraint_name(ask, module);
}


bool RelationGraph::ConstraintNode::is_active() const
{
  return expanded && module_adopted;
}

bool RelationGraph::referring(const Variable& var)
{
  if(!up_to_date) check_connected_components();
  return referred_variables.count(var) > 0;
}


void RelationGraph::initialize_node_visited()
{
  for(auto tell_node : tell_nodes){
    tell_node->visited = false;
  }
}



void RelationGraph::get_related_constraints_vector(const ConstraintStore &constraint_store, vector<ConstraintStore> &constraints_vector, vector<module_set_t> &module_set_vector){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints_vector.clear();
  module_set_vector.clear();
  for(auto constraint : constraint_store)
  {
    auto constraint_it = tell_node_map.find(constraint);
    if(constraint_it == tell_node_map.end())
    {
      VariableFinder finder;
      finder.visit_node(constraint);
      variable_set_t variables;
      variables = finder.get_all_variable_set();
      for(auto variable : variables)
      {
        if(!variable_node_map.count(variable))continue;

        ConstraintStore connected_constraints;
        module_set_t connected_ms;
        VariableNode *var_node = variable_node_map[variable];
        variable_set_t vars;
        visit_node(var_node, connected_constraints, connected_ms, vars);
        if(connected_constraints.size() > 0)
        {
          constraints_vector.push_back(connected_constraints);
          module_set_vector.push_back(connected_ms);
        }
      }
    }
    else
    {
      TellNode *tell_node = constraint_it->second;
      if(!tell_node->visited)
      {
        if(tell_node->is_active())
        {
          ConstraintStore connected_constraints;
          module_set_t connected_ms;
          variable_set_t vars;
          visit_node(tell_node, connected_constraints, connected_ms, vars);
          constraints_vector.push_back(connected_constraints);
          module_set_vector.push_back(connected_ms);
        }
        else
        {
          // adjacent node may be active (this case is mainly caused by negative asks)
          ConstraintStore connected_constraints;
          module_set_t connected_ms;
          variable_set_t vars;
          for(auto edge : tell_node->edges)
          {
            if(!ignore_prev || !edge.ref_prev) visit_node(edge.variable_node, connected_constraints, connected_ms, vars);
          }
          if(connected_constraints.size() > 0)
          {
            constraints_vector.push_back(connected_constraints);
            module_set_vector.push_back(connected_ms);
          }
        }
      }
    }
  }
}



void RelationGraph::get_related_constraints_vector(const ConstraintStore &constraint_store, const variable_set_t &variables, vector<ConstraintStore> &constraints_vector, vector<module_set_t> &module_set_vector){
  get_related_constraints_vector(constraint_store, constraints_vector, module_set_vector);
  for(auto var : variables)
  {
    ConstraintStore constraints;
    module_set_t modules;
    get_related_constraints_core(var, constraints, modules);
    if(!constraints.empty())
    {
      constraints_vector.push_back(constraints);
      module_set_vector.push_back(modules);
    }
  }
}



void RelationGraph::get_related_constraints(constraint_t constraint, ConstraintStore &constraints, module_set_t &module_set){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  auto constraint_it = tell_node_map.find(constraint);
  if(constraint_it == tell_node_map.end())
  {
    VariableFinder finder;
    finder.visit_node(constraint);
    variable_set_t variables;
    variables = finder.get_all_variable_set();
    for(auto variable : variables)
    {
      if(!variable_node_map.count(variable))continue;
      VariableNode *var_node = variable_node_map[variable];
      variable_set_t vars;
      visit_node(var_node, constraints, module_set, vars);
    }
  }
  else
  {
    variable_set_t vars;
    TellNode *tell_node = constraint_it->second;
    visit_node(tell_node, constraints, module_set, vars);
  }
}


void RelationGraph::get_related_constraints(const Variable &var, ConstraintStore &constraints, module_set_t &module_set){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  get_related_constraints_core(var, constraints, module_set);
}


void RelationGraph::get_related_constraints_core(const Variable &var, ConstraintStore &constraints, module_set_t &module_set){
  if(!variable_node_map.count(var))return;
  VariableNode *var_node = variable_node_map[var];
  if(var_node == nullptr)throw HYDLA_SIMULATE_ERROR("VariableNode is not found");
  variable_set_t vars;
  visit_node(var_node, constraints, module_set, vars);
}


void RelationGraph::check_connected_components(){
  connected_constraints_vector.clear();
  connected_modules_vector.clear();
  connected_variables_vector.clear();
  referred_variables.clear();
  initialize_node_visited();

  for(auto tell_node : tell_nodes){
    module_set_t ms;
    ConstraintStore constraints;
    variable_set_t vars;
    if(!tell_node->visited && tell_node->is_active()){
      visit_node(tell_node, constraints, ms, vars);
      connected_constraints_vector.push_back(constraints);
      connected_modules_vector.push_back(ms);
      connected_variables_vector.push_back(vars);
      referred_variables.insert(vars.begin(), vars.end());
    }
  }
  up_to_date = true;
}

void RelationGraph::visit_node(TellNode* node, ConstraintStore &constraints, module_set_t &ms, variable_set_t &vars){
  node->visited = true;
  ms.add_module(node->module);
  constraints.add_constraint(node->constraint);
  for(auto edge : node->edges)
  {
    if(!ignore_prev || !edge.ref_prev) visit_node(edge.variable_node, constraints, ms, vars);
  }
}

void RelationGraph::visit_node(VariableNode* node, ConstraintStore &constraints, module_set_t &ms, variable_set_t &vars){
  vars.insert(node->variable);
  for(auto edge : node->edges)
  {
    if( (!ignore_prev || !edge.ref_prev)
        && !edge.tell_node->visited && edge.tell_node->is_active()) visit_node(edge.tell_node, constraints, ms, vars);
  }
}

int RelationGraph::get_connected_count()
{
  if(!up_to_date) check_connected_components();
  return connected_constraints_vector.size();
}

void RelationGraph::set_adopted(const module_t &mod, bool adopted)
{
  if(!module_tell_nodes_map.count(mod))throw HYDLA_SIMULATE_ERROR("module " + mod.first + " is not found");
  for(auto tell_node : module_tell_nodes_map[mod]) tell_node->module_adopted = adopted;
  for(auto ask_node : module_ask_nodes_map[mod]) ask_node->module_adopted = adopted;
  up_to_date = false;
}

void RelationGraph::set_adopted(const module_set_t &ms, bool adopted)
{
  for(auto module : ms)
  {
    set_adopted(module, adopted);
  }
}

void RelationGraph::set_expanded_atomic(constraint_t cons, bool expanded)
{
  auto tell_node_it = tell_node_map.find(cons);
  if(tell_node_it != tell_node_map.end())
  {
    tell_node_it->second->expanded = expanded;
  }
  else
  {
    auto ask_node_it = ask_node_map.find(cons);
    if(ask_node_it != ask_node_map.end())
    {
      ask_node_it->second->expanded = expanded;
    }else throw HYDLA_SIMULATE_ERROR("constraint_node not found");
  }
  up_to_date = false;
}

void RelationGraph::set_expanded_recursive(constraint_t cons, bool expanded)
{
  visit_mode = expanded?EXPANDING:UNEXPANDING;
  accept(cons);
  up_to_date = false;
}


list<constraint_t> RelationGraph::set_entailed(const ask_t &ask, bool entailed)
{
  auto node_it = ask_node_map.find(ask);
  always_list.clear();
  if(node_it != ask_node_map.end())
  {
    set_expanded_recursive(ask->get_child(), entailed);
    up_to_date = false;
    node_it->second->entailed = entailed;
  }
  return always_list;
}

bool RelationGraph::get_entailed(const ask_t &ask)const
{
  auto node_it = ask_node_map.find(ask);
  if(node_it == ask_node_map.end()) throw HYDLA_SIMULATE_ERROR("AskNode is not found");
  return node_it->second->entailed;
}

bool RelationGraph::entail_if_prev(const ask_t &ask, bool entailed, list<constraint_t> &always)
{
  auto node_it = ask_node_map.find(ask);
  if(node_it == ask_node_map.end())return false;
  if(node_it->second->prev)
  {
    always_list.clear();
    up_to_date = false;
    set_expanded_recursive(ask->get_child(), entailed);
    node_it->second->entailed = entailed;
    always = always_list;
    return true;
  }
  return false;
}

RelationGraph::asks_t RelationGraph::get_adjacent_asks(const string &var, bool ignore_prev_asks){
  asks_t asks;
  if(!variable_node_map.count(var))return asks;
  VariableNode *var_node = variable_node_map[var];
  if(var_node == nullptr)throw HYDLA_SIMULATE_ERROR("VariableNode is not found");
  for(auto ask_node : var_node->ask_edges)
  {
    if(active(ask_node, ignore_prev_asks))
    {
      asks.push_back(ask_node->ask);
    }
  }
  return asks;
}


set<string> RelationGraph::get_adjacent_variables(const ask_t &ask){
  set<string> vars;
  if(!ask_node_map.count(ask))return vars;
  AskNode *ask_node = ask_node_map[ask];
  for(auto var_node: ask_node->edges)
  {
    vars.insert(var_node.variable_node->variable.get_name());
  }
  return vars;
}

RelationGraph::asks_t RelationGraph::get_active_asks(bool ignore_prev_asks)
{
  asks_t asks;
  for(auto ask_node : ask_nodes)
  {
    if(active(ask_node, ignore_prev_asks))asks.push_back(ask_node->ask);
  }
  return asks;
}

RelationGraph::AskNode::AskNode(const ask_t &a, const module_t &mod):ConstraintNode(mod), ask(a)
{
  VariableFinder finder;
  finder.visit_node(ask->get_guard());
  prev = finder.get_variable_set().empty();
}

RelationGraph::variable_set_t RelationGraph::get_variables(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  if(index >= connected_variables_vector.size())throw HYDLA_SIMULATE_ERROR("index is out of range");
  return connected_variables_vector[index];
}

bool RelationGraph::active(const AskNode *ask, bool ignore_prev) const
{
  // TODO: activeという名前が重複しているのでどうにかする
  return ask->is_active() && !(ignore_prev && ask->prev);
}


ConstraintStore RelationGraph::get_constraints(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  if(index >= connected_constraints_vector.size())throw HYDLA_SIMULATE_ERROR("index is out of range");
  return connected_constraints_vector[index];
}

ConstraintStore RelationGraph::get_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto tell_node : tell_nodes)
  {
    if(tell_node->is_active())
    {
      constraints.add_constraint(tell_node->constraint);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_expanded_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto tell_node : tell_nodes)
  {
    if(tell_node->expanded)
    {
      constraints.add_constraint(tell_node->constraint);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_adopted_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto tell_node : tell_nodes)
  {
    if(tell_node->module_adopted)
    {
      constraints.add_constraint(tell_node->constraint);
    }
  }
  return constraints;
}

RelationGraph::module_set_t RelationGraph::get_modules(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  if(index >= connected_modules_vector.size())throw HYDLA_SIMULATE_ERROR("index is out of range");
  return connected_modules_vector[index];
}

void RelationGraph::set_ignore_prev(bool ignore)
{
  ignore_prev = ignore;
  up_to_date = false;
}

void RelationGraph::visit_atomic_constraint(boost::shared_ptr<symbolic_expression::BinaryNode> node)
{
  if(visit_mode == ADDING)
  {
    VariableFinder finder;
    finder.visit_node(node);
    variable_set_t variables;
    
    TellNode* cons;
    if(tell_node_map.count(node))
    {
      cons = tell_node_map[node];
    }
    else
    {
      cons = new TellNode(node, current_module);
      cons->always = in_always;
      tell_nodes.push_back(cons);
      tell_node_map[node] = cons;
      module_tell_nodes_map[current_module].push_back(cons);
    }

    variables = finder.get_variable_set();
    for(auto variable : variables)
    {
      VariableNode* var_node = add_variable_node(variable);
      cons->edges.push_back(EdgeToVariable(var_node, false));
      var_node->edges.push_back(EdgeToConstraint(cons, false));
    }

    variable_set_t prev_variables;
    prev_variables = finder.get_prev_variable_set();
    for(auto variable : prev_variables)
    {
      if(variables.count(variable))continue;
      VariableNode* var_node = add_variable_node(variable);
      cons->edges.push_back(EdgeToVariable(var_node, true));
      var_node->edges.push_back(EdgeToConstraint(cons, true));
    }
  }
  else if(visit_mode == EXPANDING)
  {
    auto tell_node_it = tell_node_map.find(node);
    if(tell_node_it != tell_node_map.end())
    {
      TellNode* tell_node = tell_node_it->second;
      tell_node->expanded = true;
      if(in_always)always_list.push_back(node);
      else nonalways_list.push_back(node);
    }
    else HYDLA_LOGGER_WARN("(@RelationGraph) try to expand unknown node: ", get_infix_string(node));
  }
  else if(visit_mode == UNEXPANDING)
  {
    auto tell_node_it = tell_node_map.find(node);
    if(tell_node_it != tell_node_map.end())
    {
      TellNode* tell_node = tell_node_it->second;
      if(!tell_node->always)
      {
        tell_node->expanded = false;
      }
    }
    else HYDLA_LOGGER_WARN("(@RelationGraph) try to unexpand unknown node: ", get_infix_string(node));
  }
}

RelationGraph::VariableNode* RelationGraph::add_variable_node(Variable &var)
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


void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Ask> ask)
{
  if(visit_mode == ADDING)
  {
    VariableFinder finder;
    finder.visit_node(ask->get_guard());
    variable_set_t variables;
    
    AskNode* ask_node;
    assert(!ask_node_map.count(ask)); /// assume that same ask node doesn't exist
    ask_node = new AskNode(ask, current_module);
    ask_nodes.push_back(ask_node);
    ask_node_map[ask] = ask_node;
    ask_node->parent = parent_ask;
    ask_node->always = in_always;

    variables = finder.get_all_variable_set();
    for(auto variable : variables)
    {
      VariableNode* var_node = add_variable_node(variable);
      EdgeToVariable edge(var_node, !ask_node->prev);
      ask_node->edges.push_back(edge);
      var_node->ask_edges.push_back(ask_node);
    }
    bool prev_in_always = in_always;
    in_always = false;
    parent_ask = ask_node;
    accept(ask->get_rhs());
    parent_ask = ask_node->parent;
    in_always = prev_in_always;
  }
  else
  {
    auto ask_node_it = ask_node_map.find(ask);
    if(ask_node_it == ask_node_map.end())throw HYDLA_SIMULATE_ERROR("ask_node not found");
    if(visit_mode == EXPANDING)
    {
      if(in_always)always_list.push_back(ask);
      else nonalways_list.push_back(ask);
      ask_node_it->second->expanded = true;
    }
    else if(visit_mode == UNEXPANDING && !in_always)
    {
      ask_node_it->second->expanded = false;
    }
    else throw HYDLA_SIMULATE_ERROR("unknown visit_mode");
  }
}

void RelationGraph::visit(boost::shared_ptr<symbolic_expression::Always> node)
{
  if(visit_mode == UNEXPANDING)return;
  bool prev_in_always = in_always;
  in_always = true;
  accept(node->get_child());
  in_always = prev_in_always;
}


} //namespace simulator
} //namespace hydla 
