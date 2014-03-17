#ifndef _INCLUDED_HYDLA_PREV_REPLACER_H_
#define _INCLUDED_HYDLA_PREV_REPLACER_H_

#include <set>
#include <sstream>

#include <boost/shared_ptr.hpp>

#include "Node.h"
#include "DefaultTreeVisitor.h"
#include "PhaseResult.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

/**
 * A class to replace prevs with parameters.
 * (And introduce parameter into parameter_map)
 */
class PrevReplacer : public parse_tree::DefaultTreeVisitor{
  typedef parse_tree::node_sptr                 node_sptr;

  public:

  PrevReplacer(parameter_map_t& map, phase_result_sptr_t &phase, Simulator& simulator, bool approx);

  virtual ~PrevReplacer();
  
  void replace_value(value_t &val);
  void replace_node(node_sptr &exp);

  virtual void visit(boost::shared_ptr<parse_tree::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<parse_tree::ProgramDefinition> node);
  virtual void visit(boost::shared_ptr<parse_tree::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<parse_tree::ProgramCaller> node);
  virtual void visit(boost::shared_ptr<parse_tree::Constraint> node);

  virtual void visit(boost::shared_ptr<parse_tree::Ask> node);
  virtual void visit(boost::shared_ptr<parse_tree::Tell> node);

  virtual void visit(boost::shared_ptr<parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<parse_tree::LogicalOr> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<parse_tree::Parallel> node);
  virtual void visit(boost::shared_ptr<parse_tree::Always> node);
  virtual void visit(boost::shared_ptr<parse_tree::Float> node);
  virtual void visit(boost::shared_ptr<parse_tree::True> node);
  virtual void visit(boost::shared_ptr<parse_tree::False> node);


  virtual void visit(boost::shared_ptr<parse_tree::Print> node);
  virtual void visit(boost::shared_ptr<parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<parse_tree::PrintIP> node);
  virtual void visit(boost::shared_ptr<parse_tree::Scan> node);
  virtual void visit(boost::shared_ptr<parse_tree::Exit> node);
  virtual void visit(boost::shared_ptr<parse_tree::Abort> node);

  
  virtual void visit(boost::shared_ptr<parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<parse_tree::Power> node);

  virtual void visit(boost::shared_ptr<parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<parse_tree::GreaterEqual> node);
  virtual void visit(boost::shared_ptr<parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<parse_tree::UnEqual> node);

  virtual void visit(boost::shared_ptr<parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<parse_tree::Positive> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Differential> node);

  virtual void visit(boost::shared_ptr<parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<parse_tree::UnsupportedFunction> node);  

  virtual void visit(boost::shared_ptr<parse_tree::Variable> node);

  virtual void visit(boost::shared_ptr<parse_tree::Pi> node);
  virtual void visit(boost::shared_ptr<parse_tree::E> node);
  virtual void visit(boost::shared_ptr<parse_tree::Number> node);
  virtual void visit(boost::shared_ptr<parse_tree::Parameter> node);
  virtual void visit(boost::shared_ptr<parse_tree::SymbolicT> node);
  virtual void visit(boost::shared_ptr<parse_tree::Infinity> node);
  virtual void visit(boost::shared_ptr<parse_tree::SVtimer> node);
  virtual void visit(boost::shared_ptr<parse_tree::Previous> node);

  private:
  int differential_cnt_;
  bool in_prev_;
  parameter_map_t& parameter_map_;
  phase_result_sptr_t& prev_phase_;
  Simulator &simulator_;
  bool approx_;

  node_sptr new_child_;

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
    dispatch<parse_tree::UnaryNode, 
      &parse_tree::UnaryNode::get_child, 
      &parse_tree::UnaryNode::set_child>(node.get());
  }

  template<class NodeType>
  void dispatch_rhs(NodeType& node)
  {
    dispatch<parse_tree::BinaryNode, 
      &parse_tree::BinaryNode::get_rhs, 
      &parse_tree::BinaryNode::set_rhs>(node.get());
  }

  template<class NodeType>
  void dispatch_lhs(NodeType& node)
  {
    dispatch<parse_tree::BinaryNode, 
      &parse_tree::BinaryNode::get_lhs, 
      &parse_tree::BinaryNode::set_lhs>(node.get());
  }
};

}
}

#endif // include guard
