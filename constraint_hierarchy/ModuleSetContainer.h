#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_

#include <boost/function.hpp>
#include <set>

#include "ModuleSet.h"

namespace hydla {
namespace ch {

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
  virtual module_set_sptr get_module_set() const;
  
  /**
   * 探索する必要の無いモジュール集合の集合を得る
   */
  std::set<module_set_sptr> get_visited_module_sets() const;
  
  /**
   * 次に探索すべきモジュール集合に進む
   * 存在しなければfalseを返す．
   */
  virtual bool go_next();
  
  /**
   * そのノードを探索済みとし，以降探索しないようにする
   */
  virtual void mark_current_node();

  /**
   * そのノードと子ノードを以降探索しないようにする
   */
  virtual void mark_nodes() = 0;
  
  /**
   * 探索すべきモジュール集合の集合を初期化し，注目する集合を最初に戻す．
   */
  virtual void reset();
  
  
  /**
   * 与えられたモジュール集合の集合は探索する必要が無いものとし，
   * その上で最初に探索すべきモジュール集合に注目する．
   */
  virtual void reset(const std::set<module_set_sptr> &mss);
  
  protected:
  module_set_list_t module_set_list_;
  module_set_set_t  visited_module_sets_;
  module_set_list_t::iterator current_module_set_;
};

std::ostream& operator<<(std::ostream& s, const ModuleSetContainer& m);

} // namespace ch
} // namespace hydla

#endif // _INCLUDED_HTDLA_CH_MODULE_SET_CONTAINER_H_
