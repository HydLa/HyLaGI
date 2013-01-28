#include "ModuleSetList.h"

#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace hydla {
namespace ch {

ModuleSetList::ModuleSetList()
{}

ModuleSetList::ModuleSetList(module_set_sptr m) :
  ModuleSetContainer(m)
{}

ModuleSetList::~ModuleSetList()
{}

void ModuleSetList::add_parallel(ModuleSetList& parallel_module_set_list) 
{

  // parallel(X, Y) = X ∪ Y ∪ {x ∪ y | x∈X, y∈Y}

  module_set_list_t::const_iterator p_it = 
    parallel_module_set_list.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    parallel_module_set_list.module_set_list_.end();

  // Y
  module_set_list_t new_list(module_set_list_);
    
  // X
  new_list.insert(new_list.end(), p_it, p_end);

  // {x ∪ y | x∈X, y∈Y}
  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
    
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_list.push_back(ms);
    }
  }

  sort(new_list.begin(), new_list.end(), ModuleSetComparator());

  module_set_list_.swap(new_list);
}

void ModuleSetList::add_required_parallel(ModuleSetList& parallel_module_set_list) 
{
  // parallel(X, Y) = {x ∪ y | x∈X, y∈Y}

  module_set_list_t::const_iterator p_it = 
    parallel_module_set_list.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    parallel_module_set_list.module_set_list_.end();

  // 空のモジュール集合の集合を用意
  module_set_list_t new_list(module_set_list_);
  new_list.clear();

  // {x ∪ y | x∈X, y∈Y}
  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
    
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_list.push_back(ms);
    }
  }

  sort(new_list.begin(), new_list.end(), ModuleSetComparator());

  module_set_list_.swap(new_list);
}

void ModuleSetList::add_weak(ModuleSetList& weak_module_set_list) 
{
  // ordered(X, Y) = Y ∪ {x ∪ y | x∈X, y∈Y}
      
  // Y
  module_set_list_t new_list(module_set_list_);

  // {x ∪ y | x∈X, y∈Y}
  module_set_list_t::const_iterator p_it = 
    weak_module_set_list.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    weak_module_set_list.module_set_list_.end();

  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
  
    for(; this_it!=this_end; ++this_it) {
      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_list.push_back(ms);
    }
  }

  sort(new_list.begin(), new_list.end(), ModuleSetComparator());

  module_set_list_.swap(new_list);
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
  module_set_list_t::const_iterator it  = module_set_list_.begin();
  module_set_list_t::const_iterator end = module_set_list_.end();

  s << "{";
  if(it!=end) s << (*(it++))->get_name();
  while(it!=end) {
    s << ", " << (*(it++))->get_name();
  }
  s << "}";

  return s;
}

std::ostream& ModuleSetList::dump_node_trees(std::ostream& s) const
{
  module_set_list_t::const_iterator it  = module_set_list_.begin();
  module_set_list_t::const_iterator end = module_set_list_.end();

  s << "{";
  if(it!=end) s << **(it++);
  while(it!=end) {
    s << ", " << **(it++);
  }
  s << "}";

  return s;
}


void ModuleSetList::mark_nodes(){
  ms_to_visit_.clear();
}

} // namespace ch
} // namespace hydla