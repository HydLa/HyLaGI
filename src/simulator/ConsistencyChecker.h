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


  ConsistencyChecker(backend_sptr_t back);
  ConsistencyChecker(ConsistencyChecker&);

  virtual ~ConsistencyChecker();

  CheckConsistencyResult check_consistency(RelationGraph &relation_graph, const PhaseType& phase, const bool reuse);


  /**
   * Get inconsistent module sets in the last check_consistency
   */
  std::vector<module_set_t> get_inconsistent_module_sets();

  void add_continuity(const continuity_map_t&, const PhaseType &phase);

  /**
   * Check whether a guard is entailed or not.
   * If the entailment depends on the condition of variables or parameters, return BRANHC_VAR or BRANCH_PAR.
   * If the return value is BRANCH_PAR, the value of cc_result consists of cases the guard is entailed and cases the guard is not entailed.
   */
  CheckEntailmentResult check_entailment(
    RelationGraph &relation_graph,
    CheckConsistencyResult &cc_result,
    const ask_t &guard,
    const PhaseType &phase
    );


private:
  CheckConsistencyResult check_consistency(const ConstraintStore& constraint_store, const continuity_map_t&, const PhaseType& phase);
  CheckConsistencyResult call_backend_check_consistency(const PhaseType &phase);
  backend_sptr_t backend;
  std::vector<module_set_t> inconsistent_module_sets;
};


} //namespace simulator
} //namespace hydla
