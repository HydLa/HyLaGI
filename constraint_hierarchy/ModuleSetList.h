#ifndef _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_
#define _INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_

#include <vector>

#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ModuleSetTester.h"

namespace hydla {
namespace ch {

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
  ~ModuleSetList();

  /**
   * 並列合成として集合を合成する
   */
  void add_parallel(ModuleSetList& parallel_module_set_list);
  
  /**
   * 弱合成として集合を合成する
   */
  void add_weak(ModuleSetList& weak_module_set_list);

  /**
   * 集合を出力する
   */
  std::ostream& dump(std::ostream& s);

  /**
   * 極大な制約モジュール集合を無矛盾なものが見つかるまでためす
   */
  virtual bool dispatch(ModuleSetTester* tester, int threads = 1);

private:
  module_set_list_t module_set_list_;
};

std::ostream& operator<<(std::ostream& s, ModuleSetList& m);

} // namespace ch
} // namespace hydla

#endif //_INCLUDED_HTDLA_CH_MODULE_SET_LIST_H_
