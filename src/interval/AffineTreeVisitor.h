#pragma once

#include <boost/bimap.hpp>

#include "TreeVisitor.h"
#include "AffineOrInteger.h"
#include "Parameter.h"
#include "PhaseResult.h"

namespace hydla {
namespace interval {

typedef symbolic_expression::node_sptr        node_sptr;
typedef simulator::Parameter                  parameter_t;
typedef simulator::Value                      value_t;
typedef simulator::ValueRange                 range_t;
typedef simulator::parameter_map_t            parameter_map_t;
typedef simulator::variable_map_t             variable_map_t;


typedef boost::bimaps::bimap<parameter_t, int >
  parameter_idx_map_t;
typedef parameter_idx_map_t::value_type parameter_idx_t;

/**
 * A tree visitor to approximates symbolic formulas as affine forms
 */
class AffineTreeVisitor : public symbolic_expression::TreeVisitor{
  public:

  AffineTreeVisitor(parameter_idx_map_t&, variable_map_t&);

  AffineOrInteger approximate(node_sptr &node);
  ///calculate x^y
  AffineOrInteger pow(AffineOrInteger x, AffineOrInteger y);
  affine_t pow(affine_t affine, int exp);
  AffineOrInteger sqrt_affine(const AffineOrInteger &a);
  
  virtual ~AffineTreeVisitor();  

  virtual void visit(boost::shared_ptr<symbolic_expression::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramDefinition> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramCaller> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Constraint> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Ask> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Tell> node);


  virtual void visit(boost::shared_ptr<symbolic_expression::Equal> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::UnEqual> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Less> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::LessEqual> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Greater> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::GreaterEqual> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::LogicalOr> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Plus> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Subtract> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Times> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Divide> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Power> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Negative> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Positive> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Weaker> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Parallel> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::Always> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Differential> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::Previous> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Print> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::PrintPP> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::PrintIP> node);
    
  virtual void visit(boost::shared_ptr<symbolic_expression::Scan> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Exit> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Abort> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::SVtimer> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Not> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Pi> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::E> node);
  
  virtual void visit(boost::shared_ptr<symbolic_expression::Function> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::UnsupportedFunction> node);

  virtual void visit(boost::shared_ptr<symbolic_expression::Variable> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Number> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Float> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Parameter> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::SymbolicT> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::Infinity> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::True> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::False> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ExpressionList> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ConditionalExpressionList> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ProgramList> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::ConditionalProgramList> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::EachElement> node);
  virtual void visit(boost::shared_ptr<symbolic_expression::DifferentVariable> node);
  
private:

  static affine_t pi, e; 
  void invalid_node(symbolic_expression::Node& node);

  AffineOrInteger current_val_;
  parameter_idx_map_t& parameter_idx_map_;
  variable_map_t& variable_map;
  int differential_count;
};

} //namespace interval
} //namespace hydla
