#pragma once

#include <map>
#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace hierarchy {

/**
 * 解候補モジュール集合の集合をインクリメンタルに生成していくクラス
 *
 */
class IncrementalModuleSet : public ModuleSetContainer {
public:

  typedef ModuleSet::module_t module_t;
  typedef ModuleSet::module_list_const_iterator module_list_const_iterator;

  typedef std::map<module_t, ModuleSet> node_relations_data_t;
  
  IncrementalModuleSet();
  IncrementalModuleSet(ModuleSet &m);
  IncrementalModuleSet(const IncrementalModuleSet& im);
  virtual ~IncrementalModuleSet();

  /**
   * 今までに出現したモジュールのセットを作る
   */
  void add_maximal_module_set(ModuleSet &ms);


  /**
   * @param current_ms あるモジュールを取り除かれる元のモジュール集合
   * @param ms 矛盾の原因となるモジュール集合
   * msを使って取り除くことのできるモジュールの集合を返す
   */
  std::vector<ModuleSet> get_removable_module_sets(ModuleSet &current_ms, const ModuleSet &ms);

  /**
   * parents_data_内の余分なデータを削除し、
   * requiredなものを分離する
   */
  void format_parents_data();

  /**
   * 並列合成として集合を合成する
   */
  void add_parallel(IncrementalModuleSet& parallel_module_set_list);

  /**
   * 並列合成として集合を合成する（required制約扱い）
   */
  void add_required_parallel(IncrementalModuleSet& parallel_module_set_list);
  
  /**
   * 弱合成として集合を合成する
   */
  void add_weak(IncrementalModuleSet& weak_module_set_list);

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

  void next();


  /**
   * ms_to_visit_内のモジュール集合で
   * 現在のモジュール集合が包含するモジュール集合を
   * ms_to_visit_から外す
   */
  virtual void mark_nodes();

  /**
   * @ param ms:矛盾するモジュールセット
   * ms_to_visit_内のmsを包含するモジュールセットを削除し、
   * ms_to_visit_の先頭モジュールセットからmsを包含しないモジュールセットで
   * 要素が一つ少ないものをms_to_visit_に追加する
   */
  virtual void mark_nodes(const module_set_list_t& mss, const ModuleSet& ms);

  /**
   * 探索対象をmssが示すモジュール集合とする
   */
  virtual void reset(const module_set_list_t &mss);

  /**
   * 探索対象を初期化する
   */
  virtual void reset();

  /**
   * 最大の要素数のモジュール集合を得る
   */
  virtual ModuleSet get_max_module_set() const;

  /**
   * 探索対象の初期状態を返す
   */
  virtual module_set_list_t get_full_ms_list() const;

private:

  /**
   * check same module set was generated
   */
  virtual bool check_same_ms_generated(module_set_list_t, ModuleSet&); 

  /**
   * generate module sets which has only required modules
   */
  virtual void generate_required_ms();

  /**
   * update ms_to_visit_ by generated module sets
   */
  virtual void update_by_new_mss(module_set_list_t);

  /**
   * children_data_[module_t ms] is a map which has weaker modules than ms.
   */ 
  node_relations_data_t stronger_modules_;
  node_relations_data_t weaker_modules_;
  ModuleSet required_ms_;
// 現在までに出現したすべてのモジュールを要素とする集合
  ModuleSet maximal_module_set_;
};


} // namespace hierarchy
} // namespace hydla
