#pragma once

#include "Node.h"

namespace hydla {
namespace simulator {

class DiscreteAsk : public hydla::symbolic_expression::Ask
{
public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;

  DiscreteAsk()
  {}

  DiscreteAsk(const symbolic_expression::node_sptr& guard, const symbolic_expression::node_sptr& child)
    : hydla::symbolic_expression::Ask(guard, child)
  {}
    
  virtual ~DiscreteAsk()
  {}

  // virtual void accept(symbolic_expression::node_sptr own, TreeVisitor* visitor);
  // virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual symbolic_expression::node_sptr clone()
  {
    node_type_sptr n(new DiscreteAsk);
    return hydla::symbolic_expression::BinaryNode::clone(n);
  }

  virtual std::string get_node_type_name() const {
    return "DiscreteAsk";
  }
};

class ContinuousAsk : public hydla::symbolic_expression::Ask
{
public:
  typedef hydla::symbolic_expression::node_sptr node_sptr;

  ContinuousAsk()
  {}

  ContinuousAsk(const symbolic_expression::node_sptr& guard, const symbolic_expression::node_sptr& child)
    : hydla::symbolic_expression::Ask(guard, child)
  {}
    
  virtual ~ContinuousAsk()
  {}

  // virtual void accept(symbolic_expression::node_sptr own, TreeVisitor* visitor);
  // virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual symbolic_expression::node_sptr clone()
  {
    node_type_sptr n(new ContinuousAsk);
    return hydla::symbolic_expression::BinaryNode::clone(n);
  }

  virtual std::string get_node_type_name() const
  {
    return "ContinuousAsk";
  }
};

} // namespace simulator
} // namespace hydla 
