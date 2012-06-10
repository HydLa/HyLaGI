#ifndef _INCLUDED_HYDLA_SIMULATOR_TYPED_ASK_H_
#define _INCLUDED_HYDLA_SIMULATOR_TYPED_ASK_H_

#include "Node.h"

namespace hydla {
namespace simulator {

class DiscreteAsk :
  public hydla::parse_tree::Ask
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  DiscreteAsk()
  {}

  DiscreteAsk(const node_sptr& guard, const node_sptr& child) :
    hydla::parse_tree::Ask(guard, child)
  {}
    
  virtual ~DiscreteAsk()
  {}

  // virtual void accept(node_sptr own, TreeVisitor* visitor);
  // virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    node_type_sptr n(new DiscreteAsk);
    return hydla::parse_tree::BinaryNode::clone(n);
  }

  virtual std::string get_node_type_name() const {
    return "DiscreteAsk";
  }
};

class ContinuousAsk :
  public hydla::parse_tree::Ask
{
public:
  typedef hydla::parse_tree::node_sptr node_sptr;

  ContinuousAsk()
  {}

  ContinuousAsk(const node_sptr& guard, const node_sptr& child) :
    hydla::parse_tree::Ask(guard, child)
  {}
    
  virtual ~ContinuousAsk()
  {}

  // virtual void accept(node_sptr own, TreeVisitor* visitor);
  // virtual bool is_same_struct(const Node& n, bool exactly_same) const;

  virtual node_sptr clone()
  {
    node_type_sptr n(new ContinuousAsk);
    return hydla::parse_tree::BinaryNode::clone(n);
  }

  virtual std::string get_node_type_name() const {
    return "ContinuousAsk";
  }
};

} //namespace simulator
} //namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_TYPED_ASK_H_