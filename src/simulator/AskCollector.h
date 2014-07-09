#pragma once

#include <vector>
#include <set>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ModuleSet.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

/**
 * askノードを集めるビジタークラス
 */
class AskCollector : public symbolic_expression::DefaultTreeVisitor {
public:
  AskCollector();

  virtual ~AskCollector();

  /** 
   * ガード条件の判定対象となるaskノードを集める
   *
   * @param constraints      捜索対象となる制約の集合
   * @param positive_asks    ガード条件が成立したaskノードの集合
   * @param negative_asks    ガード条件が成立しないaskノードの集合
   * @param unknown_asks     成否の定まっていないaskノードの集合
   */
  void collect_ask(ConstraintStore &constraints,                   
                   const ask_set_t*   positive_asks,
                   const ask_set_t*   negative_asks,
                   ask_set_t*        unknown_asks);

  // Ask制約
  virtual void visit(boost::shared_ptr<symbolic_expression::Ask> node);

private:  
  /// 制約ストアと矛盾するaskのリスト
  const ask_set_t*         negative_asks_;

  /// 有効となっているaskのリスト
  const ask_set_t*         positive_asks_;
  
  /// まだ導出の可否が不明なaskのリスト
  ask_set_t*              unknown_asks_;
  
  /// 有効になったaskの中かどうか（出力用）
  bool                in_positive_ask_;
};

} //namespace simulator
} //namespace hydla 

