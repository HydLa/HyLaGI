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
  module_set_list_.erase(current_module_set_++);
  return current_module_set_ != module_set_list_.end();
}

void ModuleSetContainer::mark_super_module_set(){
  module_set_list_t::reverse_iterator it = r_current_module_set_;
  it++;
  for(; it != module_set_list_.rend(); it++){
    if((*it)->is_super_set(*(*r_current_module_set_))){
      visited_module_sets_.insert(*it);
    }
  }
}

bool ModuleSetContainer::reverse_go_next(){
  while(r_current_module_set_ != module_set_list_.rend() && visited_module_sets_.find(*r_current_module_set_) != visited_module_sets_.end()){
    r_current_module_set_++;
  }
  return r_current_module_set_ != module_set_list_.rend();
}

bool ModuleSetContainer::go_next(){
  while(current_module_set_ != module_set_list_.end() && visited_module_sets_.find(*current_module_set_) != visited_module_sets_.end()){
    current_module_set_++;
  }
  return current_module_set_ != module_set_list_.end();
}

module_set_sptr ModuleSetContainer::get_max_module_set() const{
  return module_set_list_.front();
}

module_set_sptr ModuleSetContainer::get_reverse_module_set() const{ 
  return *r_current_module_set_;
}

module_set_sptr ModuleSetContainer::get_module_set()const{
  return *current_module_set_;
}


std::set<module_set_sptr> ModuleSetContainer::get_visited_module_sets()const{
  return visited_module_sets_;
}

void ModuleSetContainer::mark_r_current_node(){
  visited_module_sets_.insert(*r_current_module_set_);
}

void ModuleSetContainer::mark_current_node(){
  visited_module_sets_.insert(*current_module_set_);
}

void ModuleSetContainer::reverse_reset(){
  r_current_module_set_ = module_set_list_.rbegin();
  visited_module_sets_.clear();
}


void ModuleSetContainer::reset(){
  //module_set_list_t::iterator it  = current_module_set_ = module_set_list_.begin();
  //module_set_list_t::iterator end = module_set_list_.end();
  current_module_set_ = module_set_list_.begin();
  // ëSÉmÅ[ÉhÇñ¢íTçıèÛë‘Ç…Ç∑ÇÈ
  visited_module_sets_.clear();
}


void ModuleSetContainer::reset(const std::set<module_set_sptr> &mss){
  visited_module_sets_ = mss;
  current_module_set_ = module_set_list_.begin();
  go_next();
}

} // namespace ch
} // namespace hydla
