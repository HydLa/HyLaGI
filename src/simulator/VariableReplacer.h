#ifndef _INCLUDED_HYDLA_VARIABLE_REPLACER_H_
#define _INCLUDED_HYDLA_VARIABLE_REPLACER_H_

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
class VariableReplacer : public symbolic_expression::DefaultTreeVisitor{
  typedef hydla::symbolic_expression::node_sptr                 node_sptr;

  public:

  VariableReplacer(const variable_map_t& map);

  void replace_node(symbolic_expression::node_sptr& node);

  virtual ~VariableReplacer();

  void replace_value(value_t &val);
  void replace_range(ValueRange &range);
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Power> node);

  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Positive> node);
  
  virtual void visit(boost::shared_ptr<hydla::symbolic_expression::Differential> node);

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
  hydla::simulator::Value* processing_value;
  int differential_cnt;
  uint replace_cnt;
  const variable_map_t& variable_map;


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

} //namespace parser
} //namespace hydla

#endif // _INCLUDED_HYDLA_PARAMETER_REPLACER_H_
