#include "ModuleSetGraph.h"

#include <assert.h>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace boost;

namespace hydla{
namespace ch {

ModuleSetGraph::ModuleSetGraph()
{}

ModuleSetGraph::ModuleSetGraph(module_set_sptr m)
{
  Node n;
  n.mod     = m;
  n.visited = false;
  nodes_.push_back(n);
}

ModuleSetGraph::~ModuleSetGraph()
{}

void  ModuleSetGraph::add_parallel(ModuleSetGraph& parallel_module_set_graph)
{

  // parallel(X, Y) = X ∪ Y ∪ {x ∪ y | x∈X, y∈Y}

  nodes_t::const_iterator p_it = 
    parallel_module_set_graph.nodes_.begin();
  nodes_t::const_iterator p_end = 
    parallel_module_set_graph.nodes_.end();

  // X
  nodes_t new_nodes(nodes_);
    
  // Y
  new_nodes.insert(new_nodes.end(), p_it, p_end);

  // {x ∪ y | x∈X, y∈Y}
  for(; p_it!=p_end; ++p_it) {
    nodes_t::iterator this_it =  nodes_.begin();
    nodes_t::iterator this_end = nodes_.end();
    
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(*(this_it->mod),  *(p_it->mod)));
      Node n;
      n.mod     = ms;
      n.visited = false;
      new_nodes.push_back(n);
    }
  }

  sort(new_nodes.begin(), new_nodes.end(), NodeComp());
  nodes_.swap(new_nodes);
  build_edges();
}

void  ModuleSetGraph::add_required_parallel(ModuleSetGraph& parallel_module_set_graph)
{

  // parallel(X, Y) = {x ∪ y | x∈X, y∈Y}

  nodes_t::const_iterator p_it = 
    parallel_module_set_graph.nodes_.begin();
  nodes_t::const_iterator p_end = 
    parallel_module_set_graph.nodes_.end();

  // 空のモジュール集合の集合を用意
  nodes_t new_nodes(nodes_);
  new_nodes.clear();

  // {x ∪ y | x∈X, y∈Y}
  for(; p_it!=p_end; ++p_it) {
    nodes_t::iterator this_it =  nodes_.begin();
    nodes_t::iterator this_end = nodes_.end();
    
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(*(this_it->mod),  *(p_it->mod)));
      Node n;
      n.mod     = ms;
      n.visited = false;
      new_nodes.push_back(n);
    }
  }

  sort(new_nodes.begin(), new_nodes.end(), NodeComp());
  nodes_.swap(new_nodes);
  build_edges();
}

void  ModuleSetGraph::add_weak(ModuleSetGraph& weak_module_set_graph)
{
  // ordered(X, Y) = Y ∪ {x ∪ y | x∈X, y∈Y}
      
  // Y
  nodes_t new_nodes(nodes_);

  // {x ∪ y | x∈X, y∈Y}
  nodes_t::const_iterator p_it = 
    weak_module_set_graph.nodes_.begin();
  nodes_t::const_iterator p_end = 
    weak_module_set_graph.nodes_.end();

  for(; p_it!=p_end; ++p_it) {
    nodes_t::iterator this_it =  nodes_.begin();
    nodes_t::iterator this_end = nodes_.end();
  
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(*(this_it->mod),  *(p_it->mod)));
      Node n;
      n.mod     = ms;
      n.visited = false;
      new_nodes.push_back(n);
    }
  }

  sort(new_nodes.begin(), new_nodes.end(), NodeComp());
  nodes_.swap(new_nodes);
  build_edges();
}

namespace {
struct ModSizePred {
  ModSizePred(size_t size) :
    size_(size)
  {}

  bool operator()(const ModuleSetGraph::Node& n) const {
    return n.mod->size() == size_;
  }

  size_t size_;
};
}

void ModuleSetGraph::build_edges()
{
  edges_.clear();

  if(nodes_.size() == 0) return;

  nodes_t::iterator node_begin = nodes_.begin();
  nodes_t::iterator node_end   = nodes_.end();

  int modsize = (int)node_begin->mod->size();

  nodes_t::iterator super_it = node_begin;
  nodes_t::iterator super_end = 
    std::find_if(node_begin, node_end, ModSizePred(--modsize));
  
  while(super_it!=node_end) {
    nodes_t::iterator subset_it  = super_end;
    nodes_t::iterator subset_end = 
      std::find_if(subset_it, node_end, ModSizePred(--modsize));
    
    for(; super_it!=super_end; ++super_it) {
      for(nodes_t::iterator tmp_subset_it = subset_it;
          tmp_subset_it!=subset_end; 
          ++tmp_subset_it) {
        if(super_it->mod->is_super_set(*tmp_subset_it->mod)) {
          edges_.insert(
            edges_t::value_type(&(*super_it), &(*tmp_subset_it)));
        }
      }
    } 
    
    super_it  = subset_it;
    super_end = subset_end;
  }
}

std::ostream& ModuleSetGraph::dump(std::ostream& s) const
{
  dump_node_names(s);
  s << "\n";
  dump_node_trees(s);
  s << "\n";
  dump_edges(s);

  return s;
}

std::ostream& ModuleSetGraph::dump_node_names(std::ostream& s) const
{
  nodes_t::const_iterator it  = nodes_.begin();
  nodes_t::const_iterator end = nodes_.end();

  s << "{";
  if(it!=end) s << (it++)->mod->get_name();
  while(it!=end) {
    s << ", " << (it++)->mod->get_name();
  }
  s << "}";

  return s;
}

std::ostream& ModuleSetGraph::dump_node_trees(std::ostream& s) const
{
  nodes_t::const_iterator it  = nodes_.begin();
  nodes_t::const_iterator end = nodes_.end();

  s << "{";
  if(it!=end) s << *(it++)->mod;
  while(it!=end) {
    s << ",\n" << *(it++)->mod;
  }
  s << "}";

  return s;
}

std::ostream& ModuleSetGraph::dump_edges(std::ostream& s) const
{
  edges_t::left_const_iterator it  = edges_.left.begin();
  edges_t::left_const_iterator end = edges_.left.end();

    
  s << "{";
  for(; it!=end; ++it) {
    s << it->first->mod->get_name() 
      << "->" 
      << it->second->mod->get_name() 
      << "\n";
  }
  s << "}";

  return s;
}


std::ostream& ModuleSetGraph::dump_graphviz(std::ostream& s) const
{
  s << "digraph g {\n"
    << "  edge [dir=back];\n";
  
  edges_t::left_const_iterator it  = edges_.left.begin();
  edges_t::left_const_iterator end = edges_.left.end();
  for(; it!=end; ++it) {
    s << "  \"" 
      << it->first->mod->get_name() 
      << "\" -> \"" 
      << it->second->mod->get_name() 
      << "\";\n";
  }

  s << "}" << std::endl;

  return s;
}

bool ModuleSetGraph::dispatch(
  boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
  int threads)
{  
  nodes_t::iterator it  = nodes_.begin();
  nodes_t::iterator end = nodes_.end();

  // 全ノードを未探索状態にする
  for(; it!=end; ++it) {
    it->visited = false;
  }

  // 探索
  for(it=nodes_.begin(); it!=end; ++it) {
    if(!it->visited && callback_func(it->mod)) {
      // 包含されるモジュール集合は探索済みにする
      mark_visited_flag(&*it);
    }
  }
  return false;
}

void ModuleSetGraph::mark_visited_flag(Node* node)
{
  node->visited = true;

  std::pair<edges_t::map_by<superset>::const_iterator, 
            edges_t::map_by<superset>::const_iterator>
    superset_range = edges_.by<superset>().equal_range(node);

  for(; superset_range.first!=superset_range.second; ++superset_range.first) {
    mark_visited_flag(superset_range.first->get<subset>());
  }
}

} // namespace ch
} // namespace hydla
