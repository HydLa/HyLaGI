#pragma once

#include <boost/function.hpp>
#include <list>

#include "ModuleSet.h"

namespace hydla {
namespace hierarchy {

class ModuleSetContainer {
public:

  typedef std::set<ModuleSet> module_set_set_t;

  ModuleSetContainer() 
  {}
  ModuleSetContainer(ModuleSet m);
  
  virtual ~ModuleSetContainer()
  {}

  /**
   * 集合の集合のダンプ
   */
  virtual std::ostream& dump(std::ostream& s) const = 0;

  /**
   * 要素数最大であるモジュール集合を得る
   */
  virtual ModuleSet get_max_module_set() const;

  /**
   * 現在の注目ノードを得る
   */
  virtual ModuleSet get_module_set() const;
  
  /**
   * 次に探索すべきモジュール集合が存在しなければfalseを返す．
   */
  bool has_next(){ return !ms_to_visit_.empty(); }
  
  virtual module_set_set_t get_full_ms_list() const;
  
  /**
   * 採用されていないモジュールの集合を返す
   */
  virtual ModuleSet unadopted_module_set();

  virtual ModuleSet get_module_set(){ return *ms_to_visit_.rbegin(); }

  /**
   * そのノードと子ノードを以降探索しないようにする
   */
  virtual void remove_included_ms_by_current_ms();
  
  /**
   * mark nodes which include given module_set
   */
  virtual void generate_new_ms(const module_set_set_t& mms, const ModuleSet& ms);
  
  /**
   * 探索すべきモジュール集合の集合を初期化し，注目する集合を最初に戻す．
   */
  virtual void reset();
  
  /**
   * 与えられたモジュール集合の集合は探索する必要が無いものとし，
   * その上で最初に探索すべきモジュール集合に注目する．
   */
  virtual void reset(const module_set_set_t &mss);
  
  protected:
  module_set_set_t full_module_set_set_;
  module_set_set_t ms_to_visit_;
  /// a module set which has all modules
  ModuleSet maximal_module_set_;
};

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m);

} // namespace hierarchy
} // namespace hydla
