#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_

#include <vector>

#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace hierarchy {

/**
 * 解候補モジュール集合の集合をリスト構造で表すクラス
 * 解候補モジュール集合の集合を導出するアルゴリズムは
 * 「制約階層によるハイブリッドシステムのモデリング手法(JSSST2009)」
 * 参照のこと
 *
 */
class ModuleSetList : public ModuleSetContainer {
public:
  typedef std::vector<module_set_sptr> module_set_list_t;

  ModuleSetList();
  ModuleSetList(module_set_sptr m);
  virtual ~ModuleSetList();

  /**
   * 並列合成として集合を合成する
   */
  void add_parallel(ModuleSetList& parallel_module_set_list);

  /**
   * 並列合成として集合を合成する（required制約扱い）
   */
  void add_required_parallel(ModuleSetList& parallel_module_set_list);
  
  /**
   * 弱合成として集合を合成する
   */
  void add_weak(ModuleSetList& weak_module_set_list);

  /**
   * 集合の集合(このクラス)の名前
   */ 
  std::string get_name() const;

  /**
   * 集合の集合のダンプ
   */
  virtual std::ostream& dump(std::ostream& s) const;

  /**
   * 名前表現によるダンプ
   */
  std::ostream& dump_node_names(std::ostream& s) const;
  
  /**
   * ツリー表現によるダンプ
   */
  std::ostream& dump_node_trees(std::ostream& s) const;

  void mark_nodes();
};

} // namespace hierarchy
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_
