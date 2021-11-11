#pragma once

#include <set>
#include <sstream>

#include <memory>

#include "DefaultTreeVisitor.h"
#include "Node.h"
#include "Variable.h"

namespace hydla {
namespace simulator {

typedef std::set<Variable, VariableComparator> variable_set_t;

/**
 * 制約を調べ，変数の出現を取得するクラス．
 */
class VariableFinder : public symbolic_expression::DefaultTreeVisitor {
public:
  VariableFinder();

  virtual ~VariableFinder();

  /**
   * 制約を調べ，変数の出現を取得する
   * @param node 調べる対象となる制約
   */
  void visit_node(std::shared_ptr<symbolic_expression::Node> node);

  void clear();

  /// get found variables (include prev)
  variable_set_t get_all_variable_set() const;

  /// get found variables (without prev)
  variable_set_t get_variable_set() const;

  /// get prev variables
  variable_set_t get_prev_variable_set() const;

  /// get exists variables
  variable_set_t get_exists_variable_set() const;

  /// judge if found variables include given variables
  bool include_variables(std::set<std::string> variables) const;
  bool include_variables_prev(std::set<std::string> variables) const;

  bool include_variable(const Variable &var) const;
  bool include_variable_prev(const Variable &var) const;

  /// judge if given constraints include any of found variables
  bool include_variables(
      const std::shared_ptr<symbolic_expression::Node> &constraint) const;

  /// Ask制約
  virtual void visit(std::shared_ptr<hydla::symbolic_expression::Ask> node);

  virtual void visit(std::shared_ptr<hydla::symbolic_expression::Exists> node);

  /// 微分
  virtual void
  visit(std::shared_ptr<hydla::symbolic_expression::Differential> node);

  /// 左極限
  virtual void
  visit(std::shared_ptr<hydla::symbolic_expression::Previous> node);

  /// 変数
  virtual void
  visit(std::shared_ptr<hydla::symbolic_expression::Variable> node);

  /// 時刻
  virtual void
  visit(std::shared_ptr<hydla::symbolic_expression::SymbolicT> node);

private:
  variable_set_t variables_, prev_variables_, exists_variables_;
  int differential_count_;
  bool in_prev_;
};

} // namespace simulator
} // namespace hydla
