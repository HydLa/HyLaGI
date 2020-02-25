#include "RelationGraph.h"
#include "HydLaError.h"
#include "Logger.h"
#include "VariableFinder.h"
#include <iostream>

using namespace std;

namespace hydla {
namespace simulator {

RelationGraph::RelationGraph(const module_set_t &ms,
                             parser::ParseTreeSemanticAnalyzer *analyzer) {
  analyzer_ = analyzer;
  for (auto module : ms) {
    add(module);
  }
  current_guard_node = nullptr;
  ignore_prev = true;
}

RelationGraph::~RelationGraph() {
  for (auto var : variable_nodes) {
    delete var;
  }

  for (auto constraint : tell_nodes) {
    delete constraint;
  }
  for (auto ask : ask_nodes) {
    delete ask;
  }
  for (auto guard_node : guard_nodes) {
    delete guard_node;
  }
}

void RelationGraph::add(module_t &mod) {
  current_module = mod;
  visit_mode = ADDING;
  in_always = false;
  in_exists = 0;
  parent_ask = nullptr;
  expanding_caller_ = 0;
  first_ = true;
  accept(mod.second);
  first_ = false;
}

void RelationGraph::add_guard(constraint_t &guard) {
  visit_mode = ADDING_ASK;
  accept(guard);
}

void dump_tell_node(TellNode *node, ostream &os) {
  string constraint_name = node->get_name();
  os << " \"" << constraint_name << "\" [shape = box]\n";
  for (auto edge : node->edges) {
    string variable_name = edge.variable_node->get_name();
    os << "  \"" << constraint_name << "\" -> \"" << variable_name
       << "\"[dir = none]";
    if (edge.ref_prev) {
      os << " [style = dashed]";
    }
    os << ";\n";
  }
}

void dump_ask_node(AskNode *node, ostream &os) {
  string constraint_name = " \"" + node->get_name() + "\"";
  if (node->prev)
    os << constraint_name << " [shape = hexagon, style = dashed]\n";
  else
    os << constraint_name << " [shape = hexagon]\n";
  for (auto edge : node->edges) {
    string variable_name = edge.variable_node->get_name();
    os << constraint_name << " -> \"" << variable_name << "\" [dir = none]";
    if (edge.ref_prev) {
      os << " [style = dashed]";
    }
    os << ";\n";
  }
  for (auto child : node->children) {
    os << constraint_name << " -> \""
       << child->get_name()
       // arrowtail is not displayed if dir != both
       << "\" [dir = both, arrowtail = odot]; \n";
  }
}

void RelationGraph::dump_graph(ostream &os, DumpMode mode) const {
  os << "digraph g {\n";
  os << "graph [ranksep = 2.0 ,rankdir = LR];\n";
  if (mode != ASK_ONLY) {
    for (auto tell_node : tell_nodes) {
      dump_tell_node(tell_node, os);
    }
  }

  if (mode != TELL_ONLY) {
    for (auto ask_node : ask_nodes) {
      dump_ask_node(ask_node, os);
    }
  }
  os << "}" << endl;
}

void RelationGraph::dump_active_graph(ostream &os, DumpMode mode) const {
  os << "digraph g {" << endl;
  os << "graph [ranksep = 2.0 ,rankdir = LR];" << endl;
  if (mode != ASK_ONLY) {
    for (auto tell_node : tell_nodes) {
      if (tell_node->is_active())
        dump_tell_node(tell_node, os);
    }
  }
  if (mode != TELL_ONLY) {
    for (auto ask_node : ask_nodes) {
      if (ask_node->is_active())
        dump_ask_node(ask_node, os);
    }
  }
  os << "}" << endl;
}

EdgeToConstraint::EdgeToConstraint(TellNode *cons, bool prev)
    : tell_node(cons), ref_prev(prev) {}

EdgeToVariable::EdgeToVariable(VariableNode *var, bool prev)
    : variable_node(var), ref_prev(prev) {}

string VariableNode::get_name() const { return variable.get_string(); }

string get_constraint_name(const constraint_t &constraint,
                           const module_t &module) {
  string ret = symbolic_expression::get_infix_string(constraint);
  // if too long, cut latter part
  const string::size_type max_length = 10;
  if (ret.length() > max_length) {
    ret = ret.substr(0, max_length) + ("...");
  }
  return ret + " (" + module.first + ")";
}

string TellNode::get_name() const {
  return (always ? "[]" : "") + get_constraint_name(tell, module);
}

string AskNode::get_name() const {
  return (always ? "[]" : "") + get_constraint_name(ask->get_guard(), module);
}

bool ConstraintNode::is_active() const { return expanded && module_adopted; }

bool RelationGraph::referring(const Variable &var) {
  auto var_it = variable_node_map.find(var);
  if (var_it == variable_node_map.end())
    return false;
  VariableNode *var_node = var_it->second;
  for (auto edge : var_node->edges) {
    if (edge.tell_node->is_active())
      return true;
  }
  return false;
}

void RelationGraph::initialize_node_collected() {
  for (auto tell_node : tell_nodes) {
    tell_node->collected = false;
  }
}

void RelationGraph::get_related_constraints_vector(
    const ConstraintStore &constraint_store,
    vector<ConstraintStore> &constraints_vector,
    vector<module_set_t> &module_set_vector) {
  initialize_node_collected();
  constraints_vector.clear();
  module_set_vector.clear();

  for (auto constraint : constraint_store) {
    auto constraint_it = tell_node_map.find(constraint);
    if (constraint_it == tell_node_map.end()) {
      VariableFinder finder;
      finder.visit_node(constraint);
      variable_set_t variables;
      variables = finder.get_all_variable_set();
      for (auto base_variable : variables) {
        for (int i = 0; i <= base_variable.get_differential_count(); i++) {
          Variable variable(base_variable.get_name(), i);
          if (!variable_node_map.count(variable))
            continue;
          ConstraintStore connected_constraints;
          module_set_t connected_ms;
          VariableNode *var_node = variable_node_map[variable];
          collect_node(var_node, &connected_constraints, &connected_ms, nullptr,
                       false);
          if (connected_constraints.size() > 0) {
            constraints_vector.push_back(connected_constraints);
            module_set_vector.push_back(connected_ms);
          }
        }
      }
    } else {
      TellNode *tell_node = constraint_it->second;
      if (!tell_node->collected) {
        ConstraintStore connected_constraints;
        module_set_t connected_ms;
        if (tell_node->is_active()) {
          collect_node(tell_node, &connected_constraints, &connected_ms,
                       nullptr, false);
          constraints_vector.push_back(connected_constraints);
          module_set_vector.push_back(connected_ms);
        } else {
          // adjacent node may be active (this case is mainly caused by negative
          // asks)
          for (auto edge : tell_node->edges) {
            if (!ignore_prev || !edge.ref_prev)
              collect_node(edge.variable_node, &connected_constraints,
                           &connected_ms, nullptr, false);
          }
          if (connected_constraints.size() > 0) {
            constraints_vector.push_back(connected_constraints);
            module_set_vector.push_back(connected_ms);
          }
        }
      }
    }
  }
}

module_set_t
RelationGraph::get_related_modules(const ConstraintStore &constraint_store) {
  initialize_node_collected();
  module_set_t ret;
  for (auto constraint : constraint_store) {
    auto constraint_it = tell_node_map.find(constraint);
    if (constraint_it == tell_node_map.end()) {
      VariableFinder finder;
      finder.visit_node(constraint);
      variable_set_t variables;
      variables = finder.get_all_variable_set();
      for (auto base_variable : variables) {
        for (int i = 0; i <= base_variable.get_differential_count(); i++) {
          Variable variable(base_variable.get_name(), i);
          if (!variable_node_map.count(variable))
            continue;
          VariableNode *var_node = variable_node_map[variable];
          collect_node(var_node, nullptr, &ret, nullptr, true);
        }
      }
    } else {
      TellNode *tell_node = constraint_it->second;
      if (!tell_node->collected) {
        if (tell_node->is_active()) {
          collect_node(tell_node, nullptr, &ret, nullptr, true);
        } else {
          // adjacent node may be active (this case is mainly caused by negative
          // asks)
          for (auto edge : tell_node->edges) {
            if (!ignore_prev || !edge.ref_prev)
              collect_node(edge.variable_node, nullptr, &ret, nullptr, true);
          }
        }
      }
    }
  }
  return ret;
}

void RelationGraph::get_related_constraints_vector(
    const ConstraintStore &constraint_store, const variable_set_t &variables,
    vector<ConstraintStore> &constraints_vector,
    vector<module_set_t> &module_set_vector) {
  get_related_constraints_vector(constraint_store, constraints_vector,
                                 module_set_vector);
  for (auto var : variables) {
    ConstraintStore constraints;
    module_set_t modules;
    get_related_constraints_core(var, constraints, modules);
    if (!constraints.empty()) {
      constraints_vector.push_back(constraints);
      module_set_vector.push_back(modules);
    }
  }
}

void RelationGraph::get_related_constraints(constraint_t constraint,
                                            ConstraintStore &constraints) {
  initialize_node_collected();
  constraints.clear();
  auto constraint_it = tell_node_map.find(constraint);
  if (constraint_it == tell_node_map.end()) {
    VariableFinder finder;
    finder.visit_node(constraint);
    variable_set_t variables;
    variables = finder.get_all_variable_set();
    for (auto variable : variables) {
      if (!variable_node_map.count(variable))
        continue;
      VariableNode *var_node = variable_node_map[variable];
      variable_set_t vars;
      collect_node(var_node, &constraints, nullptr, nullptr, false);
    }
  } else {
    variable_set_t vars;
    TellNode *tell_node = constraint_it->second;
    collect_node(tell_node, &constraints, nullptr, nullptr, false);
  }
}

void RelationGraph::get_related_constraints_core(const Variable &var,
                                                 ConstraintStore &constraints,
                                                 module_set_t &module_set) {
  VariableNode *var_node = variable_node_map[var];
  if (var_node == nullptr)
    throw HYDLA_ERROR("VariableNode is not found");
  collect_node(var_node, &constraints, &module_set, nullptr, false);
}

void RelationGraph::collect_node(TellNode *node, ConstraintStore *constraints,
                                 module_set_t *module_set, variable_set_t *vars,
                                 bool visit_guards) {
  node->collected = true;
  if (constraints != nullptr)
    constraints->add_constraint(node->tell);
  if (module_set != nullptr)
    module_set->add_module(node->module);
  for (auto edge : node->edges) {
    if (!ignore_prev || !edge.ref_prev)
      collect_node(edge.variable_node, constraints, module_set, vars,
                   visit_guards);
  }
  if (visit_guards && node->parent != nullptr)
    collect_node(node->parent, constraints, module_set, vars, visit_guards);
}

void RelationGraph::collect_node(AskNode *node, ConstraintStore *constraints,
                                 module_set_t *module_set, variable_set_t *vars,
                                 bool visit_guards) {
  node->collected = true;
  if (module_set != nullptr)
    module_set->add_module(node->module);
  for (auto edge : node->edges) {
    if (!ignore_prev || !edge.ref_prev)
      collect_node(edge.variable_node, constraints, module_set, vars,
                   visit_guards);
  }
  if (visit_guards && node->parent != nullptr)
    collect_node(node->parent, constraints, module_set, vars, visit_guards);
}

void RelationGraph::collect_node(VariableNode *node,
                                 ConstraintStore *constraints,
                                 module_set_t *module_set, variable_set_t *vars,
                                 bool visit_guards) {
  if (vars != nullptr)
    vars->insert(node->variable);
  for (auto edge : node->edges) {
    if ((!ignore_prev || !edge.ref_prev) && !edge.tell_node->collected &&
        edge.tell_node->is_active())
      collect_node(edge.tell_node, constraints, module_set, vars, visit_guards);
  }
}

// 後件existsの制約に新しい変数をbindして複製する
void RelationGraph::clone_exists_constraint(ask_t parent, constraint_t conseq) {
  auto it = ask_node_map.find(parent);
  if (it == ask_node_map.end())
    throw HYDLA_ERROR("AskNode for " + get_infix_string(parent) +
                      " is not found");
  AskNode *parent_asknode = (*it).second;
  exists_asknodes_.push_back(parent_asknode);

  current_module = parent_asknode->module;
  parent_ask = parent_asknode;
  visit_mode = ADDING;

  accept(conseq);
  for (auto child : parent_asknode->children) {
    child->expanded = true;
  }
}

void RelationGraph::expand_caller(ask_t ask) {
  auto it = ask_node_map.find(ask);
  if (it == ask_node_map.end())
    throw HYDLA_ERROR("AskNode for " + get_infix_string(ask) + " is not found");
  AskNode *parent_asknode = (*it).second;
  std::cout << "expand_caller:" << std::endl;
  std::cout << ask->get_string() << std::endl;

  auto it2 = ask_caller_map_.find(parent_asknode);
  if (it2 == ask_caller_map_.end())
    return;

  auto callers = it2->second;
  parent_ask = parent_asknode;
  visit_mode = ADDING;
  in_always = false;
  in_exists = 0;

  for (auto caller : callers) {
    caller->set_child(nullptr);
    std::cout << "Analyze:"<< std::endl;
    std::cout << caller->get_string() << std::endl;
    symbolic_expression::node_sptr ptr = caller;
    analyzer_->analyze(ptr);
    accept(ptr);
    std::cout << "Result:" << std::endl;
    std::cout << ptr->get_string() << std::endl;
  }
}

void RelationGraph::disable_exists_nonalways() {
  for (auto asknode : exists_asknodes_) {
    for (auto child : asknode->children) {
      if (!child->always) {
        child->expanded = false;
      }
    }
  }
}

void RelationGraph::set_adopted(const module_t &mod, bool adopted) {
  if (!module_tell_nodes_map.count(mod))
    throw HYDLA_ERROR("module " + mod.first + " is not found");
  for (auto tell_node : module_tell_nodes_map[mod])
    tell_node->module_adopted = adopted;
  for (auto ask_node : module_ask_nodes_map[mod])
    ask_node->module_adopted = adopted;
}

void RelationGraph::set_adopted(const module_set_t &ms, bool adopted) {
  for (auto module : ms) {
    set_adopted(module, adopted);
  }
}

void RelationGraph::set_expanded_atomic(constraint_t cons, bool expanded) {
  auto constraint_node_it = constraint_node_map.find(cons);
  if (constraint_node_it != constraint_node_map.end()) {
    constraint_node_it->second->expanded = expanded;
  } else
    throw HYDLA_ERROR("constraint_node not found");
}

void RelationGraph::set_expanded_recursive(constraint_t cons, bool expanded) {
  visit_mode = expanded ? EXPANDING : UNEXPANDING;
  accept(cons);
}

void RelationGraph::set_entailed(const ask_t &ask, bool entailed) {
  auto node_it = ask_node_map.find(ask);
  if (node_it != ask_node_map.end()) {
    node_it->second->entailed = entailed;
    if (node_it->second->expanded) {
      set_expanded_recursive(node_it->second->ask->get_child(), entailed);
    }
  } else
    throw HYDLA_ERROR("AtomicGuardNode for " + get_infix_string(ask) +
                      " is not found");
}

ConstraintStore RelationGraph::get_always_list(const ask_t &ask) const {
  auto it = ask_node_map.find(ask);
  if (it == ask_node_map.end())
    throw HYDLA_ERROR("AskNode for " + get_infix_string(ask) + " is not found");
  return it->second->always_children;
}

bool RelationGraph::get_entailed(const ask_t &ask) const {
  auto node_it = ask_node_map.find(ask);
  if (node_it == ask_node_map.end())
    throw HYDLA_ERROR("AskNode is not found");
  return node_it->second->entailed;
}

bool RelationGraph::entail_if_prev(const ask_t &ask, bool entailed) {
  auto node_it = ask_node_map.find(ask);
  if (node_it == ask_node_map.end())
    return false;
  if (node_it->second->prev) {
    node_it->second->entailed = entailed;
    if (node_it->second->expanded) {
      set_expanded_recursive(node_it->second->ask->get_child(), entailed);
    }
    return true;
  }
  return false;
}

asks_t RelationGraph::get_adjacent_asks(const string &var_name,
                                        bool ignore_prev_asks) {
  asks_t asks;
  list<VariableNode *> var_nodes;
  for (auto node : var_name_nodes_map[var_name]) {
    var_nodes.push_back(node);
  }
  if (var_nodes.empty())
    throw HYDLA_ERROR("VariableNode " + var_name + " is not found");
  for (auto var_node : var_nodes) {
    for (auto ask_node : var_node->ask_edges) {
      if (to_be_considered(ask_node, ignore_prev_asks)) {
        asks.insert(ask_node->ask);
      }
    }
  }
  std::cout << "" << std::endl;
  std::cout << "Adjacent Ask!" << std::endl;
  for (auto ask_node : ask_nodes) {
    if (ask_node->expanded && !ask_node->entailed) {
      VariableFinder finder;
      finder.visit_node(ask_node->ask->get_guard());
      if(finder.get_all_variable_set().empty()){
        std::cout << ask_node->ask->get_string() << std::endl;
        std::cout << ask_node->ask << std::endl;
        std::cout << ask_node->expanded << std::endl;
        std::cout << " " << std::endl;
        asks.insert(ask_node->ask);
      }
    }
  }
  return asks;
}

asks_t
RelationGraph::get_adjacent_asks2var_and_derivatives(const Variable &var,
                                                     bool ignore_prev_asks) {
  asks_t asks;
  Variable tmp_var(var.get_name(), var.get_differential_count());
  for (int n = tmp_var.get_differential_count() + 1;
       variable_node_map.find(tmp_var) != variable_node_map.end(); n++) {
    VariableNode *var_node = variable_node_map[tmp_var];
    for (auto ask_node : var_node->ask_edges) {
      if (to_be_considered(ask_node, ignore_prev_asks)) {
        asks.insert(ask_node->ask);
      }
    }
    tmp_var = Variable(tmp_var.get_name(), n);
  }
  return asks;
}

list<AtomicConstraint *>
RelationGraph::get_atomic_guards(const constraint_t &guard) const {
  auto node_it = guard_node_map.find(guard);
  if (node_it == guard_node_map.end())
    throw HYDLA_ERROR("unknown guard:" + get_infix_string(guard));
  return node_it->second->get_atomic_guards();
}

set<string> RelationGraph::get_adjacent_variables(const ask_t &ask) {
  set<string> vars;
  if (!ask_node_map.count(ask))
    return vars;
  AskNode *ask_node = ask_node_map[ask];
  for (auto var_node : ask_node->edges) {
    vars.insert(var_node.variable_node->variable.get_name());
  }
  return vars;
}

asks_t RelationGraph::get_active_asks(bool ignore_prev_asks) {
  asks_t asks;
  for (auto ask_node : ask_nodes) {
    if (to_be_considered(ask_node, ignore_prev_asks) && ask_node->entailed)
      asks.insert(ask_node->ask);
  }
  return asks;
}

ConstraintStore RelationGraph::get_active_tells() {
  ConstraintStore tells;
  for (auto tell_node : tell_nodes) {
    if (tell_node->is_active())
      tells.add_constraint(tell_node->tell);
  }
  return tells;
}

AskNode::AskNode(const ask_t &a, const module_t &mod, GuardNode *g)
    : ConstraintNode(mod), ask(a), entailed(false), guard_node(g) {
  VariableFinder finder;
  finder.visit_node(ask->get_guard());
  prev = finder.get_variable_set().empty();
}

variable_set_t RelationGraph::get_related_variables(constraint_t cons) {
  variable_set_t vars;

  initialize_node_collected();

  if (tell_node_map.count(cons) == 0) {
    VariableFinder finder;
    finder.visit_node(cons);
    for (auto var : finder.get_all_variable_set()) {
      if (variable_node_map.count(var)) {
        collect_node(variable_node_map[var], nullptr, nullptr, &vars, false);
      }
    }
    return vars;
  } else {
    TellNode *cons_node = tell_node_map[cons];
    collect_node(cons_node, nullptr, nullptr, &vars, false);
  }
  return vars;
}

bool RelationGraph::to_be_considered(const AskNode *ask,
                                     bool ignore_prev) const {
  return ask->expanded && !(ignore_prev && ask->prev);
}

ConstraintStore RelationGraph::get_constraints() {
  ConstraintStore constraints;
  for (auto tell_node : tell_nodes) {
    if (tell_node->is_active()) {
      constraints.add_constraint(tell_node->tell);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_expanded_constraints() {
  ConstraintStore constraints;
  for (auto tell_node : tell_nodes) {
    if (tell_node->expanded) {
      constraints.add_constraint(tell_node->tell);
    }
  }
  return constraints;
}

ConstraintStore RelationGraph::get_adopted_constraints() {
  ConstraintStore constraints;
  for (auto tell_node : tell_nodes) {
    if (tell_node->module_adopted) {
      constraints.add_constraint(tell_node->tell);
    }
  }
  return constraints;
}

asks_t RelationGraph::get_all_asks() {
  asks_t asks;
  for (auto ask_node : ask_nodes) {
    asks.insert(ask_node->ask);
  }
  return asks;
}

asks_t RelationGraph::get_expanded_asks() {
  asks_t asks;
  for (auto ask_node : ask_nodes) {
    if (ask_node->expanded) {
      asks.insert(ask_node->ask);
    }
  }
  return asks;
}

AskNode *RelationGraph::get_ask_node(const ask_t &ask) {
  auto node_it = ask_node_map.find(ask);
  if (node_it == ask_node_map.end())
    return nullptr;
  return node_it->second;
}

GuardNode *RelationGraph::get_guard_node(const constraint_t &guard) {
  auto node_it = guard_node_map.find(guard);
  if (node_it == guard_node_map.end())
    return nullptr;
  return node_it->second;
}

void RelationGraph::set_ignore_prev(bool ignore) { ignore_prev = ignore; }

constraints_t RelationGraph::get_all_guards() {
  constraints_t result;
  for (auto atomic_guard_node : atomic_guard_list) {
    result.insert(atomic_guard_node->atomic_guard.constraint);
  }
  return result;
}

void RelationGraph::visit_atomic_constraint(
    std::shared_ptr<symbolic_expression::Node> node) {
  if (visit_mode == ADDING_ASK) {
    AtomicConstraint atomic_guard;
    atomic_guard.constraint = node;
    AtomicGuardNode *atomic_node = new AtomicGuardNode(atomic_guard);
    current_guard_node = atomic_node;
    atomic_guard_list.push_back(atomic_node);
    guard_nodes.push_back(current_guard_node);
    guard_node_map[node] = atomic_node;
  }
  if (visit_mode == ADDING) {
    VariableFinder finder;
    finder.visit_node(node);
    variable_set_t variables;

    TellNode *tell_node;
    if (tell_node_map.count(node)) {
      tell_node = tell_node_map[node];
    } else {
      tell_node = new TellNode(node, current_module);
      tell_node->always = in_always;
      tell_node->parent = parent_ask;
      if (parent_ask != nullptr)
        parent_ask->children.push_back(tell_node);
      tell_nodes.push_back(tell_node);
      tell_node_map[node] = tell_node;
      constraint_node_map[node] = tell_node;
      module_tell_nodes_map[current_module].push_back(tell_node);
    }

    variables = finder.get_variable_set();
    for (auto variable : variables) {
      // for default continuity
      for (int i = 0; i <= variable.get_differential_count(); i++) {
        Variable differentiated_variable(variable.get_name(), i);
        VariableNode *var_node = add_variable_node(differentiated_variable);
        tell_node->edges.push_back(EdgeToVariable(var_node, false));
        var_node->edges.push_back(EdgeToConstraint(tell_node, false));
      }
    }

    variable_set_t prev_variables;
    prev_variables = finder.get_prev_variable_set();
    for (auto variable : prev_variables) {
      if (variables.count(variable))
        continue;
      VariableNode *var_node = add_variable_node(variable);
      tell_node->edges.push_back(EdgeToVariable(var_node, true));
      var_node->edges.push_back(EdgeToConstraint(tell_node, true));
    }
    if (in_always)
      always_list.add_constraint(node);
  } else if (visit_mode == EXPANDING) {
    auto tell_node_it = tell_node_map.find(node);
    if (tell_node_it != tell_node_map.end()) {
      TellNode *tell_node = tell_node_it->second;
      tell_node->expanded = true;
    } else
      HYDLA_LOGGER_WARN("(@RelationGraph) try to expand unknown node: ",
                        get_infix_string(node));
  } else if (visit_mode == UNEXPANDING) {
    auto tell_node_it = tell_node_map.find(node);
    if (tell_node_it != tell_node_map.end()) {
      TellNode *tell_node = tell_node_it->second;
      if (!tell_node->always) {
        tell_node->expanded = false;
      }
    } else
      HYDLA_LOGGER_WARN("(@RelationGraph) try to unexpand unknown node: ",
                        get_infix_string(node));
  }
}

VariableNode *RelationGraph::add_variable_node(Variable &var) {
  if (variable_node_map.count(var)) {
    return variable_node_map[var];
  } else {
    VariableNode *ret = new VariableNode(var);
    variable_nodes.push_back(ret);
    variable_node_map[var] = ret;
    var_name_nodes_map[var.get_name()].push_back(ret);
    return ret;
  }
}

void RelationGraph::visit(std::shared_ptr<symbolic_expression::Ask> ask) {
  if (visit_mode == ADDING) {
    if (in_always)
      always_list.add_constraint(ask);
    constraint_t guard = ask->get_guard();
    VariableFinder finder;
    finder.visit_node(guard);
    variable_set_t variables;

    AskNode *ask_node;
    assert(
        !ask_node_map.count(ask)); /// assume that same ask node doesn't exist
    visit_mode = ADDING_ASK;
    atomic_guard_list.clear();

    accept(guard);

    ask_node = new AskNode(ask, current_module, current_guard_node);
    guard_node_map[guard] = current_guard_node;
    current_guard_node->asks.push_back(ask_node);
    ask_node->atomic_guard_list = atomic_guard_list;

    ask_nodes.push_back(ask_node);
    ask_node_map[ask] = ask_node;
    ask_node->parent = parent_ask;
    if (parent_ask != nullptr)
      parent_ask->children.push_back(ask_node);
    ask_node->always = in_always;
    constraint_node_map[ask] = ask_node;

    variables = finder.get_all_variable_set();
    for (auto variable : variables) {
      VariableNode *var_node = add_variable_node(variable);
      EdgeToVariable edge(var_node, ask_node->prev);
      ask_node->edges.push_back(edge);
      var_node->ask_edges.push_back(ask_node);
    }
    bool prev_in_always = in_always;
    in_always = false;
    parent_ask = ask_node;
    ConstraintStore prev_always_list = always_list;
    always_list.clear();
    visit_mode = ADDING;
    accept(ask->get_rhs());
    ask_node->always_children = always_list;
    always_list = prev_always_list;
    parent_ask = ask_node->parent;
    in_always = prev_in_always;
  } else {
    auto ask_node_it = ask_node_map.find(ask);
    if (ask_node_it == ask_node_map.end())
      throw HYDLA_ERROR("ask_node not found");
    if (visit_mode == EXPANDING) {
      std::cout << "EXPAND!!!!!" << std::endl;
      std::cout << ask << std::endl;
      std::cout << ask->get_string() << std::endl;
      ask_node_it->second->expanded = true;
      if (ask_node_it->second->entailed)
        accept(ask_node_it->second->ask->get_child());
    } else if (visit_mode == UNEXPANDING && !in_always) {
      std::cout << "UNEXPAND!!!!!" << std::endl;
      std::cout << ask << std::endl;
      std::cout << ask->get_string() << std::endl;
      ask_node_it->second->expanded = false;
      if (ask_node_it->second->entailed) {
        accept(ask_node_it->second->ask->get_child());
      }
    } else
      throw HYDLA_ERROR("unknown visit_mode");
  }
}

void RelationGraph::visit(std::shared_ptr<symbolic_expression::Exists> exists) {
  in_exists++;
  accept(exists->get_variable());
  accept(exists->get_child());
  in_exists--;
}

void RelationGraph::visit(std::shared_ptr<symbolic_expression::Always> node) {
  if (visit_mode == UNEXPANDING)
    return;
  bool prev_in_always = in_always;
  in_always = true;
  accept(node->get_child());
  in_always = prev_in_always;
}

void RelationGraph::visit(
    std::shared_ptr<symbolic_expression::LogicalOr> logical_or) {
  if (visit_mode != ADDING_ASK) {
    accept(logical_or->get_lhs());
    accept(logical_or->get_rhs());
  } else {
    accept(logical_or->get_lhs());
    GuardNode *lhs_guard = current_guard_node;
    accept(logical_or->get_rhs());
    GuardNode *rhs_guard = current_guard_node;
    current_guard_node = new OrGuardNode(lhs_guard, rhs_guard);
    rhs_guard->parent = lhs_guard->parent = current_guard_node;
    guard_node_map[logical_or] = current_guard_node;
    guard_nodes.push_back(current_guard_node);
  }
}

void RelationGraph::visit(
    std::shared_ptr<symbolic_expression::LogicalAnd> logical_and) {
  if (visit_mode != ADDING_ASK) {
    accept(logical_and->get_lhs());
    accept(logical_and->get_rhs());
  } else {
    accept(logical_and->get_lhs());
    GuardNode *lhs_guard = current_guard_node;
    accept(logical_and->get_rhs());
    GuardNode *rhs_guard = current_guard_node;
    current_guard_node = new AndGuardNode(lhs_guard, rhs_guard);
    rhs_guard->parent = lhs_guard->parent = current_guard_node;
    guard_node_map[logical_and] = current_guard_node;
    guard_nodes.push_back(current_guard_node);
  }
}

void RelationGraph::visit(
    std::shared_ptr<symbolic_expression::Not> not_expr) {
  if (visit_mode != ADDING_ASK) {
    accept(not_expr->get_child());
  } else {
    accept(not_expr->get_child());
    GuardNode *child = current_guard_node;
    current_guard_node = new NotGuardNode(child);
    child->parent = current_guard_node;
    guard_node_map[not_expr] = current_guard_node;
    guard_nodes.push_back(current_guard_node);
  }
}

void RelationGraph::visit(
    std::shared_ptr<symbolic_expression::ConstraintCaller> caller) {
  if(in_exists > 0 && visit_mode == ADDING){
    std::cout << "VISIT" << std::endl;
    std::cout << "caller:" << std::endl;
    std::cout << caller->get_string() << std::endl;
    std::cout << "parent" << std::endl;
    std::cout << parent_ask->ask->get_string() << std::endl;
    std::cout << "" << std::endl;
    ask_caller_map_[parent_ask].push_back(caller);
  }
  accept(caller->get_child());
}

} // namespace simulator
} // namespace hydla
