#pragma once

#include "Variable.h"
#include "Node.h"
#include "ModuleSetContainer.h"
#include "ModuleSet.h"
#include "DefaultTreeVisitor.h"
#include "ConstraintStore.h"

namespace hydla {
namespace simulator {

/**
 * Graph to represent relations of guard conditions and variables
 */
class GuardRelationGraph: public symbolic_expression::DefaultTreeVisitor{

public:
  typedef hierarchy::ModuleSet module_set_t;
  typedef hierarchy::ModuleSet::module_t module_t;
  typedef std::set<Variable, VariableComparator> variable_set_t;  
  
  struct VariableNode;
  struct GuardNode;
  typedef std::vector<VariableNode* > var_nodes_t;
  typedef std::vector<GuardNode* >    guard_nodes_t;

  /**
   * Node for constraint
   */
  struct GuardNode{
    constraint_t guard;
    module_t module; /// module which the constraint belongs to
    std::vector<VariableNode *> edges;
    GuardNode(const constraint_t &g, const module_t &mod):guard(g), module(mod){}
    std::string get_name() const;
  };
  
  /**
   * Node for variable
   */
  struct VariableNode{
    Variable variable;
    std::vector<GuardNode *> edges;
    VariableNode(Variable var):variable(var)
    {}
    std::string get_name() const;
  };

  GuardRelationGraph(const module_set_t &mods);

  virtual ~GuardRelationGraph();  

  /**
   * Print the structure in graphviz format.
   */
  void dump_graph(std::ostream &os) const;

  /**
   * Get guards adjacent to given variable
   * @parameter constraints for output
   */
  void get_adjacent_guards(const Variable &variable, ConstraintStore &guards);

  /**
   * Get variables adjacent to given guard
   * @parameter constraints for output
   */
  void get_adjacent_variables(const constraint_t &guards, variable_set_t &variables);

  /**
   * Get all guards
   */
  ConstraintStore get_guards();

private:
  typedef std::map<Variable, VariableNode*> variable_map_t;  
  typedef std::vector<std::pair<constraint_t, Variable > > relation_t;
  
  void add(module_t &mod);

  VariableNode* add_variable_node(Variable &);
  
  void visit(boost::shared_ptr<symbolic_expression::Ask> node);

  var_nodes_t variable_nodes;
  guard_nodes_t guard_nodes;
  module_t current_module;

  std::map<constraint_t, GuardNode*> guard_node_map;
  std::map<Variable, VariableNode*> variable_node_map;
};

} //namespace simulator
} //namespace hydla 
