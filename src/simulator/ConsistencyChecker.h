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

typedef enum{
  ENTAILED,
  CONFLICTING,
  BRANCH_VAR,
  BRANCH_PAR
} CheckEntailmentResult;


class VariableFinder;
class ConstraintDifferenceCalculator;

class ConsistencyChecker{

public:


  ConsistencyChecker(backend_sptr_t back);
  ConsistencyChecker(ConsistencyChecker&);

  virtual ~ConsistencyChecker();

  CheckConsistencyResult check_consistency(RelationGraph &relation_graph, ConstraintStore &diff_constraints, const PhaseType& phase, profile_t &profile);

  /**
   * Get inconsistent module sets in the last check_consistency
   */
  std::vector<module_set_t> get_inconsistent_module_sets();

  void add_continuity(const VariableFinder&, const PhaseType &phase);

  /**
   * Check whether a guard is entailed or not.
   * If the entailment depends on the condition of variables or parameters, return BRANHC_VAR or BRANCH_PAR.
   * If the return value is BRANCH_PAR, the value of cc_result consists of cases the guard is entailed and cases the guard is not entailed.
   */
  CheckEntailmentResult check_entailment(
    RelationGraph &relation_graph,
    CheckConsistencyResult &cc_result,
    const ask_t &guard,
    const PhaseType &phase,
    profile_t &profile
    );

  void set_prev_map(const variable_map_t *);


  std::vector<variable_map_t> get_result_maps();

  /// reset internal counters
  void reset_count();

  int get_backend_check_consistency_count();

  int get_backend_check_consistency_time();


private:
  CheckConsistencyResult check_consistency(const ConstraintStore& constraint_store,   const VariableFinder&, const PhaseType& phase, profile_t &profile);
  void check_consistency(const ConstraintStore& constraint_store, RelationGraph &relation_graph, module_set_t &module_set, CheckConsistencyResult &result, const PhaseType& phase, profile_t &profile);
  CheckConsistencyResult call_backend_check_consistency(const PhaseType &phase);
  std::map<std::string, int> get_differential_map(variable_set_t &);
  void send_init_equation(Variable &var, std::string fmt);
  void send_prev_constraint(Variable &var);

  backend_sptr_t backend;
  const variable_map_t *prev_map;
  std::vector<variable_map_t> result_maps;
  int backend_check_consistency_count;
  int backend_check_consistency_time;
  std::vector<module_set_t> inconsistent_module_sets;
};


} //namespace simulator
} //namespace hydla
