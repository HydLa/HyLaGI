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


typedef hydla::symbolic_expression::node_sptr          node_sptr;
typedef simulator::Parameter                  parameter_t;
typedef simulator::Value                      value_t;
typedef simulator::ValueRange                 range_t;
typedef simulator::parameter_map_t            parameter_map_t;


/**
 * A class which transforms symbolic formula to affine form
 */
class AffineTransformer : public symbolic_expression::TreeVisitor{
  public:

  static AffineTransformer* get_instance();
  
  void set_simulator(simulator::Simulator* simulator);

  /**
   * transform given expression to affine form
   * @param node expression
   * @param parameter_map
   *          map of parameter to be added parameter for new dummy variables
   */
  value_t transform(symbolic_expression::node_sptr &node, parameter_map_t &parameter_map);
  ///calculate x^y
  AffineOrInteger pow(AffineOrInteger x, AffineOrInteger y);
  affine_t pow(affine_t affine, int exp);
  
  /**
   * Reduce dummy variables
   * @param formulas Formulas to be reduced
   * @param limit the number which the number of dummy variables are to be after reduction
   */
  void reduce_dummy_variables(boost::numeric::ublas::vector<affine_t> &formulas, int limit);

  virtual ~AffineTransformer();  

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
  
private:

  typedef boost::bimaps::bimap<parameter_t, int >
    parameter_idx_map_t;
  typedef parameter_idx_map_t::value_type parameter_idx_t;


  AffineTransformer();
  AffineTransformer(const AffineTransformer& rhs);
  AffineTransformer& operator=(const AffineTransformer& rhs);

  void invalid_node(symbolic_expression::Node& node);

  simulator::Simulator* simulator_;

  static interval::AffineTransformer* affine_translator_;

  AffineOrInteger current_val_;
  parameter_idx_map_t parameter_idx_map_;
  int epsilon_index;
};

} //namespace interval
} //namespace hydla

#endif // include guard
