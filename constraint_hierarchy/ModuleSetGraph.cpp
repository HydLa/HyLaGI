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
  // parallel(X, Y) = X ¾ Y ¾ {x ¾ y | x¸X, y¸Y}
  nodes_t::const_iterator p_it = 
    parallel_module_set_graph.nodes_.begin();
  nodes_t::const_iterator p_end = 
    parallel_module_set_graph.nodes_.end();

  // X
  nodes_t new_nodes(nodes_);
    
  // Y
  new_nodes.insert(new_nodes.end(), p_it, p_end);

  // {x ¾ y | x¸X, y¸Y}
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
}

void  ModuleSetGraph::add_weak(ModuleSetGraph& weak_module_set_graph)
{
  // ordered(X, Y) = Y ¾ {x ¾ y | x¸X, y¸Y}
      
  // Y
  nodes_t new_nodes(nodes_);

  // {x ¾ y | x¸X, y¸Y}
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

  std::cout << "aaa" << nodes_.size() << std::endl;

  if(nodes_.size() == 0) return;

  {
    nodes_t::const_iterator it  = nodes_.begin();
    nodes_t::const_iterator end = nodes_.end();

    for(; it!=end; ++it) {
      std::cout << it->mod << std::endl;
    }
  }

  nodes_t::const_iterator node_begin = nodes_.begin();
  nodes_t::const_iterator node_end   = nodes_.end();

  int modsize = (int)node_begin->mod->size();
  std::cout << "modsize:" << modsize << std::endl;

  nodes_t::const_iterator super_it = node_begin;
  nodes_t::const_iterator super_end = 
    std::find_if(node_begin, node_end, ModSizePred(--modsize));
  
  while(super_it!=node_end) {
  std::cout << "a" << std::endl;

    nodes_t::const_iterator subset_it  = super_end;
    nodes_t::const_iterator subset_end = 
      std::find_if(subset_it, node_end, ModSizePred(--modsize));
    
    for(; super_it!=super_end; ++super_it) {
      std::cout << "super_it:" << super_it->mod << std::endl;
      std::cout << "super_end:" << super_end->mod << std::endl;
      std::cout << "subset_it:" << subset_it->mod << std::endl;
      std::cout << "subset_end:" << subset_end->mod << std::endl;

      std::cout << "b" << std::endl;

      for(nodes_t::const_iterator tmp_subset_it = subset_it;
          tmp_subset_it!=subset_end; 
          ++tmp_subset_it) {
        std::cout << super_it->mod->get_name() << ">>>>" 
                  << tmp_subset_it->mod->get_name() << std::endl;

        if(super_it->mod->is_super_set(*tmp_subset_it->mod)) {
          std::cout << super_it->mod->get_name() << "=>" 
                    << tmp_subset_it->mod->get_name() << std::endl;
        }
      }
    } 
    
    super_it  = subset_it;
    super_end = subset_end;
  }
}

std::string ModuleSetGraph::get_name() const
{
  std::string s;
  nodes_t::const_iterator it  = nodes_.begin();
  nodes_t::const_iterator end = nodes_.end();
  
  s += "{";
  if(it!=end) s += (it++)->mod->get_name();
  while(it!=end) {
    s += ", " + (it++)->mod->get_name();
  }
  s += "}";
  return s;
}

std::ostream& ModuleSetGraph::dump(std::ostream& s) const
{
  return s;
}

bool ModuleSetGraph::dispatch(
  boost::function<bool (hydla::ch::module_set_sptr)> callback_func, 
  int threads)
{
  nodes_t::const_iterator it  = nodes_.begin();
  nodes_t::const_iterator end = nodes_.end();
  while(it!=end) {
    if(callback_func((it++)->mod)) return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& s, const ModuleSetGraph& m)
{
  return m.dump(s);
}


} // namespace ch
} // namespace hydla
