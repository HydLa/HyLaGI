#ifndef _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_
#define _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "ParseTree.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * A class to remove init-node
 */
class InitNodeRemover : public symbolic_expression::DefaultTreeVisitor {
public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;

  InitNodeRemover();
  virtual ~InitNodeRemover();

  /**
   *
   */
  void apply(hydla::parse_tree::ParseTree* pt);

  // 制約式
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Tell> node);

  // 論理積
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::LogicalAnd> node);
  
  // 時相演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Always> node);

  // モジュールの弱合成
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Weaker> node);

  // モジュールの並列合成
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parallel> node);
   
  // 制約呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ProgramCaller> node);

private:
  symbolic_expression::node_sptr child_;

  void unary_node(boost::shared_ptr<hydla::symbolic_expression::UnaryNode> node);
  void binary_node(boost::shared_ptr<hydla::symbolic_expression::BinaryNode> node);
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_INIT_NODE_REMOVER_H_
