#pragma once

#include "Variable.h"
#include "Node.h"
#include "ModuleSetContainer.h"
#include "TreeVisitorForAtomicConstraint.h"
#include "ConstraintStore.h"

namespace hydla {
namespace simulator {

/**
 * Graph to represent relations of constraints and variables
 */
class RelationGraph: public symbolic_expression::TreeVisitorForAtomicConstraint{

// TODO: 数式のnodeとグラフ上のnodeが混じって大変なことになっているので分離したい

public:
  typedef hierarchy::ModuleSet module_set_t;
  typedef hierarchy::ModuleSet::module_t module_t;
  typedef std::set<Variable, VariableComparator> variable_set_t;
  typedef boost::shared_ptr<symbolic_expression::Ask> ask_t;
  typedef std::list<ask_t> asks_t;
  
  struct VariableNode;
  struct ConstraintNode;
  struct TellNode;
  struct AskNode;
  typedef std::list<VariableNode* > var_nodes_t;
  typedef std::list<TellNode* > tell_nodes_t;
  typedef std::list<AskNode* > ask_nodes_t;

  struct EdgeToVariable
  {
    VariableNode *variable_node;
    bool ref_prev;               // indicates whether the reference is only for left hand limit or not
    EdgeToVariable(VariableNode *, bool);
  };

  struct EdgeToConstraint
  {
    TellNode *tell_node;
    bool ref_prev;               // indicates whether the reference is only for left hand limit or not
    EdgeToConstraint(TellNode *, bool);
  };

  struct ConstraintNode{
    module_t module; /// module which the constraint belongs to
    bool visited;
    bool module_adopted; /// whether the module is in ms
    bool expanded;
    bool always;
    AskNode* parent;
    ConstraintNode(const module_t &mod):module(mod), module_adopted(true), expanded(false){}
    std::vector<EdgeToVariable> edges;
    bool is_active() const;
  };
  
  struct TellNode: public ConstraintNode{
    constraint_t constraint;
    TellNode(const constraint_t &cons, const module_t &mod):ConstraintNode(mod), constraint(cons)
    {}
    std::string get_name() const;
  };

  
  struct AskNode: public ConstraintNode{
    ask_t ask;
    bool prev, entailed;
    std::list<ConstraintNode*> children;
    AskNode(const ask_t &a, const module_t &mod);
    std::string get_name() const;
  };
  
  
  /**
   * Node for variable
   */
  struct VariableNode{
    Variable variable;
    std::vector<EdgeToConstraint> edges;
    std::vector<AskNode *> ask_edges;
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

  /// Set adoptedness of module
  void set_adopted(const module_t &mod, bool adopted);

  /// Set adoptedness of modules
  void set_adopted(const module_set_t &ms, bool adopted);

  /// expand given atomic constraint
  void set_expanded_atomic(constraint_t cons, bool expanded);

  ///  expand given constraint recursively
  void set_expanded_recursive(constraint_t cons, bool expanded);

  std::list<constraint_t> set_entailed(const ask_t &ask, bool entailed);

  bool get_entailed(const ask_t &ask)const;

    /**
   * Get active asks adjacent to given variable
   * @parameter asks for output
   */
  asks_t get_adjacent_asks(const std::string &variable, bool ignore_prev_asks = false);

  /**
   * Get variables adjacent to given ask
   * @parameter constraints for output
   */
  std::set<std::string> get_adjacent_variables(const ask_t &asks);

  /**
   * Get all active asks
   */
  asks_t get_active_asks(bool ignore_prev_asks = false);

  bool active(const AskNode* ask, bool ignore_prev)const;

  
  /**
   * Get the number of connected component in the graph.
   */
  int get_connected_count();

  
  /**
   * Get constraints and modules related to given variable
   * @parameter list of connectedconstraints for output
   * @parameter modules for output
   */
  void get_related_constraints(const Variable &variable, ConstraintStore &constraints,
                               module_set_t &modules);

  /**
   * Get constraints and modules related to given constraint
   * @parameter constraints for output
   * @parameter modules for output
   */
  void get_related_constraints(constraint_t constraint, ConstraintStore &constraints,
                               module_set_t &modules);



  /**
   * Get vector of constraints and modules related to given constraints
   * @parameter vector of connected constraints for output
   * @parameter vector of modules for output
   */
  void get_related_constraints_vector(const ConstraintStore &constraints, std::vector<ConstraintStore> &constraints_vector,
                               std::vector<module_set_t> &modules_vector);

  
  /**
   * Get vector of constraints and modules related to given variables or constraints
   * @parameter list of connectedconstraints for output
   * @parameter modules for output
   */
  void get_related_constraints_vector(const ConstraintStore &constraints, const variable_set_t &variables, std::vector<ConstraintStore> &constraints_vector, std::vector<module_set_t> &modules_vector);

  /**
   * Get variabes included by connected component specified by index
   */
  variable_set_t get_variables(unsigned int index);

  
  /// Entail the ask if it is prev_ask
  /// @param always_list always nodes refered as children of the ask
  /// @return true if the ask is prev_ask
  bool entail_if_prev(const ask_t &ask, bool entailed, std::list<constraint_t> &always_list);

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

  /// return whether the variable is referred by some constraints or not
  bool referring(const Variable& variable);

private:
  typedef std::map<Variable, VariableNode*> variable_map_t;  
  
  void add(module_t &mod);

  void get_related_constraints_core(const Variable &var, ConstraintStore &constraints, module_set_t &module_set);
  
  VariableNode* add_variable_node(Variable &);
  
  void check_connected_components();
  void visit_node(TellNode* node, ConstraintStore &constraints, module_set_t &ms, variable_set_t &vars);
  void visit_node(VariableNode* node, ConstraintStore &constraint, module_set_t &ms, variable_set_t &vars);
  void initialize_node_visited();
  using TreeVisitorForAtomicConstraint::visit; // suppress warnings
  void visit(boost::shared_ptr<symbolic_expression::Ask> ask);
  void visit(boost::shared_ptr<symbolic_expression::Always> always); 
  void visit_atomic_constraint(boost::shared_ptr<symbolic_expression::BinaryNode> binary_node);

  var_nodes_t variable_nodes;
  ask_nodes_t ask_nodes;
  tell_nodes_t tell_nodes;

  AskNode* parent_ask;
  module_t current_module;

  typedef enum{
    EXPANDING,
    UNEXPANDING,
    ADDING
  }VisitMode;

  std::vector<ConstraintStore>    connected_constraints_vector;
  std::vector<module_set_t>       connected_modules_vector;
  std::vector<variable_set_t>     connected_variables_vector;
  variable_set_t referred_variables;
  bool up_to_date;     /// if false, recalculation of connected components is necessary
  std::map<module_t, tell_nodes_t>  module_tell_nodes_map;
  std::map<constraint_t, TellNode*> tell_node_map;
  std::map<Variable, VariableNode*> variable_node_map;

  std::map<module_t, std::vector<AskNode*> > module_ask_nodes_map;
  std::map<constraint_t, AskNode*> ask_node_map;

  std::list<constraint_t> always_list, nonalways_list; /// temporary variables to return always and nonalways

  VisitMode visit_mode;
  bool in_always;
  bool ignore_prev;
};

} //namespace simulator
} //namespace hydla 
