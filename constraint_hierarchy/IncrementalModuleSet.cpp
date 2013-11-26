#include "IncrementalModuleSet.h"

#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace hydla {
namespace ch {

IncrementalModuleSet::IncrementalModuleSet()
{}

IncrementalModuleSet::IncrementalModuleSet(module_set_sptr m) :
maximal_module_set_(m), formated_(false)
{}

IncrementalModuleSet::IncrementalModuleSet(const IncrementalModuleSet& im)
{
  parents_data_ = im.parents_data_;
  required_ms_ = im.required_ms_;
  maximal_module_set_ = im.maximal_module_set_;
  /*
  for(module_set_list_t::const_iterator it = im.checked_ms_.begin(); it != im.checked_ms_.end(); it++){
    checked_ms_.push_back(*it);
  }
  */
  formated_ = im.formated_;
}

IncrementalModuleSet::~IncrementalModuleSet()
{}

void IncrementalModuleSet::add_maximal_module_set(module_set_sptr ms)
{
  // この関数を呼び出したインスタンスが持つmaximal_module_set_のイテレータ
  module_list_const_iterator p_it = 
    maximal_module_set_->begin();
  module_list_const_iterator p_end =
    maximal_module_set_->end();
  
  // 引数のmsに要素を追加する
  for(; p_it != p_end; ++p_it){
    //    多分このif文はなくても問題ないはず
    //    if(maximal_module_set_->find(*p_it)!=maximal_module_set_->end())
      ms->add_module(*p_it);
  }
  // maximal_module_set_の更新
  maximal_module_set_ = ms;
  // リスト内で優先度が低いモジュールはそれより優先度が高いモジュールより必ず前に来るようにする
}

void IncrementalModuleSet::format_parents_data()
{
  std::map<module_t,int> include_weak;
  std::map<module_t,int> will_remove;
  m_str_ms_t::iterator it = parents_data_.begin();
  m_str_ms_t::iterator end = parents_data_.end();
  while(it!=end){
    str_ms_list_t::iterator str_ms_it = it->second.begin();
    str_ms_list_t::iterator str_ms_end = it->second.end();
    bool erased = false;
    unsigned int end;
    for(unsigned int i = 0; i < it->second.size(); i++){
      switch(it->second[i].first){
      case WEAKER_THAN:
        // << の左側
        // 取り除かれる場合があるフラグ(i+1はどうでもいい)
        will_remove[it->first] = i+1;
        // ここまでに << の右側になっていなければ必ず取り除くことができる
        // << の左側になった時点で required にはなりえない
        if(include_weak.find(it->first)==include_weak.end()){
          parents_data_.erase(it++);
          erased = true;
          break;
        }
        end = parents_data_[it->first].size();
        // 最後に << の右側になってから << の左側になるまでの情報は必要ない
        // << の左側より上の親の直接の子になることはない
        for(unsigned int j = 0; j < end-include_weak[it->first]; j++){
          parents_data_[it->first].pop_back();
        }
        break;
      case STRONGER_THAN:
        // << の右側
        // 最後にどの位置の親ノードで << の右側になったのか覚えておく
        include_weak[it->first] = i+1;
        break;
      case PARALLEL:
        // required でない , 
        // 取り除ける場合があるフラグ(i+1は実はどうでもいい)
        will_remove[it->first] = i+1;
        break;
      case REQUIRED_PARALLEL:
	break;
      }
      if(erased) break;
    }
    if(!erased) it++;
  }
  
  module_list_const_iterator m_it = maximal_module_set_->begin();
  module_list_const_iterator m_end = maximal_module_set_->end();
  required_ms_ = module_set_sptr(new ModuleSet());
  // <<の左側に来ない、あるいはrequiredでない,の子で無いノードはrequired
  for(; m_it!=m_end; m_it++){
    if(will_remove.find(*m_it)==will_remove.end()){
      required_ms_->add_module(*m_it);
      parents_data_.erase(*m_it);
    }
  }
}

module_set_sptr IncrementalModuleSet::get_removable_module_set(const ModuleSet& ms)
{
  module_set_sptr removable(new ModuleSet());
  module_set_sptr removed_ms(new ModuleSet());

  module_list_const_iterator it = maximal_module_set_->begin();
  module_list_const_iterator end = maximal_module_set_->end();

  for(; it!=end; it++){
    if(ms.find(*it)==ms.end()) removed_ms->add_module(*it);
  }

  it = ms.begin();
  end = ms.end();

  for(; it!=end; it++){
    if(required_ms_->find(*it)!=required_ms_->end()) continue;
    if(parents_data_.find(*it)==parents_data_.end()){
      removable->add_module(*it);
      continue;
    }
    str_ms_list_t::iterator p_it = parents_data_[*it].begin();
    str_ms_list_t::iterator p_end = parents_data_[*it].end();
    for(; p_it!=p_end; p_it++){
      if(removed_ms->including(*(p_it->second))) continue;
      switch(p_it->first){
      case WEAKER_THAN:
      case PARALLEL:
        removable->add_module(*it);
        break;
      default:
        break;
      }
      break;
    }
    if(p_it==p_end) removable->add_module(*it);
  }
  return removable;

}

void IncrementalModuleSet::add_parallel(IncrementalModuleSet& parallel_module_set_list) 
{
  // parallel(X, Y) = X ∪ Y ∪ {x ∪ y | x∈X, y∈Y}
  module_list_const_iterator p_it = 
    parallel_module_set_list.maximal_module_set_->begin();
  module_list_const_iterator p_end =
    parallel_module_set_list.maximal_module_set_->end();
  module_list_const_iterator this_it = maximal_module_set_->begin();
  module_list_const_iterator this_end = maximal_module_set_->end();

  parents_data_.insert(
    parallel_module_set_list.parents_data_.begin(),
    parallel_module_set_list.parents_data_.end()
  );

  module_set_sptr new_ms(new ModuleSet());
  module_set_sptr against_max(parallel_module_set_list.maximal_module_set_);
  for(; this_it != this_end; ++this_it){
    parents_data_[*this_it].push_back(str_ms_pair_t(PARALLEL,against_max));
  }
  for(; p_it!=p_end; ++p_it){
    parents_data_[*p_it].push_back(str_ms_pair_t(PARALLEL,maximal_module_set_));
    new_ms->add_module(*p_it);
  }
  add_maximal_module_set(new_ms);
}

void IncrementalModuleSet::add_required_parallel(IncrementalModuleSet& parallel_module_set_list) 
{
  // parallel(X, Y) = {x ∪ y | x∈X, y∈Y}
  module_list_const_iterator p_it = 
    parallel_module_set_list.maximal_module_set_->begin();
  module_list_const_iterator p_end =
    parallel_module_set_list.maximal_module_set_->end();
  module_list_const_iterator this_it = maximal_module_set_->begin();
  module_list_const_iterator this_end = maximal_module_set_->end();

  parents_data_.insert(
    parallel_module_set_list.parents_data_.begin(),
    parallel_module_set_list.parents_data_.end()
  );

  module_set_sptr new_ms(new ModuleSet());
  module_set_sptr against_max(parallel_module_set_list.maximal_module_set_);
  for(; this_it != this_end; ++this_it){
    parents_data_[*this_it].push_back(str_ms_pair_t(REQUIRED_PARALLEL,against_max));
  }
  for(; p_it!=p_end; ++p_it){
    parents_data_[*p_it].push_back(str_ms_pair_t(REQUIRED_PARALLEL,maximal_module_set_));
    new_ms->add_module(*p_it);
  }
  add_maximal_module_set(new_ms);
}

void IncrementalModuleSet::add_weak(IncrementalModuleSet& weak_module_set_list) 
{
  // ordered(X, Y) = Y ∪ {x ∪ y | x∈X, y∈Y}
  module_list_const_iterator p_it = 
    weak_module_set_list.maximal_module_set_->begin();
  module_list_const_iterator p_end =
    weak_module_set_list.maximal_module_set_->end();
  module_list_const_iterator this_it = maximal_module_set_->begin();
  module_list_const_iterator this_end = maximal_module_set_->end();

  parents_data_.insert(
    weak_module_set_list.parents_data_.begin(),
    weak_module_set_list.parents_data_.end()
  );

  module_set_sptr new_ms(new ModuleSet());
  module_set_sptr against_max(weak_module_set_list.maximal_module_set_);
  for(; this_it != this_end; ++this_it){
    parents_data_[*this_it].push_back(str_ms_pair_t(STRONGER_THAN,against_max));
  }
  for(; p_it!=p_end; ++p_it){
    parents_data_[*p_it].push_back(str_ms_pair_t(WEAKER_THAN,maximal_module_set_));
    new_ms->add_module(*p_it);
  }
  add_maximal_module_set(new_ms);
}

std::ostream& IncrementalModuleSet::dump(std::ostream& s) const
{
  dump_node_names(s);
  s << "\n";
  dump_node_trees(s);

  return s;
}

std::ostream& IncrementalModuleSet::dump_node_names(std::ostream& s) const
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

std::ostream& IncrementalModuleSet::dump_node_trees(std::ostream& s) const
{
  m_str_ms_t::const_iterator it = parents_data_.begin();
  m_str_ms_t::const_iterator end = parents_data_.end();

  s << "*****parents_data_*****" << std::endl;
  while(it!=end){
    s << (*it).first.first << " : ";
    str_ms_list_t::const_iterator it2 = (*it).second.begin();
    str_ms_list_t::const_iterator end2 = (*it).second.end();
    while(it2!=end2){
      s << "{" << (*it2).first << "-" << (*it2).second->get_name() << "}, ";
      it2++;
    }
    s << std::endl;
    it++;
  }
  s << std::endl;
  module_list_const_iterator it2 = required_ms_->begin();
  module_list_const_iterator end2 = required_ms_->end();
  s << "*****required_ms_*****" << std::endl;
  while(it2!=end2){
    s << it2->first << " , "; 
    it2++;
  }
  s << std::endl;

  s << "*****maximal_module_set*****" << std::endl;
  s << maximal_module_set_->get_name() << std::endl;

  s << std::endl;

  s << "***** list *****" << std::endl;

  /*
  module_set_list_t::const_iterator it  = module_set_list_.begin();
  module_set_list_t::const_iterator end = module_set_list_.end();

  s << "{";
  if(it!=end) s << **(it++);
  while(it!=end) {
    s << ", " << **(it++);
  }
  s << "}";
  
  */
  return s;
}


void IncrementalModuleSet::next(){
}

void IncrementalModuleSet::mark_nodes(){
  module_set_sptr current = ms_to_visit_.front();
  module_set_list_t::iterator lit = ms_to_visit_.begin();
  while(lit!=ms_to_visit_.end()){
    if(current->including(**lit)){
      lit = ms_to_visit_.erase(lit);
    }
    else lit++;
  }
}

void IncrementalModuleSet::mark_nodes(const module_set_list_t& mms, const ModuleSet& ms){
  // ここは無くしたい
  if(!formated_){
    format_parents_data();
    formated_ = true;
  }
  module_set_sptr rm = get_removable_module_set(ms);
  module_set_list_t add, remain;
  module_set_list_t::iterator lit = ms_to_visit_.begin();
  for(;lit!=ms_to_visit_.end();lit++){
    if((*lit)->including(ms)){
      module_list_const_iterator mend = (*lit)->end();
      module_set_sptr removed(new ModuleSet());
      while(removed->size()!=rm->size()){
        // *litからrm内の要素一つを除いたものをすべてms_to_visit_に追加
        // ModuleSetにremove(module_t m)みたいのを作ってもいいのかも
        module_list_const_iterator mit = (*lit)->begin();
        module_set_sptr new_ms(new ModuleSet());
        bool not_remove = true;
        for(; mit!=mend; mit++){
          if(rm->find(*mit)!=rm->end()){
            if(not_remove && removed->find(*mit)==removed->end()){
              removed->add_module(*mit);
              not_remove = false;
              continue;
            }
          }
          new_ms->add_module(*mit);
        }
        bool checked = false;
        for(module_set_list_t::iterator cit = ms_to_visit_.begin(); cit != ms_to_visit_.end(); cit++){
	  if((*cit)->including(*new_ms) && new_ms->including(**cit)){
            checked = true;
            break;
          }
        }
        for(module_set_list_t::const_iterator cit = mms.begin(); cit != mms.end(); cit++){
          if((*cit)->including(*new_ms)){
            checked = true;
            break;
	  }
        }
	if(!checked)  add.push_back(new_ms);
      }
    }else{
      remain.push_back(*lit);
    }
  }
  remain.insert(remain.end(),add.begin(),add.end());
  ms_to_visit_ = remain;
}

  module_set_sptr IncrementalModuleSet::get_max_module_set() const{
    return maximal_module_set_;
  }

  IncrementalModuleSet::module_set_list_t IncrementalModuleSet::get_full_ms_list() const{
    module_set_list_t ret;
    ret.push_back(maximal_module_set_);
    return ret;
  }

  void IncrementalModuleSet::reset(const module_set_list_t &mss){
    ms_to_visit_ = mss;
  }

  void IncrementalModuleSet::reset(){
    ms_to_visit_.clear();
    ms_to_visit_.push_back(maximal_module_set_);
  }

} // namespace ch
} // namespace hydla
