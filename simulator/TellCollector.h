#ifndef _INCLUDED_HYDLA_TELL_COLLECTOR_H_
#define _INCLUDED_HYDLA_TELL_COLLECTOR_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "TreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * tellノードを集めるビジタークラス
 */
class TellCollector : public parse_tree::TreeVisitor {
public:
  TellCollector();
  virtual ~TellCollector();

  /** 
   * tellノードを集める
   *
   * @param expanded_always  展開済みalwaysノードの集合
   *                           （askの中にあったalwaysが展開されたもの）
   * @param collected_tells  集められたtellノードの集合
   * @param positive_asks    ガード条件がエンテール可能なaskノードの集合
   */
  void collect_tell(module_set_t*      ms,
                    expanded_always_t* expanded_always,                   
                    collected_tells_t* collected_tells,
                    positive_asks_t*   positive_asks);

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);

  // 論理積
  virtual void visit(boost::shared_ptr<hydla::parse_tree::LogicalAnd> node);
  
  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);

private:
  expanded_always_t* expanded_always_;                 
  collected_tells_t* collected_tells_;
  positive_asks_t*   positive_asks_;

  /// askノードの子ノードかどうか
  bool               in_ask_;

  /// 展開済みalwaysノードのリストからの探索かどうか
  bool               in_expanded_always_;

  /// 探索したalwaysノードのリスト
  visited_always_t   visited_always_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_TELL_COLLECTOR_H_
