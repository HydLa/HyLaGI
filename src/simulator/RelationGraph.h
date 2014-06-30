#pragma once

#include "Variable.h"
#include "IncrementalModuleSet.h"
#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "ConstraintStore.h"

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
  
  struct VariableNode;
  struct ConstraintNode;
  typedef std::vector<VariableNode* > var_nodes_t;
  typedef std::vector<ConstraintNode* > constraint_nodes_t;

  struct EdgeToVariable
  {
    VariableNode *variable_node;
    bool ref_prev;               // indicates whether the reference is only for left hand limit or not
    EdgeToVariable(VariableNode *, bool);
  };

  struct EdgeToConstraint
  {
    ConstraintNode *constraint_node;
    bool ref_prev;               // indicates whether the reference is only for left hand limit or not
    EdgeToConstraint(ConstraintNode *, bool);
  };

  /**
   * Node for constraint
   */
  struct ConstraintNode{
    constraint_t constraint;
    module_t module; /// module which the constraint belongs to
    bool visited;
    bool module_adopted; /// whether the module is in ms
    bool expanded; /// whether the guard of the constraint is entailed
    std::vector<EdgeToVariable> edges;
    ConstraintNode(const constraint_t &cons, const module_t &mod):constraint(cons), module(mod), module_adopted(true), expanded(true)
    {}
    std::string get_name() const;
    bool active() const;
  };
  
  /**
   * Node for variable
   */
  struct VariableNode{
    Variable variable;
    std::vector<EdgeToConstraint> edges;
    VariableNode(Variable var):variable(var)
    {}
    std::string get_name() const;
  };

  
  RelationGraph(const module_set_t &mods);

  virtual ~RelationGraph();  

  /**
   * Print the structure in graphviz format.
   */
  void dump_graph(std::ostream &os) const;

  /**
   * Print active nodes and edges in graphviz format.
   */
  void dump_active_graph(std::ostream &os) const;
  
  /**
   * Set a moduld adopted or not
   */
  void set_adopted(const module_t &mod, bool adopted);
  
  /**
   * Set modules in ms adopted and modules not in ms not adopted
   */
  void set_adopted(const module_set_t &ms);

  /**
   * Set the constraint expanded or not
   */ 
  void set_expanded(constraint_t cons, bool expanded);

  /**
   * Set all constraints expanded or not
   */
  void set_expanded_all(bool expanded);

  /**
   * Set which constraints is changing.
   */
  void set_changing_constraints(const ConstraintStore& constraints);

  /**
   * Get connected constraints which are changed
   * @parameter constraints_vector for output
   */
  ConstraintStore get_changing_constraints();
  
  /**
   * Get the number of connected component in the graph.
   */
  int get_connected_count();

  /**
   * Get constraints and modules related to given variable
   * @parameter constraints for output
   * @parameter modules for output
   */
  void get_related_constraints(const Variable &variable, ConstraintStore &constraints,
                               module_set_t &modules);

  /**
   * Get constraints and modules related to given variable
   * @parameter constraints for output
   * @parameter modules for output
   */
  void get_related_constraints(constraint_t constraint, ConstraintStore &constraints,
                               module_set_t &modules);


  /**
   * Get variabes included by connected component specified by index
   */
  variable_set_t get_variables(unsigned int index);


  /**
   * Get constraints corresponding to the connected component specified by index.
   */
  ConstraintStore get_constraints(unsigned int index);

  /**
   * Get a ModuleSet included by the connected component specified by index.
   */ 
  module_set_t get_modules(unsigned int index);

  /**
   * Get constraints which related to given variables
   */
  ConstraintStore get_constraints(const std::vector<Variable>& variables);

  /**
   * Get all valid (adopted and expanded) constraints
   */
  ConstraintStore get_constraints();

  /**
   * Get all expanded constraints
   */
  ConstraintStore get_expanded_constraints();

  /**
   * Get all adopted constraints
   */
  ConstraintStore get_adopted_constraints();

  /**
   * if true, the left hand limit is regareded as a constant (ignoring its relation)
   */
  void set_ignore_prev(bool);

  /**
   * return whether consistency of constraint_store has to be checked
   */
  bool is_changing(const ConstraintStore constraint_store);

  bool is_changing(const constraint_t constraint);

  /**
   * return whether variable is related to changing connected constraints
   */
  bool is_changing(const Variable& variable);

  /**
   * clear changing_connected_constraints_index_set
   */
  void clear_changing();

private:
  typedef std::map<Variable, VariableNode*> variable_map_t;  
  typedef std::vector<std::pair<constraint_t, Variable > > relation_t;
  
  void add(module_t &mod);

  VariableNode* add_variable_node(Variable &);
  
  void check_connected_components();
  void visit_node(ConstraintNode* node, ConstraintStore &constraints, module_set_t &ms, variable_set_t &vars);
  void visit_node(VariableNode* node, ConstraintStore &constraint, module_set_t &ms, variable_set_t &vars);
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

  std::vector<ConstraintStore> connected_constraints_vector;
  std::vector<module_set_t> connected_modules_vector;
  std::vector<variable_set_t>     connected_variables_vector;
  ConstraintStore changing_constraints;
  bool up_to_date;     /// if false, recalculation of connected components is necessary
  std::map<module_t, constraint_nodes_t>  module_constraint_nodes_map;
  std::map<constraint_t, ConstraintNode*> constraint_node_map;
  std::map<Variable, VariableNode*> variable_node_map;
  VisitMode visit_mode;
  bool ignore_prev;
};

} //namespace simulator
} //namespace hydla 
