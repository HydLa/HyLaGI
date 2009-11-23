#ifndef _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_

#include <boost/shared_ptr.hpp>

#include "Node.h"

namespace hydla { 
namespace parse_tree {
  
/**
 * ParseTreeのノード集合に対するVisitorクラス
 */
class TreeVisitor {
public:
  TreeVisitor();

  virtual ~TreeVisitor();

  /**
   * Nodeクラスのaccept関数呼び出し用ヘルパ関数
   */
  template<class T>
  void accept(const T& n)
  {
    n->accept(n, this);
  }

  // 制約定義
  virtual void visit(boost::shared_ptr<ConstraintDefinition> node);
  
  // プログラム定義
  virtual void visit(boost::shared_ptr<ProgramDefinition> node);

  // 制約呼び出し
  virtual void visit(boost::shared_ptr<ConstraintCaller> node);
  
  // プログラム呼び出し
  virtual void visit(boost::shared_ptr<ProgramCaller> node);

  // 制約式
  virtual void visit(boost::shared_ptr<Constraint> node);

  // Ask制約
  virtual void visit(boost::shared_ptr<Ask> node);

  // Tell制約
  virtual void visit(boost::shared_ptr<Tell> node);

  // 比較演算子
  virtual void visit(boost::shared_ptr<Equal> node);
  virtual void visit(boost::shared_ptr<UnEqual> node);
  virtual void visit(boost::shared_ptr<Less> node);
  virtual void visit(boost::shared_ptr<LessEqual> node);
  virtual void visit(boost::shared_ptr<Greater> node);
  virtual void visit(boost::shared_ptr<GreaterEqual> node);

  // 論理演算子
  virtual void visit(boost::shared_ptr<LogicalAnd> node);
  virtual void visit(boost::shared_ptr<LogicalOr> node);
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<Plus> node);
  virtual void visit(boost::shared_ptr<Subtract> node);
  virtual void visit(boost::shared_ptr<Times> node);
  virtual void visit(boost::shared_ptr<Divide> node);
  
  // 算術単項演算子
  virtual void visit(boost::shared_ptr<Negative> node);
  virtual void visit(boost::shared_ptr<Positive> node);
  
  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<Always> node);
  
  // 微分
  virtual void visit(boost::shared_ptr<Differential> node);

  // 左極限
  virtual void visit(boost::shared_ptr<Previous> node);
  
  // 変数
  virtual void visit(boost::shared_ptr<Variable> node);

  // 数字
  virtual void visit(boost::shared_ptr<Number> node);

};

} //namespace parse_tree
} //namespace hydla

#endif //_INCLUDED_HYDLA_PARSE_TREE_TREE_VISITOR_H_
