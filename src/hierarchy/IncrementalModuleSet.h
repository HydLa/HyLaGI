#pragma once

#include <map>
#include "ModuleSetContainer.h"

namespace hydla {
namespace hierarchy {

/**
 * 解候補モジュール集合の集合をインクリメンタルに生成していくクラス
 *
 */
class IncrementalModuleSet : public ModuleSetContainer{
public:

  typedef ModuleSet::module_t module_t;
  typedef ModuleSet::module_list_const_iterator module_list_const_iterator;

  typedef std::map<module_t, ModuleSet> node_relations_data_t;
  
  IncrementalModuleSet();
  IncrementalModuleSet(ModuleSet ms);
  IncrementalModuleSet(const IncrementalModuleSet& im);
  virtual ~IncrementalModuleSet();

  /**
   * @param current_ms あるモジュールを取り除かれる元のモジュール集合
   * @param ms 矛盾の原因となるモジュール集合
   * msを使って取り除くことのできるモジュールの集合を返す
   */
  std::vector<ModuleSet> get_removable_module_sets(ModuleSet &current_ms, const ModuleSet &ms);

  /**
   * 並列合成として集合を合成する
   */
  void add_parallel(IncrementalModuleSet&);

  /**
   * 並列合成として集合を合成する（required制約扱い）
   */
  void add_required_parallel(IncrementalModuleSet& im){ add_parallel(im); }
  
  /**
   * 弱合成として集合を合成する
   */
  void add_weak(IncrementalModuleSet&);

  /**
   * 集合の集合(このクラス)の名前
   */ 
  std::string get_name() const;

  /**
   * dump priority data in dot language
   */
  virtual std::ostream& dump_module_sets_for_graphviz(std::ostream& s);
  
  /**
   * dump priority data in dot language
   */
  virtual std::ostream& dump_priority_data_for_graphviz(std::ostream& s) const;
  
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

  bool has_next(){ return !ms_to_visit_.empty(); }

  ModuleSet get_module_set(){ return *ms_to_visit_.rbegin(); }

  /**
   * @ param ms:矛盾するモジュールセット
   * ms_to_visit_内のmsを包含するモジュールセットを削除し、
   * ms_to_visit_の先頭モジュールセットからmsを包含しないモジュールセットで
   * 要素が一つ少ないものをms_to_visit_に追加する
   */
  virtual void generate_new_ms(const module_set_set_t& mss, const ModuleSet& ms);

  /**
   * 探索対象をmssが示すモジュール集合とする
   */
  virtual void reset(const module_set_set_t &mss);

  /**
   * 探索対象を初期化する
   */
  virtual void reset();

  /**
   * 最大の要素数のモジュール集合を得る
   */
  virtual ModuleSet get_max_module_set() const;

  /**
   * initialize 
   */
  virtual void init();

private:
  /**
   * add << data
   */
  virtual void add_order_data(IncrementalModuleSet&);

  /**
   * check same module set was generated
   */
  virtual bool check_same_ms_generated(module_set_set_t&, ModuleSet&); 

  /**
   * update ms_to_visit_ by generated module sets
   */
  virtual void update_by_new_mss(module_set_set_t&);

  /**
   * stronger_data_[module_t ms] is a map which has stronger modules than ms.
   * children_data_[module_t ms] is a map which has weaker modules than ms.
   */ 
  node_relations_data_t stronger_modules_;
  node_relations_data_t weaker_modules_;
  /// required module set
  ModuleSet required_ms_;
};


} // namespace hierarchy
} // namespace hydla
