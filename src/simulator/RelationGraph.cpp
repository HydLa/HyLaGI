#include "RelationGraph.h"
#include <iostream>
#include "VariableFinder.h"

namespace hydla {
namespace simulator {


RelationGraph::~RelationGraph()
{
  for(var_nodes_t::const_iterator it = var_list_.begin();
      it != var_list_.end();
      it++){
    delete *it;
  }

  for(mod_nodes_t::const_iterator it = mod_list_.begin();
      it != mod_list_.end();
      it++){
    delete *it;
  }
}

void RelationGraph::add(const relation_t &rel)
{
  for(relation_t::const_iterator it = rel.begin(); it != rel.end(); it++){
    ModuleNode*& mod = module_map_[it->first];
    if(mod == NULL){
      mod = new ModuleNode(it->first);
      mod_list_.push_back(mod);
    }
    VariableNode*& var = variable_map_[it->second];
    if(var == NULL){
      var = new VariableNode(it->second);
      var_list_.push_back(var);
    }
    mod->edges.push_back(var);
    var->edges.push_back(mod);
  }
  check_completed = false;
}

std::ostream& RelationGraph::dump_graph(std::ostream & os) const
{
  os << "graph g {\n";
  
  for(std::map<const module_t *, ModuleNode*>::const_iterator it = module_map_.begin(); it != module_map_.end(); ++it) {
    os << "  \"" << it->second->get_name() << "\" [shape = box]\n";
    for(var_nodes_t::const_iterator e_it = it->second->edges.begin(); e_it != it->second->edges.end(); ++e_it){
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

std::string RelationGraph::VariableNode::get_name() const
{
  return variable.get_string();
}

std::string RelationGraph::ModuleNode::get_name() const
{
  return module->first;
}

void RelationGraph::check_connected_components(){
  connected_components_.clear();
  
  for(module_map_t::const_iterator it = module_map_.begin();
      it != module_map_.end();
      it++){
    it->second->visited = !it->second->valid;
  }
  
  for(module_map_t::const_iterator it = module_map_.begin();
      it != module_map_.end();
      it++){
    module_set_t ms;
    if(!it->second->visited){
      visit_node(it->second, ms);
      connected_components_.push_back(ms);
    }
  }
  check_completed = true;
}

void RelationGraph::visit_node(ModuleNode* node, module_set_t &ms){
  if(!node->visited){
    node->visited = true;
    ms.add_module(*node->module);
    visit_edges(node, ms);
  }
}

void RelationGraph::visit_edges(ModuleNode* node, module_set_t &ms){
  for(var_nodes_t::const_iterator it = node->edges.begin();
      it != node->edges.end();
      it++){
      for(mod_nodes_t::const_iterator m_it = (*it)->edges.begin();
        m_it != (*it)->edges.end();
        m_it++){
      visit_node(*m_it, ms);
    }
  }
}

RelationGraph::RelationGraph(const module_set_t &ms, const variable_set_t& vs)
{
  relation_t relations;
  for(module_set_t::module_list_const_iterator it = ms.begin();it != ms.end(); it++){
    VariableFinder finder;
    finder.visit_node(it->second);
    VariableFinder::variable_set_t variables;
    variables = finder.get_all_variable_set();
    for(auto variable : variables)
    {
      relations.push_back(std::make_pair(&(*it), Variable(variable.first, variable.second)));
    }
  }
  add(relations);
  check_completed = false;
}

int RelationGraph::get_connected_count()
{
  if(!check_completed){
    check_connected_components();
  }
  return connected_components_.size();
}

void RelationGraph::set_valid(const module_t * mod, bool valid)
{
  module_map_[mod]->valid = valid;
  check_completed = false;
}

void RelationGraph::set_valid(const module_set_t* ms)
{
  for(unsigned int i=0; i < mod_list_.size(); i++){
    ModuleNode& node = *mod_list_[i];
    node.valid = (ms->find(*(node.module)) != ms->end());
  }
  check_completed = false;
}

boost::shared_ptr<RelationGraph::module_set_t> RelationGraph::get_component(unsigned int index)
{
  if(!check_completed){
    check_connected_components();
  }
  assert(index < connected_components_.size());
  return boost::shared_ptr<module_set_t>(new module_set_t(connected_components_[index]));
}


} //namespace simulator
} //namespace hydla 
