#include "ModuleSetContainer.h"
#include <iostream>

namespace hydla {
namespace ch {

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m)
{
  return m.dump(s);
}


ModuleSetContainer::ModuleSetContainer(module_set_sptr m) :
  module_set_list_(1, m)
{}

bool ModuleSetContainer::eliminate_current_node(){
  for(module_set_list_t::iterator it = module_set_list_.begin(); it != module_set_list_.end(); it++)
  {
    if((*it)->get_name() == ms_to_visit_.front()->get_name())
    {
      module_set_list_.erase(it);
      break;
    }
  }
  ms_to_visit_.erase(ms_to_visit_.begin());
  return !ms_to_visit_.empty();
}

void ModuleSetContainer::mark_super_module_set(){
  module_set_list_t::reverse_iterator it = r_ms_to_visit_.rbegin();
  it++;
  for(; it != r_ms_to_visit_.rend();it++){
    if((*it)->is_super_set(*r_ms_to_visit_.front())){
      r_ms_to_visit_.erase(--it.base());
    }
  }
}

bool ModuleSetContainer::reverse_go_next(){
  return !r_ms_to_visit_.empty();
}

bool ModuleSetContainer::go_next(){
  return !ms_to_visit_.empty();
}

module_set_sptr ModuleSetContainer::get_max_module_set() const{
if(module_set_list_.empty())
{
  return module_set_sptr(new ModuleSet());
}
  return module_set_list_.front();
}

module_set_sptr ModuleSetContainer::get_reverse_module_set() const{ 
  return r_ms_to_visit_.back();
}

module_set_sptr ModuleSetContainer::get_module_set()const{
  return ms_to_visit_.front();
}

ModuleSetContainer::module_set_list_t ModuleSetContainer::get_ms_to_visit()const{
  return ms_to_visit_;
}

ModuleSetContainer::module_set_list_t ModuleSetContainer::get_full_ms_list()const{
  return module_set_list_;
}

void ModuleSetContainer::mark_r_current_node(){
  r_ms_to_visit_.erase(r_ms_to_visit_.rbegin().base());
}

void ModuleSetContainer::mark_current_node(){
  ms_to_visit_.erase(ms_to_visit_.begin());
}

void ModuleSetContainer::reverse_reset(){
  r_ms_to_visit_ = module_set_list_;
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
    diff = (*it)->size() - ms.size();
    if((*it)->including(ms))
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

} // namespace ch
} // namespace hydla
