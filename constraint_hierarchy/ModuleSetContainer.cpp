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

bool ModuleSetContainer::eliminate_r_current_module_set(){
  module_set_list_t::reverse_iterator it = r_current_module_set_;
  it++;
  while(it != module_set_list_.rend()){
    if((*it)->is_super_set(*(*r_current_module_set_))){
      //      std::cout << "current      : " << *(*r_current_module_set_) << std::endl;
      //      std::cout << "  eliminated : " << *(*it) << std::endl;
      module_set_list_.erase(--(it.base()));
      r_current_module_set_++;
    }else{
      //      std::cout << "current    : " << *(*r_current_module_set_) << std::endl;
      //      std::cout << "  remained : " << *(*it) << std::endl;
      it++;
    }
  }
  module_set_list_.erase(--(r_current_module_set_.base()));
  return r_current_module_set_ != module_set_list_.rend();  
}

void ModuleSetContainer::add_conditions_to_super_set(){
  /*
1. まずModuleSetクラスに変数の値を保持するメンバ変数を用意する
   (mapかvectorでいいと思う)
2. module_set_list_のreverse_iteratorを使ってr_current_module_set_以下の
   module_setに
   if(module_set.is_super_set(*current_module_set)){
     // r_current_module_set_が探索済みならこの関数は呼ばない
     // 他の子ノードから尋ねられたときに重複するが、
     // 変数表の中身が違えば、それだけ矛盾の候補の条件が追加されるので、
     // 特に省いたりしない
     // この方式なら、そこまですごく多いものを
     // チェックすることにはならないと思う
     変数表追加
     visited_module_sets_.insert(*module_set);
   }
   のような処理を追加
   */
  module_set_list_t::reverse_iterator it = r_current_module_set_;
  it++;
  //  std::cout << (*r_current_module_set_)->get_name() << " : " << std::endl;
  for(; it != module_set_list_.rend(); it++){
    if((*it)->is_super_set(*(*r_current_module_set_))){
      //      std::cout << "   " << (*it)->get_name() << std::endl;
      (*it)->set_false_conditions((*r_current_module_set_)->get_false_conditions());
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
  // 全ノードを未探索状態にする
  visited_module_sets_.clear();
}


void ModuleSetContainer::reset(const std::set<module_set_sptr> &mss){
  visited_module_sets_ = mss;
  current_module_set_ = module_set_list_.begin();
  go_next();
}

} // namespace ch
} // namespace hydla
