#pragma once

/* 柏木先生の区間演算用のライブラリ */
#include "kv/constants.hpp"
#include "kv/dd.hpp"
#include "kv/interval.hpp"
#include "kv/rdd.hpp"
#include "kv/rdouble.hpp"

#include "DefaultTreeVisitor.h"
#include "IntervalOrInteger.h"
#include "Node.h"
#include "Parameter.h"
#include "PhaseResult.h"
#include "Value.h"
#include "ValueRange.h"

namespace hydla {
namespace interval {

typedef symbolic_expression::node_sptr node_sptr;
typedef symbolic_expression::DefaultTreeVisitor DefaultTreeVisitor;
typedef simulator::Parameter parameter_t;
typedef simulator::Value value_t;
typedef simulator::ValueRange range_t;
typedef simulator::parameter_map_t parameter_map_t;
typedef kv::interval<double> itvd;

class IntervalTreeVisitor : public symbolic_expression::TreeVisitor {
public:
  IntervalTreeVisitor();

  itvd get_interval_value(const node_sptr &, itvd *time_interval = nullptr,
                          parameter_map_t *parameter_map = nullptr);

  virtual void
  visit(std::shared_ptr<symbolic_expression::ConstraintDefinition> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramDefinition> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ConstraintCaller> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramCaller> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Constraint> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Ask> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Exists> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Tell> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Equal> node);
  virtual void visit(std::shared_ptr<symbolic_expression::UnEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Less> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LessEqual> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Greater> node);
  virtual void visit(std::shared_ptr<symbolic_expression::GreaterEqual> node);

  virtual void visit(std::shared_ptr<symbolic_expression::LogicalAnd> node);
  virtual void visit(std::shared_ptr<symbolic_expression::LogicalOr> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Plus> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Subtract> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Times> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Divide> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Power> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Negative> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Positive> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Weaker> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Parallel> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Always> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Differential> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Previous> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Print> node);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintPP> node);
  virtual void visit(std::shared_ptr<symbolic_expression::PrintIP> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Scan> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Exit> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Abort> node);

  virtual void visit(std::shared_ptr<symbolic_expression::SVtimer> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Not> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Pi> node);

  virtual void visit(std::shared_ptr<symbolic_expression::E> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ImaginaryUnit> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Function> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::UnsupportedFunction> node);

  virtual void visit(std::shared_ptr<symbolic_expression::Variable> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Float> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Parameter> node);
  virtual void visit(std::shared_ptr<symbolic_expression::SymbolicT> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Infinity> node);
  virtual void visit(std::shared_ptr<symbolic_expression::True> node);
  virtual void visit(std::shared_ptr<symbolic_expression::False> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ExpressionList> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ConditionalExpressionList> node);
  virtual void visit(std::shared_ptr<symbolic_expression::ProgramList> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ConditionalProgramList> node);
  virtual void visit(std::shared_ptr<symbolic_expression::EachElement> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::DifferentVariable> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramListCaller> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramListElement> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ProgramListDefinition> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ExpressionListCaller> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ExpressionListElement> node);
  virtual void
  visit(std::shared_ptr<symbolic_expression::ExpressionListDefinition> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Union> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Intersection> node);
  virtual void visit(std::shared_ptr<symbolic_expression::Range> node);
  virtual void visit(std::shared_ptr<symbolic_expression::SumOfList> node);
  virtual void visit(std::shared_ptr<symbolic_expression::MulOfList> node);
  virtual void visit(std::shared_ptr<symbolic_expression::SizeOfList> node);

private:
  void invalid_node(symbolic_expression::Node &node);
  void debug_print(std::string str, itvd x);

  static itvd pi, e;
  IntervalOrInteger current_value;
  itvd interval_value;
  itvd *time_interval;
  parameter_map_t *parameter_map;
};

class IntervalException : public std::runtime_error {
public:
  IntervalException(const std::string &msg)
      : std::runtime_error("error occured in interval calculation: " + msg) {}
};

} // namespace interval
} // namespace hydla
