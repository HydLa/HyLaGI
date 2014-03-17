#include "RelationGraph.h"
#include <iostream>
#include "VariableFinder.h"

namespace hydla {
namespace simulator {

RelationGraph::RelationGraph(const relation_set_t& rel)
{
  add(rel);
  check_completed = false;
}

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

void RelationGraph::add(const relation_set_t &rel)
{
  for(relation_set_t::const_iterator it = rel.begin(); it != rel.end(); it++){
    ModuleRelationNode*& mod = module_map_[it->first];
    if(mod == NULL){
      mod = new ModuleRelationNode(it->first);
      mod_list_.push_back(mod);
    }
    VariableRelationNode*& var = variable_map_[it->second];
    if(var == NULL){
      var = new VariableRelationNode(it->second);
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
  
  for(std::map<const module_t *, ModuleRelationNode*>::const_iterator it = module_map_.begin(); it != module_map_.end(); ++it) {
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

std::string RelationGraph::VariableRelationNode::get_name() const
{
  std::string ret = variable.variable.get_string();
  if(variable.is_prev)ret += "-";
  return ret;
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

void RelationGraph::visit_node(ModuleRelationNode* node, module_set_t &ms){
  if(!node->visited){
    node->visited = true;
    ms.add_module(*node->module);
    visit_edges(node, ms);
  }
}

void RelationGraph::visit_edges(ModuleRelationNode* node, module_set_t &ms){
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

boost::shared_ptr<RelationGraph> RelationGraph::new_graph(const module_set_t &ms, const variable_set_t& vs, bool in_IP)
{
  relation_set_t relations;
  for(module_set_t::module_list_const_iterator it = ms.begin();it != ms.end(); it++){
    VariableFinder finder;
    finder.visit_node(it->second);
    VariableFinder::variable_set_t variables, prev_variables;
    if(in_IP)
    {
      variables = finder.get_all_variable_set();
    }
    else
    {
      variables = finder.get_variable_set();
      prev_variables = finder.get_prev_variable_set();
    }
    for(auto variable : variables)
    {
      variable_t var(DefaultVariable(variable.first, variable.second), false);
      relations.push_back(std::make_pair(&(*it), var));
    }
    
    for(auto variable : prev_variables)
    {
      relations.push_back(std::make_pair(&(*it), 
        variable_t(DefaultVariable(variable.first, variable.second), true)));
    }
  }
  return boost::shared_ptr<RelationGraph>(new RelationGraph(relations));
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
    ModuleRelationNode& node = *mod_list_[i];
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
