#ifndef _INCLUDED_HTDLA_CH_INCREMENTAL_MODULE_SET_H_
#define _INCLUDED_HTDLA_CH_INCREMENTAL_MODULE_SET_H_

#include <vector>
#include <map>
#include "ModuleSet.h"
#include "ModuleSetContainer.h"

namespace hydla {
namespace ch {

/**
 * 解候補モジュール集合の集合をインクリメンタルに生成していくクラス
 */
class IncrementalModuleSet : public ModuleSetContainer {
public:

typedef enum{
  WEAKER_THAN,
  STRONGER_THAN,
  PARALLEL,
  REQUIRED_PARALLEL,
}NodeRelation;

  typedef ModuleSet::module_t module_t;
  typedef ModuleSet::module_list_const_iterator module_list_const_iterator;

  //  typedef std::vector<module_set_sptr> module_set_list_t;
  typedef std::pair<NodeRelation,module_set_sptr> str_ms_pair_t;
  typedef std::vector<str_ms_pair_t> str_ms_list_t;
  typedef std::map<module_t,str_ms_list_t> m_str_ms_t;
  //  typedef std::map<std::string,str_ms_list_t> m_str_ms_t;

  
  IncrementalModuleSet();
  IncrementalModuleSet(module_set_sptr m);
  IncrementalModuleSet(const IncrementalModuleSet& im);
  virtual ~IncrementalModuleSet();

  /**
   * 今までに出現したモジュールのセットを作る
   */
  void add_maximal_module_set(module_set_sptr ms);

  /**
   * msから取り除くことのできるモジュールの集合を返す
   */
  module_set_sptr get_removable_module_set(const ModuleSet& ms);

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

  virtual void mark_nodes();

  /**
   * ms:矛盾するモジュールセット
   * ms_to_visit_内のmsを包含するモジュールセットを削除し、
   * ms_to_visit_の先頭モジュールセットからmsを包含しないモジュールセットで
   * 要素が一つ少ないものをms_to_visit_に追加する
   */
  virtual void mark_nodes(const module_set_list_t& mss, const ModuleSet& ms);

  virtual void reset(const module_set_list_t &mss);

  virtual void reset();

  virtual module_set_sptr get_max_module_set() const;

  virtual module_set_list_t get_full_ms_list() const;

private:

  /**
   * あるモジュールの親を辿って行ったときにどのようなノードが出現するかの情報
   * parents_data_に情報がないモジュールは無条件で除ける
   * (モジュール、(記号ID、相手モジュールセット)の配列)
   */ 
  m_str_ms_t parents_data_;
  module_set_sptr required_ms_;
// 現在までに出現したすべてのモジュールを要素とする集合
  module_set_sptr maximal_module_set_;

  bool formated_;
};

} // namespace ch
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_INCREMENTAL_MODULE_SET_H_
