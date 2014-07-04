#pragma once

#include "RelationGraph.h"
#include "PhaseResult.h"

namespace hydla {
namespace simulator {

class ConstraintStore;

class ConstraintDifferenceCalculator{

public:
  typedef hierarchy::ModuleSet module_set_t;

  /*
   * Set which constraints is difference.
   */
  void set_difference_constraints(const ConstraintStore& constraints);

  /**
   * Get connected constraints which are changed
   * @parameter constraints_vector for output
   */
  ConstraintStore get_difference_constraints();
  
  /**
   * return whether consistency of constraint_store has to be checked
   */
  bool is_difference(const ConstraintStore constraint_store);

  bool is_difference(const constraint_t constraint);

  /**
   * return whether variable is related to difference connected constraints
   */
  bool is_difference(const Variable& variable);

  /**
   * clear difference_connected_constraints_index_set
   */
  void clear_difference();

  /**
   * return whether all variables of constraint are continuous
   */
  bool is_continuous(const phase_result_sptr_t parent, const constraint_t constraint);

  void set_relation_graph(const boost::shared_ptr<RelationGraph> graph);

private:
  ConstraintStore difference_constraints_;
  boost::shared_ptr<RelationGraph> relation_graph_;
};

} //namespace simulator
} //namespace hydla