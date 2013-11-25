#ifndef _INCLUDED_HYDLA_PREV_REPLACER_H_
#define _INCLUDED_HYDLA_PREV_REPLACER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "ValueVisitor.h"
#include "PhaseResult.h"
#include "Simulator.h"
#include "SymbolicValue.h"

namespace hydla {
namespace simulator {

/**
 * A class to replace prevs with parameters.
 * (And introduce parameter into parameter_map)
 */
class PrevReplacer : public parse_tree::DefaultTreeVisitor, hydla::simulator::ValueVisitor{
  typedef hydla::parse_tree::node_sptr                 node_sptr;

  public:

  PrevReplacer(parameter_map_t& map, phase_result_sptr_t &phase, Simulator& simulator);

  virtual ~PrevReplacer();
  
  virtual void visit(hydla::simulator::symbolic::SymbolicValue&);

  void replace_value(value_t& val);
  
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Power> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Positive> node);
  
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Differential> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::UnsupportedFunction> node);  

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Variable> node);

  virtual void visit(boost::shared_ptr<hydla::parse_tree::Pi> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::E> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::ArbitraryNode> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Number> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Parameter> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SymbolicT> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Infinity> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::SVtimer> node);
  virtual void visit(boost::shared_ptr<hydla::parse_tree::Previous> node);

  private:
  int differential_cnt_;
  bool in_prev_;
  parameter_map_t& parameter_map_;
  phase_result_sptr_t& prev_phase_;
  Simulator& simulator_;

  node_sptr parameter_node_;
  node_sptr new_child_;

  void replace(node_sptr node);

  template<class C, 
           const node_sptr& (C::*getter)() const,
           void (C::*setter)(const node_sptr& child)>
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
    dispatch<hydla::parse_tree::UnaryNode, 
      &hydla::parse_tree::UnaryNode::get_child, 
      &hydla::parse_tree::UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<hydla::parse_tree::BinaryNode, 
      &hydla::parse_tree::BinaryNode::get_rhs, 
      &hydla::parse_tree::BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<hydla::parse_tree::BinaryNode, 
      &hydla::parse_tree::BinaryNode::get_lhs, 
      &hydla::parse_tree::BinaryNode::set_lhs>(node.get());
  }
};

}
}

#endif // include guard
