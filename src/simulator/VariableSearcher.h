#ifndef _INCLUDED_HYDLA_VARIABLE_SEARCHER_H_
#define _INCLUDED_HYDLA_VARIABLE_SEARCHER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"

namespace hydla {
namespace simulator {

/**
 * 制約を調べ，変数の出現を取得するクラス．
 */
class VariableSearcher : public parse_tree::DefaultTreeVisitor {
public:

  typedef std::set<std::string> variable_set_t;

  VariableSearcher();

  virtual ~VariableSearcher();
  
  /** 
   * 変数の集合の要素が制約に含まれるか調べる。
   * Ask制約はガード条件を調べる。
   * include_prev:prev変数も対象にするか
   */
  bool visit_node(std::set<std::string> variables, boost::shared_ptr<parse_tree::Node> node, const bool& include_prev);
  
  // Ask制約
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Ask> node);

    // 変数
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  // 左極限
  void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

private:

  variable_set_t variables_;
  
  bool in_prev_;
  bool include_prev_;
  bool has_variables_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VARIABLE_SEARCHER_H_
