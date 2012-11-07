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

bool ModuleSetContainer::eliminate_current_module_set(){
  current_module_set_ = module_set_list_.erase(current_module_set_);
  return current_module_set_ != module_set_list_.end();  
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

module_set_sptr ModuleSetContainer::get_module_set()const{
  return *current_module_set_;
}


std::set<module_set_sptr> ModuleSetContainer::get_visited_module_sets()const{
  return visited_module_sets_;
}


void ModuleSetContainer::mark_current_node(){
  visited_module_sets_.insert(*current_module_set_);
}

void ModuleSetContainer::reset(){
  module_set_list_t::iterator it  = current_module_set_ = module_set_list_.begin();
  module_set_list_t::iterator end = module_set_list_.end();

  // ‘Sƒm[ƒh‚ğ–¢’Tõó‘Ô‚É‚·‚é
  visited_module_sets_.clear();
}


void ModuleSetContainer::reset(const std::set<module_set_sptr> &mss){
  visited_module_sets_ = mss;
  current_module_set_ = module_set_list_.begin();
  go_next();
}

} // namespace ch
} // namespace hydla
