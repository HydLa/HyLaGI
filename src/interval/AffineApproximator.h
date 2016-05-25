#pragma once

#include "Simulator.h"
#include "AffineTreeVisitor.h"

namespace hydla {
namespace interval {


typedef symbolic_expression::node_sptr        node_sptr;
typedef simulator::Parameter                  parameter_t;
typedef simulator::Value                      value_t;
typedef simulator::ValueRange                 range_t;
typedef simulator::parameter_map_t            parameter_map_t;
typedef simulator::Variable                   variable_t;
typedef simulator::variable_map_t             variable_map_t;

/**
 * A class which approximates symbolic formulas as affine forms
 */
class AffineApproximator{
  public:

  static AffineApproximator* get_instance();
  
  void set_simulator(simulator::Simulator* simulator);


  /**
   * Approximate given variable in given variable map conserving given condition
   */
  void approximate(const simulator::variable_set_t &vars_to_approximate, variable_map_t& variable_map,  parameter_map_t &parameter_map, value_t &time);

  /**
   * Approximate time
   */
  void approximate_time(value_t &time, const variable_map_t &ip_map, variable_map_t& prev_map,  parameter_map_t &parameter_map, node_sptr condition_to_be_conserved = node_sptr());

  virtual ~AffineApproximator();  

private:

  /**
   * Reduce dummy variables
   * @param formulas Formulas to be reduced
   * @param limit the number which the number of dummy variables are to be after reduction
   */
  void reduce_dummy_variables(boost::numeric::ublas::vector<affine_t> &formulas, int limit);


  value_t translate_into_symbolic_value(const affine_t &affine_value, parameter_map_t &parameter_map);
  AffineApproximator();
  AffineApproximator(const AffineApproximator& rhs);
  AffineApproximator& operator=(const AffineApproximator& rhs);

  void invalid_node(symbolic_expression::Node& node);

  simulator::Simulator* simulator;

  static interval::AffineApproximator* affine_translator;


  parameter_idx_map_t parameter_idx_map;
  int epsilon_index;
};

} //namespace interval
} //namespace hydla
