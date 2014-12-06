#pragma once

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "Variable.h"


namespace hydla {
namespace simulator {

typedef boost::shared_ptr<hydla::symbolic_expression::ExpressionListElement> list_element_sptr_t;

struct ListElementData
{
  list_element_sptr_t node;
  int differential_count;

  ListElementData(list_element_sptr_t n, int dc) : node(n),differential_count(dc){}
  ~ListElementData(){}
};

typedef std::set<boost::shared_ptr<ListElementData> > list_element_data_set_t;

/**
 * 制約を調べ，変数の出現を取得するクラス．
 */
class ExpressionListElementFinder : public symbolic_expression::DefaultTreeVisitor {
public:


  ExpressionListElementFinder();

  virtual ~ExpressionListElementFinder();
  
  /** 
   * 制約を調べ，変数の出現を取得する
   * @param node 調べる対象となる制約
   */
  void visit_node(boost::shared_ptr<symbolic_expression::Node> node);

  void clear();
  
  /// get found variables (include prev)
  list_element_data_set_t get_all_variable_set() const;

  /// get found variables (without prev)
  list_element_data_set_t get_variable_set() const;

  /// get prev variables
  list_element_data_set_t get_prev_variable_set() const;

  /// Ask制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node);

  /// 微分
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node);

  /// 左極限
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node);

  /// 変数
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::ExpressionListElement> node);

private:

  list_element_data_set_t variables_, prev_variables_;  
  int differential_count_;
  bool in_prev_;
};

} //namespace simulator
} //namespace hydla 

