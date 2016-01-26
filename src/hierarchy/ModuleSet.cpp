#include "ModuleSet.h"

#include <iostream>
#include <algorithm>

#include <boost/lambda/lambda.hpp>

#include "TreeInfixPrinter.h"
#include <sstream>

namespace hydla {
namespace hierarchy {

ModuleSet::ModuleSet()
{}

ModuleSet::ModuleSet(const std::string& name, hydla::symbolic_expression::node_sptr node)
{
  module_list_.insert(std::make_pair(name, node));
}


ModuleSet::ModuleSet(ModuleSet& lhs, ModuleSet& rhs)
{
  module_list_.insert(lhs.module_list_.begin(), lhs.module_list_.end());
  module_list_.insert(rhs.module_list_.begin(), rhs.module_list_.end());
}

/*
ModuleSet::ModuleSet(const ModuleSet &ms)
{
  for(auto module : ms.module_list_)
    module_list_.insert(module_t(module.first, module.second->clone()));
}
*/

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

bool ModuleSet::disjoint(const ModuleSet& ms) const
{
  for(auto m : ms){
    if(module_list_.count(m)) return false;
  }
  return true;
}

ModuleSet::module_list_const_iterator ModuleSet::find(const module_t& mod) const
{
  return module_list_.find(mod);
  /*
  module_list_t::const_iterator it  = module_list_.begin();    
  module_list_t::const_iterator end = module_list_.end();

  for(; it != end; ++it) {
    if(it->first == mod.first && it->second->get_id() == mod.second->get_id()) return it;
  }
  return this->end();
  */
}

std::string ModuleSet::get_infix_string() const 
{
  hydla::symbolic_expression::TreeInfixPrinter printer;
  std::ostringstream ostr;
  module_list_t::const_iterator it  = module_list_.begin();    
  module_list_t::const_iterator end = module_list_.end();

  for(; it!=end; ++it) {
    printer.print_infix(it->second, ostr);
  }
  return ostr.str();
}

std::ostream& ModuleSet::dump(std::ostream& s) const
{
  std::string str;
  module_list_t::const_iterator it  = module_list_.begin();    
  module_list_t::const_iterator end = module_list_.end();

  s << "{";
  if(it != end) s << *(it++)->second;
  for(; it!=end; ++it) {
    s << ", " << *it->second;
  }
  s << "}";
  return s;
}

int ModuleSet::compare(const ModuleSet& rhs) const
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

int ModuleSet::erase(const module_t& m)
{
  return module_list_.erase(m);
}

int ModuleSet::erase(const ModuleSet& ms)
{
  int sum = 0;
  for(auto m : ms) sum += erase(m);
  return sum;
}	


ModuleSet::module_list_const_iterator ModuleSet::erase(const module_list_const_iterator &it)
{
  return module_list_.erase(it);
}	

bool ModuleSet::including(const ModuleSet& ms) const
{
  for(auto m : ms)
  {
    if(!module_list_.count(m))
    {
      return false;
    }
  }
  return true;
}

std::ostream& operator<<(std::ostream& s, const ModuleSet& m)
{
  return m.dump(s);
}

std::ostream& operator<<(std::ostream& s, const ModuleSet::module_t& m)
{
  return s << m.first;
}


} // namespace hierarchy
} // namespace hydla
