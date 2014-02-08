#include "IncrementalModuleSet.h"
#include "TreeInfixPrinter.h"
#include "../common/Logger.h"

#include <iostream>
#include <algorithm>
#include <boost/lambda/lambda.hpp>

using namespace boost::lambda;

namespace hydla {
namespace ch {

IncrementalModuleSet::IncrementalModuleSet()
{}

IncrementalModuleSet::IncrementalModuleSet(module_set_sptr m) :
maximal_module_set_(m)
{}

IncrementalModuleSet::IncrementalModuleSet(const IncrementalModuleSet& im)
{
  parents_data_ = im.parents_data_;
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
module_set_sptr IncrementalModuleSet::get_removable_module_set(module_set_sptr current_ms, const ModuleSet& ms)
{
  // msは矛盾集合
  module_set_sptr removable(new ModuleSet());
  module_set_sptr removed_ms(new ModuleSet());

  // 削除対象の候補となるモジュールの集合
  // 初期値は矛盾の原因となったモジュール集合に含まれるモジュール
  module_set_sptr candidates(new ModuleSet(ms)); 

  module_list_const_iterator it = maximal_module_set_->begin();
  module_list_const_iterator end = maximal_module_set_->end();

  // すでに削除されているモジュールを得る
  for(; it != end; it++){
    if(current_ms->find(*it) == current_ms->end()) removed_ms->add_module(*it);
  }

  it = ms.begin();
  end = ms.end();

  for(; it != end; it++){
    str_ms_list_t::iterator p_it = parents_data_[*it].begin();
    str_ms_list_t::iterator p_end = parents_data_[*it].end();
    for(; p_it != p_end; p_it++){
      for(module_list_const_iterator c_it = current_ms->begin(); c_it != current_ms->end(); c_it++){
        if(p_it->second->find(*(c_it)) != p_it->second->end()){
          switch(p_it->first){
          case STRONGER_THAN:
            // 矛盾の原因となるモジュールよりも優先度が低いモジュールは削除候補に追加する
            if(candidates->find(*(c_it)) == candidates->end()){
              candidates->add_module(*(c_it));
	    }
            break;
          default:
            break; 
          }
	}
      }
    }
  }

  it = candidates->begin();
  end = candidates->end();

  for(; it!=end; it++){
    // parents_data_に含まれているモジュールについて
    str_ms_list_t::iterator p_it = parents_data_[*it].begin();
    str_ms_list_t::iterator p_end = parents_data_[*it].end();
    for(; p_it!=p_end; p_it++){
      // current_msに含まれていないモジュールは関係ないので次の要素のチェック
      if(removed_ms->including(*(p_it->second))) continue;
      switch(p_it->first){
      case WEAKER_THAN:
      case PARALLEL:
        // << の左か , かで削除できる
        removable->add_module(*it);
        break;
      default:
        break;
      }
      break;
    }
    // 最後までチェックできた時も削除できる
    if(p_it==p_end) removable->add_module(*it);
  }
  HYDLA_LOGGER_DEBUG("%% current module set\n", current_ms->get_name());
  HYDLA_LOGGER_DEBUG("%% inconsistency module set\n", ms.get_name());
  HYDLA_LOGGER_DEBUG("%% removable modules\n", removable->get_name(), "\n");
  return removable;
}

void IncrementalModuleSet::add_parallel(IncrementalModuleSet& parallel_module_set_list) 
{
  // parallel_module_set_list , this
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
  // this内のモジュールについてparallel_module_set_listと並列である情報を保持
  for(; this_it != this_end; ++this_it){
    parents_data_[*this_it].push_back(str_ms_pair_t(PARALLEL,against_max));
  }
  // parallel_module_set_list内のモジュールについても同様
  for(; p_it!=p_end; ++p_it){
    parents_data_[*p_it].push_back(str_ms_pair_t(PARALLEL,maximal_module_set_));
    new_ms->add_module(*p_it);
  }
  // 今まで出現したすべてのモジュールの集合を保持
  add_maximal_module_set(new_ms);
}

void IncrementalModuleSet::add_required_parallel(IncrementalModuleSet& parallel_module_set_list) 
{
  // parallel_module_set_list , this   (required)
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
  // thisに含まれるモジュールが
  // parallel_module_setのモジュールと
  // required扱いで並列であるという情報を保持
  for(; this_it != this_end; ++this_it){
    parents_data_[*this_it].push_back(str_ms_pair_t(REQUIRED_PARALLEL,against_max));
  }
  // parallel_module_setのモジュールに関しても上と同様
  for(; p_it!=p_end; ++p_it){
    parents_data_[*p_it].push_back(str_ms_pair_t(REQUIRED_PARALLEL,maximal_module_set_));
    new_ms->add_module(*p_it);
  }
  // 今まで出現したすべてのモジュールの情報を保持
  add_maximal_module_set(new_ms);
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

  module_set_sptr new_ms(new ModuleSet());
  module_set_sptr against_max(weak_module_set_list.maximal_module_set_);
  // thisに含まれるすべてのモジュールが
  // weak_module_setに含まれるすべてのモジュールよりも強いという情報を保持
  for(; this_it != this_end; ++this_it){
    parents_data_[*this_it].push_back(str_ms_pair_t(STRONGER_THAN,against_max));
  }
  // weak_module_setに含まれるすべてのモジュールが
  // thisのすべてのモジュールよりも弱いという情報を保持
  for(; p_it!=p_end; ++p_it){
    parents_data_[*p_it].push_back(str_ms_pair_t(WEAKER_THAN,maximal_module_set_));
    new_ms->add_module(*p_it);
  }
  // 今まで出現したすべてのモジュールの集合を保持しておく
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
  HYDLA_LOGGER_DEBUG("");
  // 探索対象にmsを含むモジュール集合が存在する間この関数を再帰的に呼び出すためのフラグ
  bool recursive = false;
  // 制約の優先度を崩さない範囲で除去可能なモジュールを要素として持つ集合を得る
  // 結果用のモジュール集合の集合
  module_set_list_t add, remain;
  module_set_list_t::iterator lit = ms_to_visit_.begin();
  for(;lit!=ms_to_visit_.end();lit++){
    // 探索対象のモジュール集合がmsを含んでいる場合
    // そのモジュール集合は矛盾するため
    // 新たなモジュール集合を生成する
    if((*lit)->including(ms)){
      module_set_sptr rm = get_removable_module_set(*lit,ms);
      module_list_const_iterator mend = (*lit)->end();
      module_set_sptr removed(new ModuleSet());
      // rm内のモジュールをすべて除くまで繰り返す
      for(module_list_const_iterator rm_it = rm->begin(); rm_it != rm->end(); rm_it++){
        // *litからrm内の要素一つを除き、新たなモジュール集合(new_ms)を生成
        module_list_const_iterator mit = (*lit)->begin();
        module_set_sptr new_ms(new ModuleSet());
        for(; mit!=mend; mit++){
          if((*rm_it) != (*mit))new_ms->add_module(*mit);
        }
        bool checked = false;
	for(module_set_list_t::iterator cit = ms_to_visit_.begin(); cit != ms_to_visit_.end(); cit++){
	  if((*cit)->including(*new_ms) && new_ms->including(**cit)){
            // 生成されたモジュール集合がすでに探索対象にある場合checkedをtrueにしておく
            checked = true;
            break;
          }
        }
        for(module_set_list_t::const_iterator cit = mms.begin(); cit != mms.end(); cit++){
	  // 生成されたモジュール集合が極大無矛盾集合に包含されている場合checkedをtrueにしておく
          if((*cit)->including(*new_ms)){
            checked = true;
            break;
	  }
        }
	// checkedがtrueでない場合、生成したモジュール集合を探索対象に追加
	if(!checked){
          add.push_back(new_ms);
	  // 新たに生成したモジュール集合がまだmsを含んでいる場合再帰的にこの関数を呼ぶ
          if(new_ms->including(ms)) recursive = true;
          HYDLA_LOGGER_DEBUG("%% new ms : ", new_ms->get_name());
	}
      }
    }else{
      // 探索対象がmsを含んでいない場合そのまま残しておく
      remain.push_back(*lit);
    }
  }
  // 新たに生成したモジュール集合と残しておいたモジュール集合をマージ
  remain.insert(remain.end(),add.begin(),add.end());
  // マージした集合の集合を探索対象とする
  ms_to_visit_ = remain;
  if(recursive) mark_nodes(mms,ms);
  // 生成処理がすべて終了したら探索対象を要素数が多い順にソートする
  else sort(ms_to_visit_.begin(), ms_to_visit_.end(), ModuleSetComparator());
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

} // namespace ch
} // namespace hydla
