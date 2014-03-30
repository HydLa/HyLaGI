#ifndef _INCLUDED_HYDLA_RELATION_GRAPH_H_
#define _INCLUDED_HYDLA_RELATION_GRAPH_H_

#include "Variable.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace simulator {


/**
 * Graph to represent relation of constraint modules and variables.
 */
class RelationGraph{

public:
  typedef hydla::ch::ModuleSet module_set_t;
  typedef hydla::ch::ModuleSet::module_t module_t;
  typedef std::set<Variable, VariableComparator> variable_set_t;  
  
  struct ModuleNode;
  struct VariableNode;
  typedef std::vector<ModuleNode* > mod_nodes_t;
  typedef std::vector<VariableNode* > var_nodes_t;

  /**
   * Node for constraint modules
   */
  struct ModuleNode{
    const module_t* module;
    bool valid, visited;
    var_nodes_t edges;
    ModuleNode():module(NULL)
    {}
    ModuleNode(const module_t* mod):module(mod)
    {}
    std::string get_name() const;
  };
  
  /**
   * Node for variables.
   */
  struct VariableNode{
    Variable variable;
    mod_nodes_t edges;
    VariableNode(Variable var):variable(var)
    {}
    std::string get_name() const;
  };
  
  RelationGraph(const module_set_t &ms, const variable_set_t &vm);

  virtual ~RelationGraph();
  

  /**
   * Print the structure in graphviz format.
   */
  std::ostream& dump_graph(std::ostream & os) const;
  
  /**
   * Set a module valid or invalid
   */
  void set_valid(const module_t *mod, bool valid);
  
  /**
   * Set modules in ms valid and modules not in ms invalid
   */
  void set_valid(const module_set_t *ms);
  
  /**
   * Get the number of connected component in the graph.
   */
  int get_connected_count();
  
  /**
   * Get the module_set corresponding to the connected component specified by index.
   */
  boost::shared_ptr<module_set_t> get_component(unsigned int index);  

private:
  typedef std::map<const module_t *, ModuleNode*> module_map_t;
  typedef std::map<Variable, VariableNode*> variable_map_t;  
  typedef std::vector<std::pair<const module_t *, Variable > > relation_t;
  
  /**
   * Add nodes and edges to the graph.
   */
  void add(const relation_t &rel);
  
  void check_connected_components();
  void visit_node(ModuleNode* node, module_set_t &ms);
  void visit_edges(ModuleNode* node, module_set_t &ms);
  
  mod_nodes_t mod_list_;
  var_nodes_t var_list_;
  
  std::vector<module_set_t> connected_components_;
  bool check_completed;     /// false if recheck is necessary
  std::map<const module_t *, ModuleNode*> module_map_;
  std::map<Variable, VariableNode*> variable_map_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_RELATION_GRAPH_H_
