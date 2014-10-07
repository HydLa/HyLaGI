#pragma once

#include "Node.h"
#include "ModuleSetContainer.h"
#include "ModuleSet.h"
#include "DefaultTreeVisitor.h"
#include "ConstraintStore.h"

namespace hydla {
namespace simulator {

/**
 * Graph to represent relations of asks and variables
 */
class AskRelationGraph: public symbolic_expression::DefaultTreeVisitor{

public:
  typedef hierarchy::ModuleSet module_set_t;
  typedef hierarchy::ModuleSet::module_t module_t;
  typedef boost::shared_ptr<symbolic_expression::Ask> ask_t;
  typedef std::list<ask_t> asks_t;
  
  struct VariableNode;
  struct AskNode;
  typedef std::vector<VariableNode* > var_nodes_t;
  typedef std::vector<AskNode* >    ask_nodes_t;

  /**
   * Node for constraint
   */
  struct AskNode{
    ask_t ask;
    module_t module; /// module which the ask belongs to
    bool module_adopted,
         prev,
         entailed;
    AskNode* parent_ask;
    std::vector<VariableNode *> edges;
    AskNode(const ask_t &a, const module_t &mod);
    std::string get_name() const;
  };
  
  /**
   * Node for variable
   */
  struct VariableNode{
    std::string variable;
    std::vector<AskNode *> edges;
    VariableNode(const std::string &var):variable(var)
    {}
    std::string get_name() const;
  };

  AskRelationGraph(const module_set_t &mods);

  virtual ~AskRelationGraph();  


  /// Set adoptedness of module
  void set_adopted(const module_t &mod, bool adopted);

  /// Set adoptedness of modules
  void set_adopted(const module_set_t &ms, bool adopted);

  /// Set entailedness of the ask if it is prev_ask
  /// @return true if the ask is prev_ask
  bool set_entailed_if_prev(const ask_t &ask, bool entailed);

  void set_entailed(const ask_t &ask, bool entailed);

  // TODO: 親ガード条件が存在した場合にexpandedかどうかを考慮していない．

  /**
   * Print the structure in graphviz format.
   */
  void dump_graph(std::ostream &os) const;

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

private:
  typedef std::map<std::string, VariableNode*> variable_map_t;  
  
  void add(module_t &mod);

  VariableNode* add_variable_node(const std::string &);
  
  void visit(boost::shared_ptr<symbolic_expression::Ask> node);

  var_nodes_t variable_nodes;
  ask_nodes_t ask_nodes;
  AskNode* parent_ask;
  module_t current_module;

  std::map<module_t, std::vector<AskNode*> > module_ask_nodes_map;
  std::map<ask_t, AskNode*> ask_node_map;
  std::map<std::string, VariableNode*> variable_node_map;
};

} //namespace simulator
} //namespace hydla 
