#pragma once

#include "RelationGraph.h"

namespace hydla {
namespace simulator {

class ConstraintStore;

class ConstraintDifferenceCalculator{

public:
  typedef hierarchy::ModuleSet module_set_t;

  /*
   * Set which constraints is changing.
   */
  void set_changing_constraints(const ConstraintStore& constraints);

  /**
   * Get connected constraints which are changed
   * @parameter constraints_vector for output
   */
  ConstraintStore get_changing_constraints();
  
  /**
   * return whether consistency of constraint_store has to be checked
   */
  bool is_changing(const ConstraintStore constraint_store);

  bool is_changing(const constraint_t constraint);

  /**
   * return whether variable is related to changing connected constraints
   */
  bool is_changing(const Variable& variable);

  /**
   * clear changing_connected_constraints_index_set
   */
  void clear_changing();

  /**
   * return whether all variables of constraint are continuous
   */
  bool is_continuous(const constraint_t constraint);

  void set_relation_graph(const boost::shared_ptr<RelationGraph> graph);

private:
  ConstraintStore changing_constraints_;
  boost::shared_ptr<RelationGraph> relation_graph_;
};

} //namespace simulator
} //namespace hydla