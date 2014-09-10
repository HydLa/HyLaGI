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
  typedef std::vector<ask_t> asks_t;
  
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
    bool module_adopted;
    std::vector<VariableNode *> edges;
    AskNode(const ask_t &a, const module_t &mod):ask(a), module(mod),module_adopted(true){}
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

  // TODO: 親ガード条件が存在した場合にexpandedかどうかを考慮していない．

  /**
   * Print the structure in graphviz format.
   */
  void dump_graph(std::ostream &os) const;

  /**
   * Get asks adjacent to given variable
   * @parameter asks for output
   */
  void get_adjacent_asks(const std::string &variable, asks_t &asks);

  /**
   * Get variables adjacent to given ask
   * @parameter constraints for output
   */
  void get_adjacent_variables(const ask_t &asks, std::set<std::string> &variables);

  /**
   * Get all asks
   */
  asks_t get_asks();

private:
  typedef std::map<std::string, VariableNode*> variable_map_t;  
  
  void add(module_t &mod);

  VariableNode* add_variable_node(const std::string &);
  
  void visit(boost::shared_ptr<symbolic_expression::Ask> node);

  var_nodes_t variable_nodes;
  ask_nodes_t ask_nodes;
  module_t current_module;

  std::map<module_t, std::vector<AskNode*> > module_ask_nodes_map;
  std::map<ask_t, AskNode*> ask_node_map;
  std::map<std::string, VariableNode*> variable_node_map;
};

} //namespace simulator
} //namespace hydla 
