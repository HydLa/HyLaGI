#include "ModuleSetContainer.h"
#include <iostream>

namespace hydla {
namespace hierarchy {

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m)
{
  return m.dump(s);
}


ModuleSetContainer::ModuleSetContainer(ModuleSet &m) :
  module_set_list_(1, m)
{}

bool ModuleSetContainer::go_next(){
  return !ms_to_visit_.empty();
}

ModuleSet ModuleSetContainer::get_max_module_set() const{
if(module_set_list_.empty())
{
  return ModuleSet();
}
  return module_set_list_.front();
}

ModuleSet ModuleSetContainer::get_module_set()const{
  return ms_to_visit_.front();
}

ModuleSetContainer::module_set_list_t ModuleSetContainer::get_ms_to_visit()const{
  return ms_to_visit_;
}

ModuleSetContainer::module_set_list_t ModuleSetContainer::get_full_ms_list()const{
  return module_set_list_;
}

void ModuleSetContainer::mark_current_node(){
  ms_to_visit_.erase(ms_to_visit_.begin());
}

void ModuleSetContainer::reset(){
  reset(module_set_list_);
}

void ModuleSetContainer::reset(const module_set_list_t &mss){
  ms_to_visit_ = mss;
}

void ModuleSetContainer::mark_nodes(const ModuleSet& ms){
  unsigned int diff;
  
  module_set_list_t::iterator it = ms_to_visit_.begin();
  do
  {
    diff = it->size() - ms.size();
    if(it->including(ms))
    {
      it = ms_to_visit_.erase(it);
    }
    else
    {
      it++;
    }
  }while(diff > 0 && it != ms_to_visit_.end());
}

void ModuleSetContainer::mark_nodes(const module_set_list_t& mms, const ModuleSet& ms){
  mark_nodes(ms);  
}

} // namespace hierarchy
} // namespace hydla
