#ifndef _INCLUDED_HYDLA_AFFINE_TRANSFORMER_H_
#define _INCLUDED_HYDLA_AFFINE_TRANSFORMER_H_

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
#include "Simulator.h"
#include "Parameter.h"

namespace hydla {
namespace interval {


typedef hydla::parse_tree::node_sptr          node_sptr;
typedef simulator::Parameter                  parameter_t;
typedef simulator::Value                      value_t;
typedef simulator::ValueRange                 range_t;
typedef simulator::parameter_map_t            parameter_map_t;


/**
 * A class which transforms symbolic formula to affine form
 */
class AffineTransformer : public parse_tree::TreeVisitor{
  public:

  static AffineTransformer* get_instance();
  
  void set_simulator(simulator::Simulator* simulator);

  /**
   * transform given expression to affine form
   * @param node expression
   * @param parameter_map
   *          map of parameter to be added parameter for new dummy variables
   */
  value_t transform(node_sptr &node, parameter_map_t &parameter_map);
  ///calculate x^y
  AffineOrInteger pow(AffineOrInteger x, AffineOrInteger y);
  affine_t pow(affine_t affine, int exp);
  
  /**
   * Reduce dummy variables
   * @param formulas Formulas to be reduced
   * @param limit the number which the number of dummy variables are to be after reduction
   */
  boost::numeric::ublas::vector<affine_t> reduce_dummy_variables(boost::numeric::ublas::vector<affine_t> formulas, int limit);

  virtual ~AffineTransformer();  

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

  typedef boost::bimaps::bimap<parameter_t, int >
    parameter_idx_map_t;
  typedef parameter_idx_map_t::value_type parameter_idx_t;


  AffineTransformer();
  AffineTransformer(const AffineTransformer& rhs);
  AffineTransformer& operator=(const AffineTransformer& rhs);

  void invalid_node(parse_tree::Node& node);

  simulator::Simulator* simulator_;

  static interval::AffineTransformer* affine_translator_;

  AffineOrInteger current_val_;
  parameter_idx_map_t parameter_idx_map_;
};

} //namespace interval
} //namespace hydla

#endif // include guard
