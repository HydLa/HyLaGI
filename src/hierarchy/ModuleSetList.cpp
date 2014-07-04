#include "ModuleSetList.h"

#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace hydla {
namespace hierarchy {

ModuleSetList::ModuleSetList()
{}

ModuleSetList::ModuleSetList(ModuleSet m) :
  ModuleSetContainer(m)
{}

ModuleSetList::~ModuleSetList()
{}

void ModuleSetList::add_parallel(ModuleSetList& parallel_module_set_list) 
{

  // parallel(X, Y) = X ∪ Y ∪ {x ∪ y | x∈X, y∈Y}
  // Y
  module_set_set_t new_list(full_module_set_set_);
    
  // X
  for(auto p_it : parallel_module_set_list.full_module_set_set_){
    new_list.insert(p_it);
  }

  // {x ∪ y | x∈X, y∈Y}
  for(auto p_it : parallel_module_set_list.full_module_set_set_) {
    for(auto this_it : full_module_set_set_) {
      ModuleSet ms(this_it, p_it);
      new_list.insert(ms);
    }
  }

  full_module_set_set_.swap(new_list);
  maximal_module_set_ = *full_module_set_set_.rbegin();
}

void ModuleSetList::add_required_parallel(ModuleSetList& parallel_module_set_list) 
{
  // 空のモジュール集合の集合を用意
  module_set_set_t new_list;

  // {x ∪ y | x∈X, y∈Y}
  for(auto p_it : parallel_module_set_list.full_module_set_set_) {
    for(auto this_it : full_module_set_set_) {
      ModuleSet ms(this_it, p_it);
      new_list.insert(ms);
    }
  }

  full_module_set_set_.swap(new_list);
  maximal_module_set_ = *full_module_set_set_.rbegin();
}

void ModuleSetList::add_weak(ModuleSetList& weak_module_set_list) 
{
  // ordered(X, Y) = Y ∪ {x ∪ y | x∈X, y∈Y}
      
  // Y
  module_set_set_t new_list(full_module_set_set_);

  ModuleSet y = *full_module_set_set_.rbegin();
  // {x ∪ y | x∈X, y∈Y}
  for(auto p_it : weak_module_set_list.full_module_set_set_) {
    ModuleSet ms(y, p_it);
    new_list.insert(ms);
  }

  full_module_set_set_.swap(new_list);
  maximal_module_set_ = *full_module_set_set_.rbegin();
}

std::ostream& ModuleSetList::dump(std::ostream& s) const
{
  dump_node_names(s);
  s << "\n";
  dump_node_trees(s);

  return s;
}

std::ostream& ModuleSetList::dump_node_names(std::ostream& s) const
{
  module_set_set_t::const_iterator it  = full_module_set_set_.begin();
  module_set_set_t::const_iterator end = full_module_set_set_.end();

  s << "{";
  if(it!=end) s << (*(it++)).get_name();
  while(it!=end) {
    s << ", " << (*(it++)).get_name();
  }
  s << "}";

  return s;
}

std::ostream& ModuleSetList::dump_node_trees(std::ostream& s) const
{
  module_set_set_t::const_iterator it  = full_module_set_set_.begin();
  module_set_set_t::const_iterator end = full_module_set_set_.end();

  s << "{";
  if(it!=end) s << *(it++);
  while(it!=end) {
    s << ", " << *(it++);
  }
  s << "}";

  return s;
}

} // namespace hierarchy
} // namespace hydla
