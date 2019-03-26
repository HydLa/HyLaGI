#pragma once

#ifdef commnetout

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"
#include "Value.h"

namespace hydla {
namespace simulator {

/**
 * Replace variables with their values
 */
class NonlinearReplacer : public symbolic_expression::DefaultTreeVisitor
{
  typedef hydla::symbolic_expression::node_sptr node_sptr;

public:
  NonlinearReplacer(const variable_map_t& map, bool v_to_par);

  void replace_node(symbolic_expression::node_sptr& node);

  virtual ~NonlinearReplacer();

  void replace_value(value_t &val);
  void replace_range(ValueRange &range);

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


  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Power> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Positive> node);


  // 制約階層定義演算子
  virtual void visit(boost::shared_ptr<Weaker> node);
  virtual void visit(boost::shared_ptr<Parallel> node);

  // 時相演算子
  virtual void visit(boost::shared_ptr<Always> node);

  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node);


  //Print
  virtual void visit(boost::shared_ptr<Print> node);
  virtual void visit(boost::shared_ptr<PrintPP> node);
  virtual void visit(boost::shared_ptr<PrintIP> node);

  virtual void visit(boost::shared_ptr<Scan> node);
  virtual void visit(boost::shared_ptr<Exit> node);
  virtual void visit(boost::shared_ptr<Abort> node);

  // 否定
  virtual void visit(boost::shared_ptr<Not> node);

  // True
  virtual void visit(boost::shared_ptr<True> node);
  // False
  virtual void visit(boost::shared_ptr<False> node);



  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Function> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::UnsupportedFunction> node);  

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Variable> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Pi> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::E> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Number> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Parameter> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::SymbolicT> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Infinity> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::SVtimer> node);

private:
  int differential_cnt;
  uint replace_cnt;
  const variable_map_t& variable_map;
  const bool v_to_par;

  symbolic_expression::node_sptr new_child_;

  template<class C, 
           const symbolic_expression::node_sptr& (C::*getter)() const,
           void (C::*setter)(const symbolic_expression::node_sptr& child)>
  void dispatch(C* n) 
  {
    accept((n->*getter)());
    if(new_child_) {
      (n->*setter)(new_child_);
      new_child_.reset();
    }
  }
  
  template<class NodeType>
  void dispatch_child(NodeType& node)
  {
    dispatch<hydla::symbolic_expression::UnaryNode, 
      &hydla::symbolic_expression::UnaryNode::get_child, 
      &hydla::symbolic_expression::UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<hydla::symbolic_expression::BinaryNode, 
      &hydla::symbolic_expression::BinaryNode::get_rhs, 
      &hydla::symbolic_expression::BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<hydla::symbolic_expression::BinaryNode, 
      &hydla::symbolic_expression::BinaryNode::get_lhs, 
      &hydla::symbolic_expression::BinaryNode::set_lhs>(node.get());
  }
};

} // namespace parser
} // namespace hydla

#endif
