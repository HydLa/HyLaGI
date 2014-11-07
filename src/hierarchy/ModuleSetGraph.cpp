#include "ModuleSetGraph.h"

#include <assert.h>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace boost;

namespace hydla{
namespace hierarchy {

ModuleSetGraph::ModuleSetGraph()
{}

ModuleSetGraph::ModuleSetGraph(ModuleSet m):ModuleSetContainer(m)
{}

ModuleSetGraph::ModuleSetGraph(ModuleSet m, std::vector<symbolic_expression::node_sptr> cl) :
  ModuleSetContainer(m)
{}

ModuleSetGraph::~ModuleSetGraph()
{}

void  ModuleSetGraph::add_parallel(ModuleSetGraph& parallel_module_set_graph)
{

  // parallel(X, Y) = X ∪ Y ∪ {x ∪ y | x∈X, y∈Y}

  // X
  module_set_set_t new_module_set_list(full_module_set_set_);
    
  // Y
  for(auto p_it : parallel_module_set_graph.full_module_set_set_){
    new_module_set_list.insert(p_it);
  }

  // {x ∪ y | x∈X, y∈Y}
  for(auto p_it : parallel_module_set_graph.full_module_set_set_) {
    for(auto this_it : full_module_set_set_) {
      ModuleSet ms(this_it, p_it);
      new_module_set_list.insert(ms);
    }
  }

  full_module_set_set_.swap(new_module_set_list);
  maximal_module_set_ = *full_module_set_set_.rbegin();
  build_edges();
}

void  ModuleSetGraph::add_required_parallel(ModuleSetGraph& parallel_module_set_graph)
{

  // parallel(X, Y) = {x ∪ y | x∈X, y∈Y}
  // 空のモジュール集合の集合を用意
  module_set_set_t new_module_set_list;

  // {x ∪ y | x∈X, y∈Y}
  for(auto p_it : parallel_module_set_graph.full_module_set_set_) {
    for(auto this_it : full_module_set_set_) {
      ModuleSet ms(this_it,  p_it);
      new_module_set_list.insert(ms);
    }
  }

  full_module_set_set_.swap(new_module_set_list);
  maximal_module_set_ = *full_module_set_set_.rbegin();
  build_edges();
}

void  ModuleSetGraph::add_weak(ModuleSetGraph& weak_module_set_graph)
{
  // ordered(X, Y) = Y ∪ {x ∪ y | x∈X, y∈Y}
      
  // Y
  module_set_set_t new_module_set_list(full_module_set_set_);

  ModuleSet y = *full_module_set_set_.rbegin();
  // {x ∪ y | x∈X, y∈Y}
  for(auto p_it : weak_module_set_graph.full_module_set_set_) {
    ModuleSet ms(y,p_it);
    new_module_set_list.insert(ms);
  }

  full_module_set_set_.swap(new_module_set_list);
  maximal_module_set_ = *full_module_set_set_.rbegin();
  build_edges();
}

namespace {
struct ModSizePred {
  ModSizePred(size_t size) :
    size_(size)
  {}

  bool operator()(const hydla::hierarchy::ModuleSet& n) const {
    return n.size() == size_;
  }

  size_t size_;
};
}

void ModuleSetGraph::build_edges()
{
  edges_.clear();

  if(full_module_set_set_.size() == 0) return;

  module_set_set_t::reverse_iterator node_begin = full_module_set_set_.rbegin();
  module_set_set_t::reverse_iterator node_end   = full_module_set_set_.rend();

  int modsize = (int)(*node_begin).size();

  module_set_set_t::reverse_iterator super_it = node_begin;
  module_set_set_t::reverse_iterator super_end = 
    std::find_if(node_begin, node_end, ModSizePred(--modsize));
  
  while(super_it!=node_end) {
    module_set_set_t::reverse_iterator subset_it  = super_end;
    module_set_set_t::reverse_iterator subset_end = 
      std::find_if(subset_it, node_end, ModSizePred(--modsize));
    
    for(; super_it!=super_end; ++super_it) {
      for(module_set_set_t::reverse_iterator tmp_subset_it = subset_it;
          tmp_subset_it!=subset_end; 
          ++tmp_subset_it) {
        if((*super_it).including(*tmp_subset_it)) {
          edges_.insert(
            edges_t::value_type((*super_it), (*tmp_subset_it)));
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
  module_set_set_t::const_iterator it  = full_module_set_set_.begin();
  module_set_set_t::const_iterator end = full_module_set_set_.end();

  s << "{";
  if(it!=end) s << (*it++).get_name();
  while(it!=end) {
    s << ", " << (*it++).get_name();
  }
  s << "}";

  return s;
}

std::ostream& ModuleSetGraph::dump_node_trees(std::ostream& s) const
{
  module_set_set_t::const_iterator it  = full_module_set_set_.begin();
  module_set_set_t::const_iterator end = full_module_set_set_.end();

  s << "{";
  if(it!=end) s << *(it++);
  while(it!=end) {
    s << ",\n" << *(it++);
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
    s << it->first.get_name() 
      << "->" 
      << it->second.get_name() 
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
      << it->first.get_name() 
      << "\" -> \"" 
      << it->second.get_name() 
      << "\";\n";
  }

  s << "}" << std::endl;

  return s;
}

void ModuleSetGraph::reset(const module_set_set_t &mss){
  ms_to_visit_ = mss;
  visited_module_sets_.clear();
}

} // namespace hierarchy
} // namespace hydla
