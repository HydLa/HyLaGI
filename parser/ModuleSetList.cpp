#include "ModuleSetList.h"

#include <iostream>
#include <algorithm>

#include <boost/lambda/lambda.hpp>

namespace hydla {
namespace ch {

ModuleSetList::ModuleSetList()
{}

ModuleSetList::ModuleSetList(module_set_sptr m) :
  module_set_list_(1, m)
{}

ModuleSetList::~ModuleSetList()
{}

void ModuleSetList::add_parallel(ModuleSetList& parallel_module_set_list) 
{
  // parallel(X, Y) = X Åæ Y Åæ {x Åæ y | xÅ∏X, yÅ∏Y}

  module_set_list_t::const_iterator p_it = 
    parallel_module_set_list.module_set_list_.begin();
  module_set_list_t::const_iterator p_end = 
    parallel_module_set_list.module_set_list_.end();

  // X
  module_set_list_t new_list(module_set_list_);
    
  // Y
  new_list.insert(new_list.end(), p_it, p_end);

  // {x Åæ y | xÅ∏X, yÅ∏Y}
  for(; p_it!=p_end; ++p_it) {
    module_set_list_t::iterator this_it =  module_set_list_.begin();
    module_set_list_t::iterator this_end = module_set_list_.end();
    
    for(; this_it!=this_end; ++this_it) {
      std::cout << **this_it << ":" << **p_it << std::endl;

      module_set_sptr ms(new ModuleSet(**this_it,  **p_it));
      new_list.push_back(ms);
    }
  }


  sort(new_list.begin(), new_list.end(), 
    *boost::lambda::_1 < *boost::lambda::_2);

  module_set_list_.swap(new_list);
}

void ModuleSetList::add_weak(ModuleSetList& weak_module_set_list) 
{
  // ordered(X, Y) = Y Åæ {x Åæ y | xÅ∏X, yÅ∏Y}
      
  // Y
  module_set_list_t new_list(module_set_list_);

  // {x Åæ y | xÅ∏X, yÅ∏Y}
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

  sort(new_list.begin(), new_list.end(), 
    *boost::lambda::_1 < *boost::lambda::_2);

  module_set_list_.swap(new_list);
}

std::ostream& ModuleSetList::dump(std::ostream& s)
{
  module_set_list_t::iterator it = module_set_list_.begin();
  module_set_list_t::iterator end = module_set_list_.end();
  
  s << "{";
  if(it!=end) s << "{" << **(it++) << "}";
  for(; it!=end; ++it) {
    s << ", {" << **it << "}";
  }
  s << "}";
  return s;
}

std::ostream& operator<<(std::ostream& s, ModuleSetList& m)
{
  return m.dump(s);
}

} // namespace ch
} // namespace hydla