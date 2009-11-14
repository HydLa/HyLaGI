#include "ModuleSet.h"

#include <iostream>
#include <algorithm>

#include <boost/lambda/lambda.hpp>

namespace hydla {
namespace ch {

ModuleSet::ModuleSet()
{}

ModuleSet::ModuleSet(std::string& name, hydla::parse_tree::node_sptr node) :
  module_list_(1, std::make_pair(name, node))
{}

ModuleSet::ModuleSet(ModuleSet& lhs, ModuleSet& rhs) :
  module_list_(lhs.module_list_.size() + rhs.module_list_.size())
{
  std::merge(lhs.module_list_.begin(), lhs.module_list_.end(),
             rhs.module_list_.begin(), rhs.module_list_.end(),
             module_list_.begin());
}

ModuleSet::~ModuleSet()
{
}

std::string ModuleSet::get_name() const 
{
  std::string str;
  module_list_t::const_iterator it  = module_list_.begin();    
  module_list_t::const_iterator end = module_list_.end();

  str += "{";
  if(it!=end) str += (it++)->first;
  for(; it!=end; ++it) {
    str += ", ";
    str += it->first;
  }
  str += "}";
  return str;
}

int ModuleSet::compare(ModuleSet& rhs) const
{
  module_list_t::const_iterator this_it  = module_list_.begin();
  module_list_t::const_iterator this_end = module_list_.end();
  module_list_t::const_iterator rhs_it   = rhs.module_list_.begin();
  
  int comp = (int)module_list_.size() - rhs.module_list_.size();
  while(comp==0 && this_it!=this_end) {
    comp = (this_it++)->first.compare((rhs_it++)->first);
  }
  return comp;
}

std::ostream& operator<<(std::ostream& s, ModuleSet& m)
{
  return m.dump(s);
}

} // namespace ch
} // namespace hydla
