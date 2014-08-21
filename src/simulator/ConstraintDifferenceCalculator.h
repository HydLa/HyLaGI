#pragma once

#include "RelationGraph.h"
#include "AskRelationGraph.h"
#include "PhaseResult.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

class ConstraintStore;

class ConstraintDifferenceCalculator{

public:
  typedef hierarchy::ModuleSet module_set_t;

  /*
   * Calculate which constraints is difference between a current todo and the parent.
   */
  void calculate_difference_constraints(const simulation_todo_sptr_t todo, const boost::shared_ptr<RelationGraph> relation_graph);

  void add_difference_constraints(const constraint_t constraint, const boost::shared_ptr<RelationGraph> relation_graph);

  /**
   * Get connected constraints which are changed
   */
  ConstraintStore get_difference_constraints();

  /**
   * return whether all variables of constraint are continuous
   */
  bool is_continuous(const simulation_todo_sptr_t todo, const ask_t ask);

  void collect_ask(const boost::shared_ptr<AskRelationGraph> ask_relation_graph,
      const std::vector<ask_t> &discrete_causes,
      const ask_set_t &positive_asks,
      const ask_set_t &negative_asks,
      ask_set_t &unknown_asks);

private:
  ConstraintStore difference_constraints_;

  void set_symmetric_difference(
    const ConstraintStore& parent_constraints,
    const ConstraintStore& current_constraints,
    ConstraintStore& result );

};

} //namespace simulator
} //namespace hydla