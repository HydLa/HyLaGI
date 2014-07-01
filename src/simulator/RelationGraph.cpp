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

void RelationGraph::dump_graph(ostream & os) const
{
  os << "graph g {\n";
  os << "graph [ranksep = 2.0 ,rankdir = LR];\n";
  for(auto constraint_node : constraint_nodes) {
    string constraint_name = constraint_node->get_name();
    os << "  \"" << constraint_name << "\" [shape = box]\n";
    for(auto edge : constraint_node->edges){
      string variable_name = edge.variable_node->get_name();
      os << "  \"" 
        << constraint_name 
        << "\" -- \"" 
        << variable_name 
        << "\"";
      if(edge.ref_prev)
      {
        os << "[label = \"prev\"]";
      }
      os <<  ";\n";
    }
  }
  os << "}" << endl;
}

void RelationGraph::dump_active_graph(ostream & os) const
{
  os << "graph g {" << endl;
  os << "graph [ranksep = 2.0 ,rankdir = LR];" << endl;
  for(auto constraint_node : constraint_nodes) {
    if(constraint_node->active())
    {
      string constraint_name = constraint_node->get_name();
      os << "  \"" << constraint_name;
      os << "\" [shape = box];" << endl;
      for(auto edge : constraint_node->edges){
        if(!(ignore_prev && edge.ref_prev) )
        {
          string variable_name = edge.variable_node->get_name();
          os << "  \"" << constraint_name 
             <<   "\" -- \"" 
             << variable_name
             << "\"";
          if(edge.ref_prev)
          {
            os << "[label = \"prev\"]";
          }
          os <<  ";" << endl;
        }
      }

    }
  }

  os << "}" << endl;
}


RelationGraph::EdgeToConstraint::EdgeToConstraint(ConstraintNode *cons, bool prev)
  : constraint_node(cons), ref_prev(prev){}

RelationGraph::EdgeToVariable::EdgeToVariable(VariableNode *var, bool prev)
  : variable_node(var), ref_prev(prev){}

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

bool RelationGraph::ConstraintNode::active() const
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
  for(auto constraint_node : constraint_nodes){
    constraint_node->visited = !constraint_node->active();
  }
}


void RelationGraph::get_related_constraints(constraint_t constraint, ConstraintStore &constraints, module_set_t &module_set){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  auto constraint_it = constraint_node_map.find(constraint);
  if(constraint_it == constraint_node_map.end())
  {
    VariableFinder finder;
    finder.visit_node(constraint);
    variable_set_t variables;
    variables = finder.get_all_variable_set();
    for(auto variable : variables)
    {
      assert(variable_node_map.count(variable));
      VariableNode *var_node = variable_node_map[variable];
      variable_set_t vars;
      visit_node(var_node, constraints, module_set, vars);
    }
  }
  else
  {
    variable_set_t vars;
    ConstraintNode *constraint_node = constraint_it->second;
    visit_node(constraint_node, constraints, module_set, vars);
  }
}


void RelationGraph::get_related_constraints(const Variable &var, ConstraintStore &constraints, module_set_t &module_set){
  if(!up_to_date) check_connected_components();
  initialize_node_visited();
  constraints.clear();
  module_set.clear();
  assert(variable_node_map.count(var));
  VariableNode *var_node = variable_node_map[var];
  assert(var_node != nullptr);
  variable_set_t vars;
  visit_node(var_node, constraints, module_set, vars);
}


void RelationGraph::check_connected_components(){
  connected_constraints_vector.clear();
  connected_modules_vector.clear();
  connected_variables_vector.clear();
  referred_variables.clear();
  initialize_node_visited();

  for(auto constraint_node : constraint_nodes){
    module_set_t ms;
    ConstraintStore constraints;
    variable_set_t vars;
    if(!constraint_node->visited){
      visit_node(constraint_node, constraints, ms, vars);
      connected_constraints_vector.push_back(constraints);
      connected_modules_vector.push_back(ms);
      connected_variables_vector.push_back(vars);
      referred_variables.insert(vars.begin(), vars.end());
    }
  }
  up_to_date = true;
}

void RelationGraph::visit_node(ConstraintNode* node, ConstraintStore &constraints, module_set_t &ms, variable_set_t &vars){
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
      && !edge.constraint_node->visited) visit_node(edge.constraint_node, constraints, ms, vars);
  }
}

RelationGraph::RelationGraph(const module_set_t &ms)
{
  for(auto module : ms)
  {
    add(module);
  }
  ignore_prev = true;
  up_to_date = false;
}

int RelationGraph::get_connected_count()
{
  if(!up_to_date) check_connected_components();
  return connected_constraints_vector.size();
}

void RelationGraph::set_adopted(const module_t &mod, bool adopted)
{
  assert(module_constraint_nodes_map.count(mod));
  for(auto constraint_node : module_constraint_nodes_map[mod])
  {
    constraint_node->module_adopted = adopted;
  }
  up_to_date = false;
}

void RelationGraph::set_adopted(const module_set_t &ms)
{
  for(auto entry : module_constraint_nodes_map)
  {
    bool adopted = !(ms.find(entry.first) == ms.end());
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

void RelationGraph::set_changing_constraints(const ConstraintStore& constraints)
{
  if(!up_to_date){
    check_connected_components();
  }
  initialize_node_visited();
  ConstraintStore tmp_constraints,result_constraints;
  module_set_t module_set;
  //TODO: IPでprevを区別する
  for(auto constraint : constraints){
    get_related_constraints(constraint, tmp_constraints, module_set);
    changing_constraints.add_constraint_store(tmp_constraints);
  }
}

RelationGraph::variable_set_t RelationGraph::get_variables(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  assert(index < connected_variables_vector.size());
  return connected_variables_vector[index];
}

ConstraintStore RelationGraph::get_constraints(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  assert(index < connected_constraints_vector.size());
  return connected_constraints_vector[index];
}

ConstraintStore RelationGraph::get_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->active())
    {
      constraints.add_constraint(constraint_node->constraint);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_expanded_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->expanded)
    {
      constraints.add_constraint(constraint_node->constraint);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_adopted_constraints()
{
  if(!up_to_date) check_connected_components();
  ConstraintStore constraints;
  for(auto constraint_node : constraint_nodes)
  {
    if(constraint_node->module_adopted)
    {
      constraints.add_constraint(constraint_node->constraint);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_changing_constraints(){
  return changing_constraints;
}


RelationGraph::module_set_t RelationGraph::get_modules(unsigned int index)
{
  if(!up_to_date) check_connected_components();
  assert(index < connected_modules_vector.size());
  return connected_modules_vector[index];
}

void RelationGraph::set_ignore_prev(bool ignore)
{
  ignore_prev = ignore;
  up_to_date = false;
}

bool RelationGraph::is_changing(const ConstraintStore constraint_store)
{
  if(!up_to_date){
    check_connected_components();
    set_changing_constraints(changing_constraints);
  }
  bool ret = false;
  for(auto constraint : constraint_store){
    if(changing_constraints.count(constraint)){
      ret = true;
      break;
    }
  }
  return ret;
}

bool RelationGraph::is_changing(const constraint_t constraint)
{
  ConstraintStore constraint_store;
  module_set_t module_set;
  get_related_constraints(constraint, constraint_store, module_set);
  for(auto tmp_constraint : constraint_store){
    if(changing_constraints.count(tmp_constraint)) return true;
  }
  return false;
}

bool RelationGraph::is_changing(const Variable& variable)
{
  VariableFinder finder;
  for(auto constraint : changing_constraints){
    finder.visit_node(constraint);
  }
  return finder.include_variable(variable) || finder.include_variable_prev(variable);
}

void RelationGraph::clear_changing(){
  changing_constraints.clear();
}

void RelationGraph::visit_binary_node(boost::shared_ptr<symbolic_expression::BinaryNode> node)
{
  if(visit_mode == ADDING)
  {
    VariableFinder finder;
    finder.visit_node(node);
    variable_set_t variables;
    
    ConstraintNode* cons;
    if(constraint_node_map.count(node))
    {
      cons = constraint_node_map[node];
    }
    else
    {
      cons = new ConstraintNode(node, current_module);
      constraint_nodes.push_back(cons);
      constraint_node_map[node] = cons; 
      module_constraint_nodes_map[current_module].push_back(cons);
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
    assert(constraint_node_map.count(node));
    constraint_node_map[node]->expanded = true;
  }
  else if(visit_mode == UNEXPANDING)
  {
    assert(constraint_node_map.count(node));
    constraint_node_map[node]->expanded = false;
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
