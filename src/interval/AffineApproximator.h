#ifndef _INCLUDED_HYDLA_AFFINE_APPROXIMATOR_H_
#define _INCLUDED_HYDLA_AFFINE_APPROXIMATOR_H_

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
#include "AffineTreeVisitor.h"

namespace hydla {
namespace interval {


typedef hydla::parse_tree::node_sptr          node_sptr;
typedef simulator::Parameter                  parameter_t;
typedef simulator::Value                      value_t;
typedef simulator::ValueRange                 range_t;
typedef simulator::parameter_map_t            parameter_map_t;


/**
 * A class which approximates symbolic formulas as affine forms
 */
class AffineApproximator{
  public:

  static AffineApproximator* get_instance();
  
  void set_simulator(simulator::Simulator* simulator);

  /**
   * Approximate given a expression as an affine form
   * @param node expression
   * @param parameter_map
   *          map of parameter to be added parameter for new dummy variables
   */
  value_t approximate(node_sptr &expr, parameter_map_t &parameter_map, node_sptr condition_to_be_conserved = node_sptr());
  
  /**
   * Reduce dummy variables
   * @param formulas Formulas to be reduced
   * @param limit the number which the number of dummy variables are to be after reduction
   */
  void reduce_dummy_variables(boost::numeric::ublas::vector<affine_t> &formulas, int limit);

  virtual ~AffineApproximator();  

private:
  value_t translate_into_symbolic_value(const affine_t &affine_value, parameter_map_t &parameter_map);
  AffineApproximator();
  AffineApproximator(const AffineApproximator& rhs);
  AffineApproximator& operator=(const AffineApproximator& rhs);

  void invalid_node(parse_tree::Node& node);

  simulator::Simulator* simulator_;

  static interval::AffineApproximator* affine_translator_;


  parameter_idx_map_t parameter_idx_map_;
  int epsilon_index;
};

} //namespace interval
} //namespace hydla

#endif // include guard
