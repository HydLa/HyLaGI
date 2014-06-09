#pragma once

#include <boost/function.hpp>
#include <list>

#include "ModuleSet.h"

namespace hydla {
namespace hierarchy {

class ModuleSetContainer {
public:

  typedef std::set<module_set_sptr> module_set_set_t;
  typedef std::vector<module_set_sptr> module_set_list_t;

  ModuleSetContainer() 
  {}
  ModuleSetContainer(module_set_sptr m);
  
  virtual ~ModuleSetContainer()
  {}

  /**
   * 集合の集合のダンプ
   */
  virtual std::ostream& dump(std::ostream& s) const = 0;

  /**
   * 要素数最大であるモジュール集合を得る
   */
  virtual module_set_sptr get_max_module_set() const;

  /**
   * 現在の注目ノードを得る
   */
  virtual module_set_sptr get_reverse_module_set() const;

  /**
   * 現在の注目ノードを得る
   */
  virtual module_set_sptr get_module_set() const;
  
  /**
   * 探索すべきモジュール集合の集合を得る
   */
  module_set_list_t get_ms_to_visit() const;

  /**
   * 現在のモジュール集合を解候補モジュール集合の集合から取り除く
   */
  virtual bool eliminate_current_node();

  /**
   * 次に探索すべきモジュール集合に進む
   * 存在しなければfalseを返す.
   * 探索は小さいモジュール集合から開始する.
   */
  virtual bool reverse_go_next();
  
  /**
   * 次に探索すべきモジュール集合に進む
   * 存在しなければfalseを返す．
   */
  virtual bool go_next();
  
  virtual module_set_list_t get_full_ms_list() const;
  
  /**
   * そのノードを探索済みとし，以降探索しないようにする
   */
  virtual void mark_r_current_node();

  /**
   * そのノードを探索済みとし，以降探索しないようにする
   */
  virtual void mark_current_node();

  /**
   * そのノードと子ノードを以降探索しないようにする
   */
  virtual void mark_nodes() = 0;
  
  /**
   * mark nodes which include given module_set
   */
  virtual void mark_nodes(const ModuleSet& ms);

  virtual void mark_nodes(const module_set_list_t& mms, const ModuleSet& ms);
  
  /**
   * 探索すべきモジュール集合の集合を初期化し，注目する集合を最後にする.
   */
  virtual void reverse_reset();

  /**
   * 探索すべきモジュール集合の集合を初期化し，注目する集合を最初に戻す．
   */
  virtual void reset();
  
  
  /**
   * 与えられたモジュール集合の集合は探索する必要が無いものとし，
   * その上で最初に探索すべきモジュール集合に注目する．
   */
  virtual void reset(const module_set_list_t &mss);
  
  /**
   * 現在のモジュール集合を
   * 包含するモジュール集合を探索済みとする
   */
  virtual void mark_super_module_set();

  protected:
  module_set_list_t module_set_list_;
  module_set_list_t ms_to_visit_, r_ms_to_visit_;
};

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m);

} // namespace hierarchy
} // namespace hydla
