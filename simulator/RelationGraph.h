#ifndef _INCLUDED_HYDLA_RELATION_GRAPH_H_
#define _INCLUDED_HYDLA_RELATION_GRAPH_H_

#include "Types.h"
#include "DefaultVariable.h"

namespace hydla {
namespace simulator {


class RelationGraph{

public:
  typedef hydla::ch::ModuleSet::module_t module_t;
  
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
  typedef std::vector<RelationNode* > edges_t;
  typedef std::vector<RelationNode* > nodes_t;

  struct RelationNode{
    edges_t edges;
    virtual std::string get_name() const = 0;
  };

  struct ModuleRelationNode:public RelationNode{
    const module_t* module;
    ModuleRelationNode():module(NULL)
    {}
    ModuleRelationNode(const module_t* mod):module(mod)
    {}
    virtual std::string get_name() const
    {return module->first;}
  };

  struct VariableRelationNode:public RelationNode{
    variable_t variable;
    VariableRelationNode(variable_t var):variable(var)
    {}
    virtual std::string get_name() const;
  };
  
  typedef std::vector<std::pair<const module_t *, variable_t > > relation_set_t;

  RelationGraph(const relation_set_t& rel);

  virtual ~RelationGraph();
  
  /**
   * Add nodes and edges to the graph.
   */
  void add(const relation_set_t &rel);

  /**
   * Print the structure of the graph in graphviz format.
   */
  std::ostream& dump_graph(std::ostream & os) const;
  
  /**
   * Get the number of connected component in the graph.
   */
  int get_connected_count() const;
  
  /**
   * Get the module_set corresponding to the connected component specified by index.
   */
  module_set_sptr get_component(int index) const;  

private:
  nodes_t node_list_;
  std::map<const module_t *, ModuleRelationNode*> module_map_;
  std::map<variable_t, VariableRelationNode*> variable_map_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_RELATION_GRAPH_H_