#pragma once

#include "PhaseResult.h"
#include "RelationGraph.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {

struct CheckConsistencyResult {
  ConstraintStore consistent_store; // Constraints for parameter which make the
                                    // given constraints "consistent"
  ConstraintStore inconsistent_store; // Constraints for parameter which make
                                      // the given constraints "inconsistent"
};

typedef enum {
  ENTAILED,
  CONFLICTING,
  BRANCH_VAR,
  BRANCH_PAR
} CheckEntailmentResult;

class VariableFinder;
class ConstraintDifferenceCalculator;

class ConsistencyChecker {

public:
  ConsistencyChecker(backend_sptr_t back);
  ConsistencyChecker(ConsistencyChecker &);

  virtual ~ConsistencyChecker();

  /**
   * Check whether the constraints related to diff_constraints are consistent or
   * not.
   */
  CheckConsistencyResult check_consistency(RelationGraph &relation_graph,
                                           ConstraintStore &diff_constraints,
                                           const PhaseType &phase,
                                           profile_t &profile,
                                           const asks_t &unknown_asks,
                                           bool following_step);

  /**
   * Get inconsistent module sets in the last check_consistency
   */
  std::vector<module_set_t> get_inconsistent_module_sets();
  std::list<ConstraintStore> get_inconsistent_constraints();

  /**
   * Check whether a guard is entailed or not.
   * If the entailment depends on the condition of variables or parameters,
   * return BRANHC_VAR or BRANCH_PAR. If the return value is BRANCH_PAR, the
   * value of cc_result consists of cases the guard is entailed and cases the
   * guard is not entailed.
   */
  CheckEntailmentResult
  check_entailment(RelationGraph &relation_graph,
                   CheckConsistencyResult &cc_result, const constraint_t &guard,
                   const constraint_t &node_sptr, const asks_t &unknown_asks,
                   const PhaseType &phase, bool following_step,
                   profile_t &profile);

  CheckEntailmentResult check_entailment(variable_map_t &vm,
                                         CheckConsistencyResult &cc_result,
                                         const constraint_t &guard,
                                         const PhaseType &phase,
                                         profile_t &profile);

  void set_prev_map(const variable_map_t *);

  std::vector<variable_map_t> get_result_maps();

  /// reset internal counters
  void reset_count();

  int get_backend_check_consistency_count();

  int get_backend_check_consistency_time();

  void clear_inconsistent_constraints();

  std::map<std::string, int> get_differential_map(const variable_set_t &);

private:
  CheckConsistencyResult
  check_consistency_essential(const ConstraintStore &constraint_store,
                              VariableFinder &, const PhaseType &phase,
                              profile_t &profile, bool following_step);

  CheckEntailmentResult
  check_entailment_essential(CheckConsistencyResult &cc_result,
                             const symbolic_expression::node_sptr &guard,
                             const PhaseType &phase, profile_t &profile);

  /**
   * @todo refactor
   */
  void add_continuity(
      VariableFinder &, const PhaseType &phase,
      const constraint_t &constraint_for_default_continuity = constraint_t());

  void check_consistency_foreach(const ConstraintStore &constraint_store,
                                 RelationGraph &relation_graph,
                                 CheckConsistencyResult &result,
                                 const PhaseType &phase, profile_t &profile,
                                 bool following_step);
  CheckConsistencyResult
  call_backend_check_consistency(const PhaseType &phase,
                                 ConstraintStore tmp_cons = ConstraintStore());
  void send_init_equation(Variable &var, std::string fmt);
  void send_prev_constraint(const Variable &var);

  void send_range_constraint(const Variable &var, const variable_map_t &vm,
                             bool prev_mode);

  backend_sptr_t backend;

  const variable_map_t *prev_map;
  std::vector<variable_map_t> result_maps;
  int backend_check_consistency_count;
  unsigned long int backend_check_consistency_time;
  std::vector<module_set_t> inconsistent_module_sets;
  std::list<ConstraintStore> inconsistent_constraints;
};

} // namespace simulator
} // namespace hydla
