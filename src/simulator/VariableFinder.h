#ifndef _INCLUDED_HYDLA_VARIABLE_FINDER_H_
#define _INCLUDED_HYDLA_VARIABLE_FINDER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"


namespace hydla {
namespace simulator {

class Variable;

/**
 * 制約を調べ，変数の出現を取得するクラス．
 */
class VariableFinder : public symbolic_expression::DefaultTreeVisitor {
public:

  typedef std::set<Variable > variable_set_t;

  VariableFinder();

  virtual ~VariableFinder();
  
  /** 
   * 制約を調べ，変数の出現を取得する
   * @param node 調べる対象となる制約
   * @param include_guard ガード条件を対象とするかどうか
   */
  void visit_node(boost::shared_ptr<symbolic_expression::Node> node, bool include_guard = true);

  void clear();
  
  /// get found variables (include prev)
  variable_set_t get_all_variable_set() const;

  /// get found variables (without prev)
  variable_set_t get_variable_set() const;

  /// get prev variables
  variable_set_t get_prev_variable_set() const;

  /// judge if found variables include given variables
  bool include_variables(std::set<std::string> variables) const;
  bool include_variables_prev(std::set<std::string> variables) const;

  bool include_variable(const Variable& var)const;
  
  /// Ask制約
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Ask> node);

  /// 微分
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node);

  /// 左極限
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Previous> node);

  /// 変数
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node);

  /// 時刻
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node);

private:

  variable_set_t variables_, prev_variables_;  
  int differential_count_;
  bool in_prev_;
  bool include_guard_;
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_VARIABLE_FINDER_H_
