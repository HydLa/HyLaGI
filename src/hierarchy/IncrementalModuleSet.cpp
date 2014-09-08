#include "IncrementalModuleSet.h"
#include "TreeInfixPrinter.h"
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
  parents_data_ = im.parents_data_;
  children_data_ = im.children_data_;
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
      if(ms->find(*p_it) == ms->end()) ms->add_module(*p_it);
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
std::vector<module_set_sptr> IncrementalModuleSet::get_removable_module_sets(module_set_sptr current_ms, const ModuleSet& ms)
{
  HYDLA_LOGGER_DEBUG("%% candidate modules : ", ms.get_name(), "\n");
  // msは矛盾集合
  std::vector<module_set_sptr> removable;

  for( auto it : ms ){
    // if it is a required constraint, do nothing for it
    if(parents_data_[it] == NULL) continue;
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
      if(children_data_[roop_it] != NULL){
        for( auto wmit : *children_data_[roop_it] ){
	  // wmit is a module which is weaker than roop_it
	  // if wmit is included by current module set
          if(current_ms->find(wmit) != current_ms->end()){
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
  parents_data_.insert(
    parallel_module_set_list.parents_data_.begin(),
    parallel_module_set_list.parents_data_.end()
  );

  children_data_.insert(
    parallel_module_set_list.children_data_.begin(),
    parallel_module_set_list.children_data_.end()
  );

  // 今まで出現したすべてのモジュールの集合を保持
  add_maximal_module_set(parallel_module_set_list.maximal_module_set_);
}

void IncrementalModuleSet::add_required_parallel(IncrementalModuleSet& parallel_module_set_list) 
{
  parents_data_.insert(
    parallel_module_set_list.parents_data_.begin(),
    parallel_module_set_list.parents_data_.end()
  );

  children_data_.insert(
    parallel_module_set_list.children_data_.begin(),
    parallel_module_set_list.children_data_.end()
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

  parents_data_.insert(
    weak_module_set_list.parents_data_.begin(),
    weak_module_set_list.parents_data_.end()
  );

  children_data_.insert(
    weak_module_set_list.children_data_.begin(),
    weak_module_set_list.children_data_.end()
  );

  module_set_sptr new_ms(new ModuleSet());
  module_set_sptr against_max(weak_module_set_list.maximal_module_set_);
  // thisに含まれるすべてのモジュールが
  // weak_module_setに含まれるすべてのモジュールよりも強いという情報を保持
  for(; this_it != this_end; ++this_it){
    for(module_list_const_iterator tmp_it = p_it; tmp_it != p_end; ++tmp_it){
      if(parents_data_[*tmp_it] == NULL) parents_data_[*tmp_it] = module_set_sptr(new ModuleSet());
      parents_data_[*tmp_it]->add_module(*this_it);
      if(children_data_[*this_it] == NULL) children_data_[*this_it] = module_set_sptr(new ModuleSet());
      children_data_[*this_it]->add_module(*tmp_it);
    }
  }
  add_maximal_module_set(weak_module_set_list.maximal_module_set_);
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
  node_relations_data_t::const_iterator it = parents_data_.begin();
  node_relations_data_t::const_iterator end = parents_data_.end();

  s << "***** parents_data_ *****" << std::endl;
  while(it!=end){
    it++;
  }
  s << std::endl;
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

/**
 * この mark_nodes は調べているモジュール集合が
 * 矛盾した場合に呼ばれ、新たなモジュール集合を生成する
 * @param mms そのフェーズにおおける極大無矛盾集合
 * @param ms 矛盾の原因となったモジュール集合 
 */
void IncrementalModuleSet::mark_nodes(const module_set_list_t& mms, const ModuleSet& ms){
  HYDLA_LOGGER_DEBUG("%% current module set : ", (*(ms_to_visit_.begin()))->get_name());
  HYDLA_LOGGER_DEBUG("%% inconsistency module set : ", ms.get_name());
  std::string for_debug = "";
  for( auto mit : mms ){
    for_debug += mit->get_name() + " , ";
  }
  HYDLA_LOGGER_DEBUG("%% maximal consistent module set : ", for_debug);
  // 探索対象にmsを含むモジュール集合が存在する間この関数を再帰的に呼び出すためのフラグ
//  bool recursive = false;
  // 制約の優先度を崩さない範囲で除去可能なモジュールを要素として持つ集合を得る
  // 結果用のモジュール集合の集合
  module_set_list_t add, remain;
  for( auto lit : ms_to_visit_ ){
    // 探索対象のモジュール集合がmsを含んでいる場合
    // そのモジュール集合は矛盾するため
    // 新たなモジュール集合を生成する
    if(lit->including(ms)){
      // get vector of removable module set
      std::vector<module_set_sptr> rm = get_removable_module_sets(lit,ms);
      for(auto removable_it : rm){
        module_set_sptr removed(new ModuleSet());
        // remove removable module set of lit.
        for( auto rm_it : *removable_it ){
          // make new module set
	  // The set has no module which is in removable module set
          module_set_sptr new_ms(new ModuleSet());
          for( auto mit : *lit ){
            if(removable_it->find(mit) == removable_it->end()) new_ms->add_module(mit);
          }
          bool checked = false;
          for( auto cit : ms_to_visit_ ){
	    if(cit->including(*new_ms) && new_ms->including(*cit)){
              // 生成されたモジュール集合がすでに探索対象にある場合checkedをtrueにしておく
              checked = true;
              break;
            }
          }
          for( auto cit : add ){
	    if(cit->including(*new_ms) && new_ms->including(*cit)){
              // 生成されたモジュール集合がすでに探索対象にある場合checkedをtrueにしておく
              checked = true;
              break;
            }
          }
          for( auto cit : mms ){
	    // 生成されたモジュール集合が極大無矛盾集合に包含されている場合checkedをtrueにしておく
            if(cit->including(*new_ms)){
              checked = true;
              break;
	    }
          }
         // checkedがtrueでない場合、生成したモジュール集合を探索対象に追加
	  if(!checked){
            add.push_back(new_ms);
	    // 新たに生成したモジュール集合がまだmsを含んでいる場合再帰的にこの関数を呼ぶ
//            if(new_ms->including(ms)) recursive = true;
            HYDLA_LOGGER_DEBUG("%% new ms : ", new_ms->get_name());
	  }
        }
      }	
    }else{
      // 探索対象がmsを含んでいない場合そのまま残しておく
      remain.push_back(lit);
    }
  }
  // 新たに生成したモジュール集合と残しておいたモジュール集合をマージ
  remain.insert(remain.end(),add.begin(),add.end());
  // マージした集合の集合を探索対象とする
  ms_to_visit_ = remain;
//  if(recursive) mark_nodes(mms,ms);
  // 生成処理がすべて終了したら探索対象を要素数が多い順にソートする
//  else 
  sort(ms_to_visit_.begin(), ms_to_visit_.end(), ModuleSetComparator());
}

/// 最も要素数の多いモジュール集合を返す
  module_set_sptr IncrementalModuleSet::get_max_module_set() const{
    return maximal_module_set_;
  }

/// 探索対象の初期状態を返す
  IncrementalModuleSet::module_set_list_t IncrementalModuleSet::get_full_ms_list() const{
    module_set_list_t ret;
    ret.push_back(maximal_module_set_);
    return ret;
  }

// 探索対象を引数のmssが示す状態にする
  void IncrementalModuleSet::reset(const module_set_list_t &mss){
    ms_to_visit_ = mss;
  }

// 探索対象を初期状態に戻す
  void IncrementalModuleSet::reset(){
    ms_to_visit_.clear();
    ms_to_visit_.push_back(maximal_module_set_);
  }

} // namespace hierarchy
} // namespace hydla
