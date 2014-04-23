#ifndef _INCLUDED_HYDLA_AFFINE_TREE_VISITOR_H_
#define _INCLUDED_HYDLA_AFFINE_TREE_VISITOR_H_

#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/bimap.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#include "Node.h"
#include "PhaseResult.h"
#include "TreeVisitor.h"
#include "AffineOrInteger.h"
#include "Parameter.h"

namespace hydla {
namespace interval {

typedef hydla::parse_tree::node_sptr          node_sptr;
typedef simulator::Parameter                  parameter_t;
typedef simulator::Value                      value_t;
typedef simulator::ValueRange                 range_t;
typedef simulator::parameter_map_t            parameter_map_t;



typedef boost::bimaps::bimap<parameter_t, int >
  parameter_idx_map_t;
typedef parameter_idx_map_t::value_type parameter_idx_t;

/**
 * A tree visitor to approximates symbolic formulas as affine forms
 */
class AffineTreeVisitor : public parse_tree::TreeVisitor{
  public:

  AffineTreeVisitor(parameter_idx_map_t&);

  AffineOrInteger approximate(node_sptr &node);
  ///calculate x^y
  AffineOrInteger pow(AffineOrInteger x, AffineOrInteger y);
  affine_t pow(affine_t affine, int exp);
  AffineOrInteger sqrt_affine(const AffineOrInteger &a);
  
  virtual ~AffineTreeVisitor();  

  virtual void visit(boost::shared_ptr<parse_tree::ConstraintDefinition> node);
  virtual void visit(boost::shared_ptr<parse_tree::ProgramDefinition> node);
  virtual void visit(boost::shared_ptr<parse_tree::ConstraintCaller> node);
  virtual void visit(boost::shared_ptr<parse_tree::ProgramCaller> node);
  virtual void visit(boost::shared_ptr<parse_tree::Constraint> node);
  virtual void visit(boost::shared_ptr<parse_tree::Ask> node);
  virtual void visit(boost::shared_ptr<parse_tree::Tell> node);


  virtual void visit(boost::shared_ptr<parse_tree::Equal> node);
  virtual void visit(boost::shared_ptr<parse_tree::UnEqual> node);
  virtual void visit(boost::shared_ptr<parse_tree::Less> node);
  virtual void visit(boost::shared_ptr<parse_tree::LessEqual> node);
  virtual void visit(boost::shared_ptr<parse_tree::Greater> node);
  virtual void visit(boost::shared_ptr<parse_tree::GreaterEqual> node);

  virtual void visit(boost::shared_ptr<parse_tree::LogicalAnd> node);
  virtual void visit(boost::shared_ptr<parse_tree::LogicalOr> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Plus> node);
  virtual void visit(boost::shared_ptr<parse_tree::Subtract> node);
  virtual void visit(boost::shared_ptr<parse_tree::Times> node);
  virtual void visit(boost::shared_ptr<parse_tree::Divide> node);
  virtual void visit(boost::shared_ptr<parse_tree::Power> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Negative> node);
  virtual void visit(boost::shared_ptr<parse_tree::Positive> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Weaker> node);
  virtual void visit(boost::shared_ptr<parse_tree::Parallel> node);

  virtual void visit(boost::shared_ptr<parse_tree::Always> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Differential> node);

  virtual void visit(boost::shared_ptr<parse_tree::Previous> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Print> node);
  virtual void visit(boost::shared_ptr<parse_tree::PrintPP> node);
  virtual void visit(boost::shared_ptr<parse_tree::PrintIP> node);
    
  virtual void visit(boost::shared_ptr<parse_tree::Scan> node);
  virtual void visit(boost::shared_ptr<parse_tree::Exit> node);
  virtual void visit(boost::shared_ptr<parse_tree::Abort> node);

  virtual void visit(boost::shared_ptr<parse_tree::SVtimer> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Not> node);
  virtual void visit(boost::shared_ptr<parse_tree::Pi> node);

  virtual void visit(boost::shared_ptr<parse_tree::E> node);
  
  virtual void visit(boost::shared_ptr<parse_tree::Function> node);
  virtual void visit(boost::shared_ptr<parse_tree::UnsupportedFunction> node);

  virtual void visit(boost::shared_ptr<parse_tree::Variable> node);
  virtual void visit(boost::shared_ptr<parse_tree::Number> node);
  virtual void visit(boost::shared_ptr<parse_tree::Float> node);
  virtual void visit(boost::shared_ptr<parse_tree::Parameter> node);
  virtual void visit(boost::shared_ptr<parse_tree::SymbolicT> node);
  virtual void visit(boost::shared_ptr<parse_tree::Infinity> node);
  virtual void visit(boost::shared_ptr<parse_tree::True> node);
  virtual void visit(boost::shared_ptr<parse_tree::False> node);
  
private:


  void invalid_node(parse_tree::Node& node);

  AffineOrInteger current_val_;
  parameter_idx_map_t& parameter_idx_map_;
};

} //namespace interval
} //namespace hydla

#endif // include guard
