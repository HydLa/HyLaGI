#ifndef _INCLUDED_HYDLA_ASK_COLLECTOR_H_
#define _INCLUDED_HYDLA_ASK_COLLECTOR_H_

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "DefaultTreeVisitor.h"

#include "./Types.h"

namespace hydla {
namespace simulator {

/**
 * askノードを集めるビジタークラス
 */
class AskCollector : public parse_tree::DefaultTreeVisitor {
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
  void collect_ask(expanded_always_t* expanded_always,                   
                   positive_asks_t*   positive_asks,
                   negative_asks_t*   negative_asks,
                   ask_set_t*        unknown_asks);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Tell> node);
  
  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Always> node);
  
private:
  typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >   visited_always_t;

  /// 収集をおこなう対象の制約モジュール集合
  module_set_sptr          module_set_; 

  expanded_always_t*       expanded_always_;                   
  
  /// 制約ストアと矛盾するaskのリスト
  negative_asks_t*         negative_asks_;

  /// 有効となっているaskのリスト
  positive_asks_t*         positive_asks_;
  
  /// askのリスト
  ask_set_t*              unknown_asks_;
  
  /// 有効になったaskの中かどうか
  bool                in_positive_ask_;
  
  /// 探索したalwaysノードのリスト
  visited_always_t   visited_always_;

  expanded_always_t  new_expanded_always_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_ASK_COLLECTOR_H_
