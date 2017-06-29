#pragma once

#include "Variable.h"
#include "Node.h"
#include "AtomicConstraint.h"
#include "ModuleSetContainer.h"
#include "TreeVisitorForAtomicConstraint.h"
#include "ConstraintStore.h"
#include "GuardNode.h"

namespace hydla {
namespace simulator {

typedef hierarchy::ModuleSet module_set_t;
typedef hierarchy::ModuleSet::module_t module_t;
typedef std::set<Variable, VariableComparator> variable_set_t;
typedef std::shared_ptr<symbolic_expression::Ask> ask_t;
typedef std::set<ask_t> asks_t;
  
struct VariableNode;
struct ConstraintNode;
struct TellNode;
struct AskNode;
struct AtomicGuardNode;
typedef std::list<VariableNode *> var_nodes_t;
typedef std::list<TellNode *> tell_nodes_t;
typedef std::list<AskNode *> ask_nodes_t;
typedef std::list<GuardNode *> guard_nodes_t;

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

struct ConstraintNode
{
  module_t module; /// module which the constraint belongs to
  bool collected;
  bool module_adopted; /// whether the module is in current module sets
  bool expanded;
  bool always;
  AskNode* parent;
  ConstraintNode(const module_t &mod)
    : module(mod), module_adopted(true), expanded(false) {}
  virtual ~ConstraintNode(){}
  std::vector<EdgeToVariable> edges;
  bool is_active() const;
  virtual std::string get_name() const = 0;
};
  
struct TellNode: public ConstraintNode
{
  constraint_t tell;
  TellNode(const constraint_t &cons, const module_t &mod)
    : ConstraintNode(mod), tell(cons)
  {}
  virtual ~TellNode(){}
  std::string get_name() const;
};

typedef enum {
  BOTH,
  TELL_ONLY,
  ASK_ONLY
} DumpMode;
  
struct AskNode: public ConstraintNode
{
  ask_t ask;
  bool prev, entailed;
  GuardNode *guard_node;
  std::list<AtomicGuardNode *> atomic_guard_list;
  ConstraintStore always_children;
  std::list<ConstraintNode*> children;
  AskNode(const ask_t &a, const module_t &mod, GuardNode *guard);
  virtual ~AskNode() {}
  std::string get_name() const;
};
  
/**
 * Node for variable
 */
struct VariableNode
{
  Variable variable;
  std::vector<EdgeToConstraint> edges;
  std::vector<AskNode *> ask_edges;
  VariableNode(Variable var):variable(var)
    {}
  std::string get_name() const;
};

/**
 * Bipartite graph to represent relations of constraints and variables
 */
class RelationGraph: public symbolic_expression::TreeVisitorForAtomicConstraint
{
public:
  RelationGraph(const module_set_t &mods);

  virtual ~RelationGraph();

  void add_guard(constraint_t &guard);

  /**
   * Print the structure in graphviz format.
   */
  void dump_graph(std::ostream &os, DumpMode mode = BOTH) const;

  /**
   * Print active nodes and edges in graphviz format.
   */
  void dump_active_graph(std::ostream &os, DumpMode mode = BOTH) const;

  /// Set adoptedness of module
  void set_adopted(const module_t &mod, bool adopted);

  /// Set adoptedness of modules
  void set_adopted(const module_set_t &ms, bool adopted);

  /// expand given atomic constraint
  void set_expanded_atomic(constraint_t cons, bool expanded);

  ///  expand given constraint recursively
  void set_expanded_recursive(constraint_t cons, bool expanded);

  void set_entailed(const ask_t &ask, bool entailed);
  asks_t set_entailed(const constraint_t &guard, bool entailed);

  bool get_entailed(const ask_t &ask)const;

  ConstraintStore get_always_list(const ask_t &ask)const;

  /**
   * Get active asks adjacent to given variable
   */
  asks_t get_adjacent_asks(const std::string &variable_name, bool ignore_prev_asks = false);

  /**
   * Get active asks adjacent to given variable
   */
  asks_t get_adjacent_asks2var_and_derivatives(const Variable &variable, bool ignore_prev_asks = false);

  /**
   * Get active guards adjacent to given variable
   */
  std::list<AtomicConstraint *> get_atomic_guards(const constraint_t &guard)const;

  /**
   * Get variables adjacent to given ask
   */
  std::set<std::string> get_adjacent_variables(const ask_t &asks);

  /**
   * Get all active asks
   */
  asks_t get_active_asks(bool ignore_prev_asks = false);

  /**
   * Get all active tells
   */ 
  ConstraintStore get_active_tells();
  
  /**
   * Get the number of connected component in the graph.
   */
  int get_connected_count();
  
  /**
   * Get constraints related to given variable
   * @parameter list of connectedconstraints for output
   * @parameter modules for output
   */
  void get_related_constraints(const Variable &variable, ConstraintStore &constraints);

  /**
   * Get constraints and modules related to given constraint
   * @parameter constraints for output
   */
  void get_related_constraints(constraint_t constraint, ConstraintStore &constraints);

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
   * Get variables related to constraint
   */
  variable_set_t get_related_variables(constraint_t constraint);

  /// Get modules related to constraint_store
  module_set_t get_related_modules(const ConstraintStore &constraint_store);

  /// Entail the guard if it is prev_guard
  /// @return true if the guard is prev_guard
  bool entail_if_prev(const ask_t &ask, bool entailed);

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

  asks_t get_all_asks();

  /**
   * Get all atomic guards
   */
  constraints_t get_all_guards();

  /**
   * if true, the left hand limit is regareded as a constant (ignoring its relation)
   */
  void set_ignore_prev(bool);

  /// return whether the variable is referred by some constraints or not
  bool referring(const Variable& variable);

  AskNode *get_ask_node(const ask_t &ask);

  GuardNode *get_guard_node(const constraint_t &guard);

private:
  typedef std::map<Variable, VariableNode*> variable_map_t;  
  
  void add(module_t &mod);

  void get_related_constraints_core(const Variable &var, ConstraintStore &constraints, module_set_t &module_set);
  
  VariableNode* add_variable_node(Variable &);

  bool to_be_considered(const AskNode* ask, bool ignore_prev)const;
  
  void check_connected_components();

  asks_t set_entailed(AtomicGuardNode *node, bool entailed, bool if_prev = false);

  void collect_node(TellNode* node, ConstraintStore *constraints, module_set_t *ms, variable_set_t *vars, bool visit_guards);
  void collect_node(VariableNode* node, ConstraintStore *constraint, module_set_t *ms, variable_set_t *vars, bool visit_guards);
  void collect_node(AskNode* node, ConstraintStore *constraints, module_set_t *ms, variable_set_t *vars, bool visit_guards);
  
  void initialize_node_collected();

  using TreeVisitorForAtomicConstraint::visit; // suppress warnings
  void visit(std::shared_ptr<symbolic_expression::Ask> ask);
  void visit(std::shared_ptr<symbolic_expression::LogicalOr> logical_or);
  void visit(std::shared_ptr<symbolic_expression::LogicalAnd> logical_and);
  void visit(std::shared_ptr<symbolic_expression::Not> not_expr);
  void visit(std::shared_ptr<symbolic_expression::Always> always); 
  void visit_atomic_constraint(std::shared_ptr<symbolic_expression::Node> binary);
  
  var_nodes_t variable_nodes;
  ask_nodes_t ask_nodes;
  tell_nodes_t tell_nodes;
  guard_nodes_t guard_nodes;

  AskNode* parent_ask;
  module_t current_module;

  typedef enum {
    EXPANDING,
    UNEXPANDING,
    ADDING,
    ADDING_ASK
  } VisitMode;

  std::map<module_t, tell_nodes_t>  module_tell_nodes_map;
  std::map<constraint_t, TellNode*> tell_node_map;
  std::map<constraint_t, ConstraintNode*> constraint_node_map;
  std::map<constraint_t, GuardNode*> guard_node_map;
  std::map<Variable, VariableNode*> variable_node_map;
  std::map<std::string, std::list<VariableNode*> > var_name_nodes_map;

  std::map<module_t, std::vector<AskNode*> > module_ask_nodes_map;
  std::map<ask_t, AskNode*> ask_node_map;

  ConstraintStore always_list; /// temporary variables to return always

  GuardNode *current_guard_node;
  std::list<AtomicGuardNode *> atomic_guard_list;

  VisitMode visit_mode;
  bool in_always;
  bool ignore_prev;
};

} // namespace simulator
} // namespace hydla 
