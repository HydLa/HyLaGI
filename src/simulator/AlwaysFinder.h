#pragma once

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

/**
 * 制約を調べ，変数の出現を取得するクラス．
 */
class AlwaysFinder : public symbolic_expression::DefaultTreeVisitor {
public:

  AlwaysFinder(){}

  virtual ~AlwaysFinder(){}
  
  /** 
   * 制約を調べ，変数の出現を取得する
   * @param node 調べる対象となる制約
   * @param include_guard ガード条件を対象とするかどうか
   */
  void find_always(symbolic_expression::node_sptr node, always_set_t& always_set)
  {
    always_set_ = &always_set;
    accept(node);
  }


  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node)
  {
    // do nothing
  }
  
  // Always
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Always> node)
  {
    always_set_->insert(node);
  }

private:

  always_set_t *always_set_;
};

} //namespace simulator
} //namespace hydla 

