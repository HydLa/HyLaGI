#include "IncrementalModuleSet.h"
#include "TreeInfixPrinter.h"
#include "../common/Timer.h"
#include "../common/Logger.h"

#include <iostream>
#include <algorithm>

namespace hydla {
namespace hierarchy {

IncrementalModuleSet::IncrementalModuleSet()
{}

IncrementalModuleSet::IncrementalModuleSet(ModuleSet ms):
  ModuleSetContainer(ms)
{}

IncrementalModuleSet::IncrementalModuleSet(const IncrementalModuleSet& im)
{
  stronger_modules_ = im.stronger_modules_;
  weaker_modules_ = im.weaker_modules_;
  maximal_module_set_ = im.maximal_module_set_;
}

IncrementalModuleSet::~IncrementalModuleSet()
{}

/**
 * 対象のモジュール集合から除去可能なモジュールの集合を返す
 * @param current_ms 対象のモジュール集合 
 * @param ms 矛盾の原因となったモジュール集合
 */
std::set<ModuleSet> IncrementalModuleSet::get_removable_module_sets(ModuleSet &current_ms, const ModuleSet &ms)
{
  HYDLA_LOGGER_DEBUG("%% candidate modules : ", ms.get_name(), "\n");
  HYDLA_LOGGER_DEBUG("%% current modules : ", current_ms.get_name(), "\n");
  // msは矛盾集合
  std::set<ModuleSet> removable;
  std::set<ModuleSet> ret;

  for ( auto it : ms )
  {
    // Check if "it" is required module 
    if (!stronger_modules_.count(it)) continue;
    // Check if "it" is included in current module set 
    if (current_ms.find(it) != current_ms.end()) continue;
    ModuleSet removable_modules;
    // insert "it" to empty set
    removable_modules.add_module(it);
    // insert weaker modules than "it" to "removable_modules"
    removable_modules.insert(weaker_modules_[it]);
    // "removable_modules" = { M | M = "it" \/ M << "it" }
    removable.insert(removable_modules);
  }

  // select minimal module sets
  for ( auto it : removable )
  {
    bool is_minimal = true;
    for ( auto it2 : removable )
    {
      // If it == it2 then continue roop
      if (it.including(it2) && it2.including(it)) continue;
      // Check if it include other module sets in removable 
      if (it.including(it2))
      { 
        is_minimal = false;
        break;
      }
    }
    // "ret" are minimal module sets of "removable"
    if (is_minimal) ret.insert(it);
  }

  // string for debug
  std::string str = "";
  for (auto it : ret)
  {
    str += it.get_name();
    str += " , ";
  }
  HYDLA_LOGGER_DEBUG("%% removable modules : ", str, "\n");
  return ret;
}

ModuleSet IncrementalModuleSet::unadopted_module_set()
{
  return *ms_to_visit_.begin();
}


void IncrementalModuleSet::add_parallel(IncrementalModuleSet& parallel_module_set) 
{
  add_order_data(parallel_module_set);

  // 今まで出現したすべてのモジュールの集合を保持
  maximal_module_set_.insert(parallel_module_set.maximal_module_set_);
}

void IncrementalModuleSet::add_weak(IncrementalModuleSet& weak_module_set_list) 
{
  add_order_data(weak_module_set_list);

  // thisに含まれるすべてのモジュールが
  // weak_module_setに含まれるすべてのモジュールよりも強いという情報を保持
  for (auto this_module : maximal_module_set_)
  {
    for (auto weaker_module : weak_module_set_list.maximal_module_set_)
    {
      stronger_modules_[weaker_module].add_module(this_module);
      weaker_modules_[this_module].add_module(weaker_module);
    }
  }

  // 今まで出現したすべてのモジュールの集合を保持
  maximal_module_set_.insert(weak_module_set_list.maximal_module_set_);
}

void IncrementalModuleSet::add_order_data(IncrementalModuleSet& ims)
{
  for (auto sm : ims.stronger_modules_)
  {
    stronger_modules_[sm.first].insert(sm.second);
  }
  for (auto wm : ims.weaker_modules_)
  {
    weaker_modules_[wm.first].insert(wm.second);
  }
}

std::ostream& IncrementalModuleSet::dump_module_sets_for_graphviz(std::ostream& s)
{
  reset();
  s << "digraph module_set { " << std::endl;
  s << "  edge [dir=back];" << std::endl;
  module_set_set_t mss;
  s << "  \"" << get_max_module_set().get_name() << "\"" << std::endl;
  while(has_next())
  {
    ModuleSet tmp = get_max_module_set();
    tmp.erase(unadopted_module_set());
    generate_new_ms(mss, tmp);
    for (auto ms : ms_to_visit_)
    {
      ModuleSet child = get_max_module_set();
      child.erase(ms);
      if (tmp.including(child))
      {
        s << "  \"" << tmp.get_name() << "\" -> \"" << child.get_name() << "\"" << std::endl;
      }
    }
  }
  s << "}" << std::endl;
  reset();
  return s;
}

std::ostream& IncrementalModuleSet::dump_priority_data_for_graphviz(std::ostream& s) const
{
  s << "digraph priority_data { " << std::endl;
  s << "  edge [dir=back];" << std::endl;
  for (auto m : weaker_modules_)
  {
    for (auto wm : m.second)
    {
      s << "  \"" << m.first.first << "\" -> \"" << wm.first << "\";" << std::endl;
    }
  }
  s << "}" << std::endl;
  return s;
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

bool IncrementalModuleSet::check_same_ms_generated(module_set_set_t &new_mss, ModuleSet &ms)
{
  return new_mss.count(ms);
}

void IncrementalModuleSet::update_by_new_mss(module_set_set_t &new_mss)
{
  ms_to_visit_.clear();
  for (auto new_ms : new_mss)
  {
    ms_to_visit_.insert(new_ms);
  }
}

void IncrementalModuleSet::init()
{
}

void IncrementalModuleSet::remove_included_ms_by_current_ms()
{
  /// current は現在のモジュール集合
  ModuleSet current = unadopted_module_set();
  module_set_set_t::iterator lit = ms_to_visit_.begin();
  while(lit != ms_to_visit_.end())
  {
    /**
     * ms_to_visit_内のモジュール集合で
     * currentが包含するモジュール集合を削除
     */
    if (lit->including(current))
    {
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
void IncrementalModuleSet::generate_new_ms(const module_set_set_t& mcss, const ModuleSet& ms)
{
  HYDLA_LOGGER_DEBUG("%% inconsistent module set : ", ms.get_name());
  std::string for_debug = "";
  for (auto mit : mcss)
  {
    for_debug += mit.get_name() + " , ";
  }
  HYDLA_LOGGER_DEBUG("%% maximal consistent module set : ", for_debug);
  // 制約の優先度を崩さない範囲で除去可能なモジュールを要素として持つ集合を得る
  // 結果用のモジュール集合の集合
  module_set_set_t new_mss;
  ModuleSet inconsistent_ms = ms;
  for (auto lit : ms_to_visit_)
  {
    // 探索対象のモジュール集合がmsを含んでいる場合
    // そのモジュール集合は矛盾するため
    // 新たなモジュール集合を生成する
    if (lit.disjoint(inconsistent_ms))
    {
      // get vector of removable module set
      std::set<ModuleSet> rm = get_removable_module_sets(lit,inconsistent_ms);
      for (auto removable_module_set : rm)
      {
        // remove removable module set of lit.
        // make new module set
        // The set has no module which is in removable module set
        ModuleSet new_ms = lit;
        new_ms.insert(removable_module_set);
        // check weather the new_ms is included by maximal consistent module sets.
        bool checked = false;
        for (auto mcs : mcss)
        {
         // 生成されたモジュール集合が極大無矛盾集合に包含されている場合checkedをtrueにしておく
          if (new_ms.including(mcs))
          {
            checked = true;
            break;
          }
        }
       // checkedがtrueでない場合、生成したモジュール集合を探索対象に追加
        if (!checked)
        {
          new_mss.insert(new_ms);
          HYDLA_LOGGER_DEBUG("%% new ms : ", new_ms.get_name());
        }
      }	
    }else{
      // 探索対象がmsを含んでいない場合そのまま残しておく
      if (!check_same_ms_generated(new_mss, lit))
      {
        new_mss.insert(lit);
      }
    }
  }
  // update ms_to_visit_ by generated module sets
  update_by_new_mss(new_mss);
}

/// 最も要素数の多いモジュール集合を返す
  ModuleSet IncrementalModuleSet::get_max_module_set() const{
    ModuleSet ret = maximal_module_set_;
    return ret;
  }

// 探索対象を引数のmssが示す状態にする
  void IncrementalModuleSet::reset(const module_set_set_t &mss)
  {
    ms_to_visit_ = mss;
  }

// 探索対象を初期状態に戻す
  void IncrementalModuleSet::reset()
  {
    ms_to_visit_.clear();
    ms_to_visit_.insert(ModuleSet());
  }

  bool IncrementalModuleSet::is_required(const module_t &m) const{
    return not stronger_modules_.count(m);
  }

} // namespace hierarchy
} // namespace hydla
