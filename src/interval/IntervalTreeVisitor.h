#pragma once

/* 柏木先生の区間演算用のライブラリ */
#include "kv/interval.hpp"
#include "kv/rdouble.hpp"
#include "kv/dd.hpp"
#include "kv/rdd.hpp"
#include "kv/constants.hpp"

#include "Node.h"
#include "DefaultTreeVisitor.h"


namespace hydla
{
namespace interval
{

typedef symbolic_expression::node_sptr            node_sptr;
typedef symbolic_expression::DefaultTreeVisitor   DefaultTreeVisitor;
// typedef simulator::Parameter                  parameter_t;
// typedef simulator::Value                      value_t;
// typedef simulator::ValueRange                 range_t;
// typedef simulator::parameter_map_t            parameter_map_t;
// typedef simulator::variable_map_t             variable_map_t;
typedef kv::interval<double>                  itvd;


class IntervalTreeVisitor : public DefaultTreeVisitor
{
  public:

  IntervalTreeVisitor(itvd);
  
  // ノードを引数に取り、区間演算した結果を返す
  itvd get_interval_value(const node_sptr &);
  
  // 算術二項演算子
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Power> node);

  // 数字
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Number> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Float> node);

  // 記号定数
  // virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node);

  // t
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node);

  // 無限大
  // virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Infinity> node);
  
  // 自然対数の底
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::E> node);
  
  // 円周率
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node);
  
  
  // 任意
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Function> node);

  private:

  void invalid_node(symbolic_expression::Node &node);
  void debug_print(std::string str, itvd x);
  
  static itvd pi, e;
  itvd interval_value_;
  itvd interval_arg_;
};


}
}

