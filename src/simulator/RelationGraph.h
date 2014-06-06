#pragma once

#include "Variable.h"
#include "ModuleSetContainer.h"
#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * Graph to represent relations of constraints and variables
 */
class RelationGraph: public symbolic_expression::DefaultTreeVisitor{

public:
  typedef hierarchy::ModuleSet module_set_t;
  typedef hierarchy::ModuleSet::module_t module_t;
  typedef std::set<Variable, VariableComparator> variable_set_t;  
  typedef symbolic_expression::node_sptr constraint_t;
  typedef std::set<constraint_t> constraints_t;
  
  struct VariableNode;
  struct ConstraintNode;
  typedef std::vector<VariableNode* > var_nodes_t;
  typedef std::vector<ConstraintNode* > constraint_nodes_t;

  /**
   * Node for constraint
   */
  struct ConstraintNode{
    constraint_t constraint;
    module_t module; /// module which the constraint belongs to
    bool visited;
    bool module_adopted; /// whether the module is in ms
    bool expanded; /// whether the guard of the constraint is entailed
    var_nodes_t edges;
    ConstraintNode(const constraint_t &cons, const module_t &mod):constraint(cons), module(mod), module_adopted(true), expanded(true)
    {}
    std::string get_name() const;
  };
  
  /**
   * Node for variable
   */
  struct VariableNode{
    Variable variable;
    constraint_nodes_t edges;
    VariableNode(Variable var):variable(var)
    {}
    std::string get_name() const;
  };

  
  RelationGraph(const module_set_t &mods);

  virtual ~RelationGraph();  

  /**
   * Print the structure in graphviz format.
   */
  std::ostream& dump_graph(std::ostream & os) const;
  
  /**
   * Set a moduld adopted or not
   */
  void set_adopted(const module_t &mod, bool adopted);
  
  /**
   * Set modules in ms adopted and modules not in ms not adopted
   */
  void set_adopted(module_set_t *ms);

  /**
   * Set the constraint expanded or not
   */ 
  void set_expanded(constraint_t cons, bool expanded);
  
  /**
   * Get the number of connected component in the graph.
   */
  int get_connected_count();


  /**
   * Get constraints and modules related to given variable
   * @parameter constraints for output
   * @parameter modules for output
   */
  void get_related_constraints(const Variable &variable, constraints_t &constraints,
                               module_set_t &modules);

  /**
   * Get constraints and modules related to given variable
   * @parameter constraints for output
   * @parameter modules for output
   */
  void get_related_constraints(constraint_t constraint, constraints_t &constraints,
                               module_set_t &modules);


  /**
   * Get constraints corresponding to the connected component specified by index.
   */
  constraints_t get_constraints(unsigned int index);  

  /**
   * Get a ModuleSet included by the connected component specified by index.
   */ 
  module_set_t get_modules(unsigned int index);  

  /**
   * Get constraints which related to given variables
   */
  
  constraints_t get_constraints(const std::vector<Variable>& variables);

private:
  typedef std::map<Variable, VariableNode*> variable_map_t;  
  typedef std::vector<std::pair<constraint_t, Variable > > relation_t;
  
  void add(module_t &mod);
  
  void check_connected_components();
  void visit_node(ConstraintNode* node, constraints_t &constraints, module_set_t &ms);
  void visit_node(VariableNode* node, constraints_t &constraint, module_set_t &ms);
  void initialize_node_visited();
 

  void visit(boost::shared_ptr<symbolic_expression::Equal> node);
  void visit(boost::shared_ptr<symbolic_expression::UnEqual> node);
  void visit(boost::shared_ptr<symbolic_expression::Less> node);
  void visit(boost::shared_ptr<symbolic_expression::LessEqual> node);
  void visit(boost::shared_ptr<symbolic_expression::Greater> node);
  void visit(boost::shared_ptr<symbolic_expression::GreaterEqual> node);
  void visit(boost::shared_ptr<symbolic_expression::Ask> node);
  void visit_binary_node(boost::shared_ptr<symbolic_expression::BinaryNode> node);

  var_nodes_t variable_nodes;
  constraint_nodes_t constraint_nodes;
  module_t current_module;

  typedef enum{
    EXPANDING,
    UNEXPANDING,
    ADDING
  }VisitMode;
  
  std::vector<constraints_t> connected_constraints_vector;
  std::vector<module_set_t> connected_modules_vector;
  bool up_to_date;     /// if false, recalculation of connected components is necessary
  std::map<module_t, constraint_nodes_t>  module_constraint_nodes_map;
  std::map<constraint_t, ConstraintNode*> constraint_node_map;
  std::map<Variable, VariableNode*> variable_node_map;
  VisitMode visit_mode;
};

} //namespace simulator
} //namespace hydla 
