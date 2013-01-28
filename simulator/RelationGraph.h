#ifndef _INCLUDED_HYDLA_RELATION_GRAPH_H_
#define _INCLUDED_HYDLA_RELATION_GRAPH_H_

#include "Types.h"
#include "DefaultVariable.h"

namespace hydla {
namespace simulator {


class RelationGraph{

public:
  typedef hydla::ch::ModuleSet module_set_t;
  typedef hydla::ch::ModuleSet::module_t module_t;
  typedef std::list<DefaultVariable> variable_set_t;
  
  typedef struct Variable_{
    hydla::simulator::DefaultVariable* variable;
    bool is_prev;
    Variable_():variable(NULL), is_prev(false)
    {}
    Variable_(hydla::simulator::DefaultVariable* var, bool pr):variable(var), is_prev(pr)
    {}
    friend bool operator<(const Variable_& lhs, 
                          const Variable_& rhs){
      return (lhs.variable < rhs.variable) || (lhs.variable == rhs.variable && lhs.is_prev && !rhs.is_prev);
    }
  }variable_t;
  
  struct RelationNode;
  struct ModuleRelationNode;
  struct VariableRelationNode;
  typedef std::vector<ModuleRelationNode* > mod_nodes_t;
  typedef std::vector<VariableRelationNode* > var_nodes_t;

  struct RelationNode{
    bool valid, visited;
    virtual std::string get_name() const = 0;
    RelationNode():valid(true), visited(false){}
  };

  struct ModuleRelationNode:public RelationNode{
    const module_t* module;
    var_nodes_t edges;
    ModuleRelationNode():module(NULL)
    {}
    ModuleRelationNode(const module_t* mod):module(mod)
    {}
    virtual std::string get_name() const
    {return module->first;}
  };
  
  /**
   * VariableRelationNode has members 'valid' and 'visited' too, but they are currently ignored.
   */
  struct VariableRelationNode:public RelationNode{
    variable_t variable;
    mod_nodes_t edges;
    VariableRelationNode(variable_t var):variable(var)
    {}
    virtual std::string get_name() const;
  };
  
  static boost::shared_ptr<RelationGraph> new_graph(const module_set_t &ms, variable_set_t &vm, bool in_IP);

  virtual ~RelationGraph();
  

  /**
   * Print the structure of the graph in graphviz format.
   */
  std::ostream& dump_graph(std::ostream & os) const;
  
  /**
   * set a module valid or invalid
   */
  void set_valid(const module_t *mod, bool valid);
  
  /**
   * set modules in ms to argument valid
   * set modules not in ms negation of argument invalid
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
  typedef std::map<const module_t *, ModuleRelationNode*> module_map_t;
  typedef std::map<variable_t, VariableRelationNode*> variable_map_t;  
  typedef std::vector<std::pair<const module_t *, variable_t > > relation_set_t;

  RelationGraph(const relation_set_t& rel);
  
  /**
   * Add nodes and edges to the graph.
   */
  void add(const relation_set_t &rel);
  
  void check_connected_components();
  void visit_node(ModuleRelationNode* node, module_set_t &ms);
  void visit_edges(ModuleRelationNode* node, module_set_t &ms);
  
  mod_nodes_t mod_list_;
  var_nodes_t var_list_;
  
  std::vector<module_set_t> connected_components_;
  bool check_completed;
  std::map<const module_t *, ModuleRelationNode*> module_map_;
  std::map<variable_t, VariableRelationNode*> variable_map_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_RELATION_GRAPH_H_