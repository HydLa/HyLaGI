#include "RelationGraph.h"

namespace hydla {
namespace simulator {

RelationGraph::RelationGraph(const relation_set_t& rel)
{
  add(rel);
}

RelationGraph::~RelationGraph()
{
}

void RelationGraph::add(const relation_set_t &rel)
{
  for(relation_set_t::const_iterator it = rel.begin(); it != rel.end(); it++){
    ModuleRelationNode*& mod = module_map_[it->first];
    if(mod == NULL){
      mod = new ModuleRelationNode(it->first);
      node_list_.push_back(mod);
    }
    VariableRelationNode*& var = variable_map_[it->second];
    if(var == NULL){
      var = new VariableRelationNode(it->second);
      node_list_.push_back(var);
    }
    mod->edges.push_back(var);
    var->edges.push_back(mod);
  }
}

std::ostream& RelationGraph::dump_graph(std::ostream & os) const
{
  os << "graph g {\n";
  
  for(std::map<const module_t *, ModuleRelationNode*>::const_iterator it = module_map_.begin(); it != module_map_.end(); ++it) {
    os << "  \"" << it->second->get_name() << "\" [shape = box]\n";
    for(edges_t::const_iterator e_it = it->second->edges.begin(); e_it != it->second->edges.end(); ++e_it){
      os << "  \"" 
        << it->second->get_name() 
        << "\" -- \"" 
        << (*e_it)->get_name() 
        << "\";\n";
    }
  }
  os << "}" << std::endl;

  return os;
}

std::string RelationGraph::VariableRelationNode::get_name() const
{
  std::string ret = variable.variable->get_string();
  if(variable.is_prev)ret += "-";
  return ret;
}
  

int RelationGraph::get_connected_count() const
{
  return 0;
}

module_set_sptr RelationGraph::get_component(int index) const
{
  return module_set_sptr();
}


} //namespace simulator
} //namespace hydla 