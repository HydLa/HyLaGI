#pragma once

#include "PhaseResult.h"
#include "RelationGraph.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

struct CheckConsistencyResult
{
  ConstraintStore consistent_store, inconsistent_store;
};

class ConsistencyChecker{

public:

  typedef enum{
    ENTAILED,
    CONFLICTING,
    BRANCH_VAR,
    BRANCH_PAR
  } CheckEntailmentResult;


  typedef hydla::symbolic_expression::node_sptr node_sptr;

  ConsistencyChecker(backend_sptr_t back);
  ConsistencyChecker(ConsistencyChecker&);

  virtual ~ConsistencyChecker();

  typedef hierarchy::module_set_sptr              modulse_set_sptr;
  typedef std::set< std::string > change_variables_t;

  CheckConsistencyResult check_consistency(const ConstraintStore& constraint_store, const PhaseType& phase);

  CheckConsistencyResult check_consistency(RelationGraph &relation_graph, const PhaseType& phase);


  /**
   * Get inconsistent module set in the last check_consistency
   */
  module_set_t get_inconsistent_module_set();

  void add_continuity(const continuity_map_t&, const PhaseType &phase);

  /**
   * Check whether a guard is entailed or not.
   * If the entailment depends on the condition of variables or parameters, return BRANHC_VAR or BRANCH_PAR.
   * If the return value is BRANCH_PAR, the value of cc_result consists of cases the guard is entailed and cases the guard is not entailed.
   */
  CheckEntailmentResult check_entailment(CheckConsistencyResult &cc_result,
    const symbolic_expression::node_sptr& guard,
    const continuity_map_t& cont_map,
    const PhaseType& phase
    );


private:
  CheckConsistencyResult check_consistency(const ConstraintStore& constraint_store, const continuity_map_t&, const PhaseType& phase);
  CheckConsistencyResult call_backend_check_consistency(const PhaseType &phase);
  backend_sptr_t backend;
  module_set_t inconsistent_module_set;
};


} //namespace simulator
} //namespace hydla
