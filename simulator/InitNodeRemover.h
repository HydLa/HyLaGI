#ifndef _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_
#define _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ParseTree.h"
#include "TreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * tellノードを集めるビジタークラス
 */
class InitNodeRemover : public parse_tree::TreeVisitor {
public:
  InitNodeRemover();
  virtual ~InitNodeRemover();

  /**
   *
   */
  void apply(boost::shared_ptr<hydla::parse_tree::ParseTree> pt);

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

  // モジュールの弱合成
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Weaker> node);

  // モジュールの並列合成
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parallel> node);
   
  // 制約呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ProgramCaller> node);

private:

};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_
