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

ModuleSetGraph::ModuleSetGraph(module_set_sptr m):ModuleSetContainer(m)
{}

ModuleSetGraph::~ModuleSetGraph()
{}

void  ModuleSetGraph::add_parallel(ModuleSetGraph& parallel_module_set_graph)
{

  // parallel(X, Y) = X ∪ Y ∪ {x ∪ y | x∈X, y∈Y}

  module_set_list_t::const_iterator p_it = 
    parallel_module_set_graph.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    parallel_module_set_graph.module_set_list_.end();

  // X
  module_set_list_t new_module_set_list(module_set_list_);
    
  // Y
  new_module_set_list.insert(new_module_set_list.end(), p_it, p_end);

  // {x ∪ y | x∈X, y∈Y}
  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
    
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_module_set_list.push_back(ms);
    }
  }

  sort(new_module_set_list.begin(), new_module_set_list.end(), ModuleSetComparator());
  module_set_list_.swap(new_module_set_list);
  build_edges();
}

void  ModuleSetGraph::add_required_parallel(ModuleSetGraph& parallel_module_set_graph)
{

  // parallel(X, Y) = {x ∪ y | x∈X, y∈Y}

  module_set_list_t::const_iterator p_it = 
    parallel_module_set_graph.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    parallel_module_set_graph.module_set_list_.end();

  // 空のモジュール集合の集合を用意
  module_set_list_t new_module_set_list(module_set_list_);
  new_module_set_list.clear();

  // {x ∪ y | x∈X, y∈Y}
  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
    
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_module_set_list.push_back(ms);
    }
  }

  sort(new_module_set_list.begin(), new_module_set_list.end(), ModuleSetComparator());
  module_set_list_.swap(new_module_set_list);
  build_edges();
}

void  ModuleSetGraph::add_weak(ModuleSetGraph& weak_module_set_graph)
{
  // ordered(X, Y) = Y ∪ {x ∪ y | x∈X, y∈Y}
      
  // Y
  module_set_list_t new_module_set_list(module_set_list_);

  // {x ∪ y | x∈X, y∈Y}
  module_set_list_t::const_iterator p_it = 
    weak_module_set_graph.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    weak_module_set_graph.module_set_list_.end();

  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
  
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_module_set_list.push_back(ms);
    }
  }

  sort(new_module_set_list.begin(), new_module_set_list.end(), ModuleSetComparator());
  module_set_list_.swap(new_module_set_list);
  build_edges();
}

namespace {
struct ModSizePred {
  ModSizePred(size_t size) :
    size_(size)
  {}

  bool operator()(const hydla::ch::module_set_sptr& n) const {
    return n->size() == size_;
  }

  size_t size_;
};
}

void ModuleSetGraph::build_edges()
{
  edges_.clear();

  if(module_set_list_.size() == 0) return;

  module_set_list_t::iterator node_begin = module_set_list_.begin();
  module_set_list_t::iterator node_end   = module_set_list_.end();

  int modsize = (int)(*node_begin)->size();

  module_set_list_t::iterator super_it = node_begin;
  module_set_list_t::iterator super_end = 
    std::find_if(node_begin, node_end, ModSizePred(--modsize));
  
  while(super_it!=node_end) {
    module_set_list_t::iterator subset_it  = super_end;
    module_set_list_t::iterator subset_end = 
      std::find_if(subset_it, node_end, ModSizePred(--modsize));
    
    for(; super_it!=super_end; ++super_it) {
      for(module_set_list_t::iterator tmp_subset_it = subset_it;
          tmp_subset_it!=subset_end; 
          ++tmp_subset_it) {
        if((*super_it)->is_super_set(**tmp_subset_it)) {
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
  module_set_list_t::const_iterator it  = module_set_list_.begin();
  module_set_list_t::const_iterator end = module_set_list_.end();

  s << "{";
  if(it!=end) s << (*it++)->get_name();
  while(it!=end) {
    s << ", " << (*it++)->get_name();
  }
  s << "}";

  return s;
}

std::ostream& ModuleSetGraph::dump_node_trees(std::ostream& s) const
{
  module_set_list_t::const_iterator it  = module_set_list_.begin();
  module_set_list_t::const_iterator end = module_set_list_.end();

  s << "{";
  if(it!=end) s << **(it++);
  while(it!=end) {
    s << ",\n" << **(it++);
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
    s << it->first->get_name() 
      << "->" 
      << it->second->get_name() 
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
      << it->first->get_name() 
      << "\" -> \"" 
      << it->second->get_name() 
      << "\";\n";
  }

  s << "}" << std::endl;

  return s;
}

void ModuleSetGraph::reset(const module_set_list_t &mss){
  ms_to_visit_ = mss;
  visited_module_sets_.clear();
}

void ModuleSetGraph::mark_nodes()
{
  mark_visited_flag(ms_to_visit_.front());
}

void ModuleSetGraph::mark_visited_flag(const module_set_sptr& ms)
{
  if(visited_module_sets_.find(ms) != visited_module_sets_.end()) return;
  visited_module_sets_.insert(ms);
  

  std::pair<edges_t::map_by<superset>::const_iterator, 
            edges_t::map_by<superset>::const_iterator>
  superset_range = edges_.by<superset>().equal_range(ms);

  for(; superset_range.first!=superset_range.second; ++superset_range.first) {
    mark_visited_flag(superset_range.first->get<subset>());
  }
  
  {
    module_set_list_t::iterator it = ms_to_visit_.begin();
    for(; it != ms_to_visit_.end(); it++)
    {
      if((*it).get() == ms.get())
      {
        break;
      }
    }
    
    if(it != ms_to_visit_.end())
    {
      it = ms_to_visit_.erase(it++);
    }
  }
}

} // namespace ch
} // namespace hydla
