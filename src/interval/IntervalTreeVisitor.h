#pragma once

/* 柏木先生の区間演算用のライブラリ */
#include "kv/interval.hpp"
#include "kv/rdouble.hpp"
#include "kv/dd.hpp"
#include "kv/rdd.hpp"
#include "kv/constants.hpp"

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "Parameter.h"
#include "Value.h"
#include "ValueRange.h"
#include "PhaseResult.h"


namespace hydla
{
namespace interval
{

typedef symbolic_expression::node_sptr            node_sptr;
typedef symbolic_expression::DefaultTreeVisitor   DefaultTreeVisitor;
typedef simulator::Parameter                  parameter_t;
typedef simulator::Value                      value_t;
typedef simulator::ValueRange                 range_t;
typedef simulator::parameter_map_t            parameter_map_t;
typedef kv::interval<double>                  itvd;


class IntervalTreeVisitor : public DefaultTreeVisitor
{
  public:

  IntervalTreeVisitor();
  
  itvd get_interval_value(const node_sptr &, itvd *time_interval = nullptr, parameter_map_t *parameter_map = nullptr);
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Power> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Number> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Float> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::E> node);
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node);
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Function> node);

  private:

  void invalid_node(symbolic_expression::Node &node);
  void debug_print(std::string str, itvd x);
  
  static itvd pi, e;
  itvd interval_value;
  itvd *time_interval;
  parameter_map_t *parameter_map;
};


}
}

