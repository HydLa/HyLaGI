#pragma once

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "Variable.h"

namespace hydla {
namespace simulator {

typedef symbolic_expression::node_sptr constraint_t;
typedef std::set<constraint_t> constraints_t;

/**
 * 制約を調べ，変数の出現を取得するクラス．
 */
class AtomicConstraintFinder : public symbolic_expression::DefaultTreeVisitor
{
public:
  AtomicConstraintFinder();

  virtual ~AtomicConstraintFinder();
  
  /** 
   * 制約を調べ，変数の出現を取得する
   * @param node 調べる対象となる制約
   */
  void visit_node(boost::shared_ptr<symbolic_expression::Node> node);

  void clear();
  
  /// get found constraints
  constraints_t get_all_constraint_set() const;

  /// judge if given constraints include any of found variables
  bool include_constraint(const constraint_t &con) const;
  
  // 比較演算子
  virtual void visit(boost::shared_ptr<symbolic_expression::Equal> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::UnEqual> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Less> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::LessEqual> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Greater> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::GreaterEqual> node);
  
private:
  constraints_t constraints_;
};

} // namespace simulator
} // namespace hydla 
