#ifndef _INCLUDED_HYDLA_ASK_COLLECTOR_H_
#define _INCLUDED_HYDLA_ASK_COLLECTOR_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "DefaultTreeVisitor.h"
#include "ModuleSetContainer.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {


typedef boost::shared_ptr<hydla::hierarchy::ModuleSet>           module_set_sptr;
typedef hydla::hierarchy::ModuleSetContainer                     module_set_container_t;
typedef boost::shared_ptr<module_set_container_t>  module_set_container_sptr;
typedef hydla::hierarchy::ModuleSetContainer::module_set_list_t  module_set_list_t;
typedef boost::shared_ptr<const hydla::hierarchy::ModuleSet>    module_set_const_sptr;
typedef boost::shared_ptr<hydla::hierarchy::ModuleSetContainer> module_set_container_sptr;

/**
 * askノードを集めるビジタークラス
 */
class AskCollector : public symbolic_expression::DefaultTreeVisitor {
public:
  AskCollector(const module_set_sptr& module_set);

  virtual ~AskCollector();

  /** 
   * askノードを集める
   *
   * @param expanded_always  展開済みalwaysノードの集合
   *                           （askの中にあったalwaysが展開されたもの）
   * @param negative_asks    ガード条件がエンテール不可能なaskノードの集合
   * @param positive_asks    ガード条件がエンテール可能なaskノードの集合
   */
  void collect_ask(always_set_t* expanded_always,                   
                   const positive_asks_t*   positive_asks,
                   const negative_asks_t*   negative_asks,
                   ask_set_t*        unknown_asks);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Tell> node);
  
  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Always> node);
  
private:
  typedef std::set<boost::shared_ptr<hydla::symbolic_expression::Always> >   visited_always_t;

  /// 収集をおこなう対象の制約モジュール集合
  module_set_sptr          module_set_; 

  always_set_t*       expanded_always_;                   
  
  /// 制約ストアと矛盾するaskのリスト
  const negative_asks_t*         negative_asks_;

  /// 有効となっているaskのリスト
  const positive_asks_t*         positive_asks_;
  
  /// まだ導出の可否が不明なaskのリス
  ask_set_t*              unknown_asks_;
  
  /// 有効になったaskの中かどうか（出力用）
  bool                in_positive_ask_;
  
  /// 探索したalwaysノードのリスト
  visited_always_t   visited_always_;

  always_set_t  new_expanded_always_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_ASK_COLLECTOR_H_
