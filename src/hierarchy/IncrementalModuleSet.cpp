#include "IncrementalModuleSet.h"
#include "TreeInfixPrinter.h"
#include "../common/Timer.h"
#include "../common/Logger.h"

#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace hydla {
namespace hierarchy {

IncrementalModuleSet::IncrementalModuleSet()
{}

IncrementalModuleSet::IncrementalModuleSet(module_set_sptr m) :
maximal_module_set_(m)
{}

IncrementalModuleSet::IncrementalModuleSet(const IncrementalModuleSet& im)
{
  stronger_modules_ = im.stronger_modules_;
  weaker_modules_ = im.weaker_modules_;
  required_ms_ = im.required_ms_;
  maximal_module_set_ = im.maximal_module_set_;
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
      ms->add_module(*p_it);
  }
  // maximal_module_set_の更新
  maximal_module_set_ = ms;


  // リスト内で優先度が低いモジュールはそれより優先度が高いモジュールより必ず前に来るようにする
}

/**
 * 対象のモジュール集合から除去可能なモジュールの集合を返す
 * @param current_ms 対象のモジュール集合 
 * @param ms 矛盾の原因となったモジュール集合
 */
std::vector<module_set_sptr> IncrementalModuleSet::get_removable_module_sets(module_set_sptr current_ms, const module_set_sptr ms)
{
  HYDLA_LOGGER_DEBUG("%% candidate modules : ", ms->get_name(), "\n");
  // msは矛盾集合
  std::vector<module_set_sptr> removable;

  for( auto it : *ms ){
    // if it is a required constraint, do nothing for it
    if(stronger_modules_[it] == NULL) continue;
    // one of the removable module set
    module_set_sptr removable_for_it(new ModuleSet());
    // a vector which has all modules which is weaker than it
    std::vector<module_t> childs;
    childs.push_back(it);

    while(childs.size() > 0){
      // pop the top of childs
      auto roop_it = childs.front();
      removable_for_it->add_module(roop_it);
      // to prevent recheck, erase childs front
      childs.erase(childs.begin());
      // if there are modules which is weaker than roop_it
      if(weaker_modules_[roop_it] != NULL){
        for( auto wmit : *weaker_modules_[roop_it] ){
	  // wmit is a module which is weaker than roop_it
	  // if wmit is included by current module set
          if(current_ms->find(wmit) != current_ms->end() && roop_it != wmit){
	    // push wmit to childs
            childs.push_back(wmit);
          }
        }
      }
    }

    removable.push_back(removable_for_it);
  }

  // string for debug
  std::string str = "";
  for(auto it : removable){
    str += it->get_name();
    str += " , ";
  }
  HYDLA_LOGGER_DEBUG("%% removable modules : ", str, "\n");
  return removable;
}

void IncrementalModuleSet::add_parallel(IncrementalModuleSet& parallel_module_set_list) 
{
  stronger_modules_.insert(
    parallel_module_set_list.stronger_modules_.begin(),
    parallel_module_set_list.stronger_modules_.end()
  );

  weaker_modules_.insert(
    parallel_module_set_list.weaker_modules_.begin(),
    parallel_module_set_list.weaker_modules_.end()
  );

  // 今まで出現したすべてのモジュールの集合を保持
  add_maximal_module_set(parallel_module_set_list.maximal_module_set_);
}

void IncrementalModuleSet::add_required_parallel(IncrementalModuleSet& parallel_module_set_list) 
{
  stronger_modules_.insert(
    parallel_module_set_list.stronger_modules_.begin(),
    parallel_module_set_list.stronger_modules_.end()
  );

  weaker_modules_.insert(
    parallel_module_set_list.weaker_modules_.begin(),
    parallel_module_set_list.weaker_modules_.end()
  );
  
  // 今まで出現したすべてのモジュールの情報を保持
  add_maximal_module_set(parallel_module_set_list.maximal_module_set_);
}

void IncrementalModuleSet::add_weak(IncrementalModuleSet& weak_module_set_list) 
{
  // weak_module_set_list << this
  module_list_const_iterator p_it = 
    weak_module_set_list.maximal_module_set_->begin();
  module_list_const_iterator p_end =
    weak_module_set_list.maximal_module_set_->end();
  module_list_const_iterator this_it = maximal_module_set_->begin();
  module_list_const_iterator this_end = maximal_module_set_->end();

  stronger_modules_.insert(
    weak_module_set_list.stronger_modules_.begin(),
    weak_module_set_list.stronger_modules_.end()
  );

  weaker_modules_.insert(
    weak_module_set_list.weaker_modules_.begin(),
    weak_module_set_list.weaker_modules_.end()
  );

  module_set_sptr new_ms(new ModuleSet());
  module_set_sptr against_max(weak_module_set_list.maximal_module_set_);
  // thisに含まれるすべてのモジュールが
  // weak_module_setに含まれるすべてのモジュールよりも強いという情報を保持
  for(; this_it != this_end; ++this_it){
    for(module_list_const_iterator tmp_it = p_it; tmp_it != p_end; ++tmp_it){
      if(stronger_modules_[*tmp_it] == NULL) stronger_modules_[*tmp_it] = module_set_sptr(new ModuleSet());
      stronger_modules_[*tmp_it]->add_module(*this_it);
      if(weaker_modules_[*this_it] == NULL) weaker_modules_[*this_it] = module_set_sptr(new ModuleSet());
      weaker_modules_[*this_it]->add_module(*tmp_it);
    }
  }
  add_maximal_module_set(weak_module_set_list.maximal_module_set_);
}

std::ostream& IncrementalModuleSet::dump(std::ostream& s) const
{
  return s;
}

std::ostream& IncrementalModuleSet::dump_node_names(std::ostream& s) const
{
  return s;
}

std::ostream& IncrementalModuleSet::dump_node_trees(std::ostream& s) const
{
  return s;
}

/**
 * この mark_nodes は調べているモジュール集合が
 * 無矛盾だった場合に呼ばれ、
 * そのモジュール集合が包含しているモジュール集合を
 * 探索対象から外す
 */
void IncrementalModuleSet::mark_nodes(){
  /// current は現在のモジュール集合
  module_set_sptr current = ms_to_visit_.front();
  module_set_list_t::iterator lit = ms_to_visit_.begin();
  while(lit!=ms_to_visit_.end()){
    /**
     * ms_to_visit_内のモジュール集合で
     * currentが包含するモジュール集合を削除
     */
    if(current->including(**lit)){
      lit = ms_to_visit_.erase(lit);
    }
    else lit++;
  }
}

bool IncrementalModuleSet::check_same_ms_generated(module_set_list_t new_mss, module_set_sptr ms)
{
  for(auto new_ms : new_mss){
    if(new_ms->including(*ms) && ms->including(*new_ms)) return true;
  }
  return false;
}

void IncrementalModuleSet::update_by_new_mss(module_set_list_t new_mss)
{
  ms_to_visit_.clear();
  for(auto new_ms : new_mss){
    ms_to_visit_.push_back(module_set_sptr(new ModuleSet(*required_ms_,*new_ms)));
  }
  sort(ms_to_visit_.begin(), ms_to_visit_.end(), ModuleSetComparator());
}

void IncrementalModuleSet::generate_required_ms()
{
  required_ms_ = module_set_sptr(new ModuleSet());
  for(auto ms : *maximal_module_set_){
    if(stronger_modules_[ms] == NULL) required_ms_->add_module(ms);
  }
}

/**
 * この mark_nodes は調べているモジュール集合が
 * 矛盾した場合に呼ばれ、新たなモジュール集合を生成する
 * @param mms そのフェーズにおおける極大無矛盾集合
 * @param ms 矛盾の原因となったモジュール集合 
 */
void IncrementalModuleSet::mark_nodes(const module_set_list_t& mcss, const ModuleSet& ms){
  if(required_ms_ == NULL) generate_required_ms();
  HYDLA_LOGGER_DEBUG("%% current module set : ", (*(ms_to_visit_.begin()))->get_name());
  HYDLA_LOGGER_DEBUG("%% inconsistency module set : ", ms.get_name());
  std::string for_debug = "";
  for( auto mit : mcss ){
    for_debug += mit->get_name() + " , ";
  }
  HYDLA_LOGGER_DEBUG("%% maximal consistent module set : ", for_debug);
  // 制約の優先度を崩さない範囲で除去可能なモジュールを要素として持つ集合を得る
  // 結果用のモジュール集合の集合
  module_set_list_t new_mss;
  module_set_sptr inconsistent_ms = module_set_sptr(new ModuleSet(ms));
  inconsistent_ms->erase(*required_ms_);
  for( auto lit : ms_to_visit_ ){
    lit->erase(*required_ms_);
    // 探索対象のモジュール集合がmsを含んでいる場合
    // そのモジュール集合は矛盾するため
    // 新たなモジュール集合を生成する
    if(lit->including(*inconsistent_ms)){
      // get vector of removable module set
      std::vector<module_set_sptr> rm = get_removable_module_sets(lit,inconsistent_ms);
      for(auto removable_module_set : rm){
        // remove removable module set of lit.
        // make new module set
        // The set has no module which is in removable module set
        module_set_sptr new_ms(lit);
        new_ms->erase(*removable_module_set);
        // check weather a same module set was generated
        bool checked = check_same_ms_generated(new_mss, new_ms);
        // check weather the new_ms is included by maximal consistent module sets.
        if(!checked){
          for( auto mcs : mcss ){
           // 生成されたモジュール集合が極大無矛盾集合に包含されている場合checkedをtrueにしておく
            if(mcs->including(*new_ms)){
              checked = true;
              break;
            }
          }
        }
       // checkedがtrueでない場合、生成したモジュール集合を探索対象に追加
        if(!checked){
          new_mss.push_back(new_ms);
          HYDLA_LOGGER_DEBUG("%% new ms : ", new_ms->get_name());
        }
      }	
    }else{
      // 探索対象がmsを含んでいない場合そのまま残しておく
      if(!check_same_ms_generated(new_mss, lit)){
        new_mss.push_back(lit);
      }
    }
  }
  // update ms_to_visit_ by generated module sets
  update_by_new_mss(new_mss);
}

/// 最も要素数の多いモジュール集合を返す
  module_set_sptr IncrementalModuleSet::get_max_module_set() const{
    return maximal_module_set_;
  }

/// 探索対象の初期状態を返す
  IncrementalModuleSet::module_set_list_t IncrementalModuleSet::get_full_ms_list() const{
    module_set_list_t ret;
    ret.push_back(module_set_sptr(new ModuleSet(*maximal_module_set_)));
    return ret;
  }

// 探索対象を引数のmssが示す状態にする
  void IncrementalModuleSet::reset(const module_set_list_t &mss){
    ms_to_visit_ = mss;
  }

// 探索対象を初期状態に戻す
  void IncrementalModuleSet::reset(){
    ms_to_visit_.clear();
    ms_to_visit_.push_back(module_set_sptr(new ModuleSet(*maximal_module_set_)));
  }

} // namespace hierarchy
} // namespace hydla
