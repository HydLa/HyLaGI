#pragma once

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#include "Variable.h"
#include "ValueRange.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ConstraintStore.h"
#include "Parameter.h"


namespace hydla {
namespace backend {
class Backend;
}

namespace simulator {

class PhaseResult;
class PrevReplacer;

/**
 * type for cause of termination of simulation
 */
typedef enum{
  TIME_LIMIT,
  STEP_LIMIT,
  SOME_ERROR,
  INCONSISTENCY,
  ASSERTION,
  TIME_OUT_REACHED,
  NOT_UNIQUE_IN_INTERVAL,
  NOT_SIMULATED,
  SIMULATED,
  INTERRUPTED,
  NONE
} SimulationState;

/**
 * type of a phase
 */
typedef enum {
  InvalidPhase,
  POINT_PHASE,
  INTERVAL_PHASE
} PhaseType;

typedef std::vector<boost::shared_ptr<symbolic_expression::Tell> > tells_t;
typedef boost::shared_ptr<symbolic_expression::Ask>                ask_t;
typedef std::set<ask_t >                                           asks_t;
typedef std::set<boost::shared_ptr<symbolic_expression::Always> >  always_set_t;
typedef hierarchy::ModuleSet                              module_set_t;

typedef boost::shared_ptr<PhaseResult>                    phase_result_sptr_t;
typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;
typedef std::list<phase_result_sptr_t >                   phase_list_t;

typedef Value                                             value_t;
typedef ValueRange                                        range_t;
typedef Variable                                          variable_t;
typedef Parameter                                         parameter_t;

typedef std::map<variable_t, range_t, VariableComparator>                    variable_map_t;
typedef std::set<variable_t, VariableComparator>                             variable_set_t;

typedef std::map<parameter_t, range_t, ParameterComparator>                   parameter_map_t;
typedef boost::shared_ptr<hierarchy::ModuleSetContainer> module_set_container_sptr;

typedef std::set<std::string> change_variables_t;


typedef std::map<constraint_t, bool>              constraint_diff_t;
typedef std::map<module_set_t::module_t, bool>    module_diff_t;
typedef hierarchy::ModuleSet                      module_set_t;
typedef std::set<module_set_t>                    module_set_set_t;



struct FullInformation
{
  always_set_t                 expanded_always;
  constraints_t                positive_guards;
  constraints_t                negative_guards;
  asks_t                       positive_asks, negative_asks;
};

struct FindMinTimeCandidate
{
  value_t         time;
  bool            on_time;
  std::list<int>             guard_indices;
  std::list<constraint_t>    discrete_guards;
  ConstraintStore parameter_constraint;
  ValueRange     range_by_newton;
  constraint_t   guard_by_newton;
  std::map<constraint_t, constraint_t>  other_guards_to_time_condition;
};

typedef std::list<FindMinTimeCandidate> find_min_time_result_t;

struct DCCandidate
{
  value_t                       time;
  std::map<ask_t, bool>         discrete_asks;
  ConstraintStore               parameter_constraint;

  DCCandidate(const value_t &t,
              const std::map<ask_t, bool> &d,
              const ConstraintStore &p)
    : time(t), discrete_asks(d), parameter_constraint(p)
    {}
  DCCandidate(){}
};

typedef std::list<DCCandidate>                    pp_time_result_t;
typedef std::map<constraint_t, constraint_t>      guard_time_map_t;
/// map from variables to candidates of next PP whose time is minimum
typedef std::map<ask_t, find_min_time_result_t>   next_pp_candidate_map_t;

typedef std::map<std::string, double>  profile_t;

/**
 * A class to express the result of each phase.
 */
class PhaseResult
{
public:
  static backend::Backend *backend;  
  PhaseType                    phase_type;
  int                          id,
                               step;

  value_t                      current_time, end_time;

  variable_map_t               variable_map;
  variable_map_t               prev_map; /// variable map for left-hand limit (for PP) or initial values (for IP)

  ConstraintStore              additional_parameter_constraint; /// use for case analysis
  ConstraintStore              additional_constraint_store; /// use for case analysis
  ConstraintStore              diff_sum;
  variable_set_t               discrete_differential_set;
  
  module_diff_t                module_diff;

  module_set_t                 unadopted_ms;
  module_set_set_t             unadopted_mss;
  std::list<module_set_t>  inconsistent_module_sets;
  std::list<ConstraintStore>   inconsistent_constraints;
  next_pp_candidate_map_t      next_pp_candidate_map;
  guard_time_map_t             guard_time_map;
  ConstraintStore              always_list;

  SimulationState              simulation_state;
  PhaseResult                 *parent;
  phase_result_sptrs_t         children;
  phase_list_t                  todo_list;

  // trigger conditions
  std::map<ask_t, bool>        discrete_asks;
  std::set<constraint_t>        discrete_guards;
  
  profile_t                    profile;

  PhaseResult();  
  ~PhaseResult();

  asks_t                    get_diff_positive_asks()const;
  asks_t                    get_diff_negative_asks()const;
  void                      add_diff_positive_asks(const asks_t &asks);
  void                      add_diff_negative_asks(const asks_t &asks);
  void                      add_diff_positive_ask(const ask_t &ask);
  void                      add_diff_negative_ask(const ask_t &ask);
  asks_t                    get_all_positive_asks()const;
  asks_t                    get_all_negative_asks()const;
  constraints_t             get_all_positive_guards()const;
  constraints_t             get_all_negative_guards()const;
  void                      set_parameter_constraint(const ConstraintStore &cons);
  ConstraintStore           get_parameter_constraint()const;
  void                      add_parameter_constraint(const constraint_t &cons);
  void                      add_parameter_constraint(const ConstraintStore &cons);
  std::vector<parameter_map_t> get_parameter_maps()const;
  void                         set_full_information(FullInformation &info);
  inline bool                  in_following_step(){return parent && parent->parent && parent->parent->parent;}

  std::string get_string() const;
	std::string get_vm_string() const;
	std::string get_pc_string() const;

private:
  void generate_full_information() const;

  asks_t                   diff_positive_asks, diff_negative_asks;
  mutable ConstraintStore                      parameter_constraint;
  mutable boost::optional<std::vector<parameter_map_t> >     parameter_maps;
  mutable boost::optional<FullInformation>             full_information;
};

std::ostream& operator<<(std::ostream& s, const PhaseResult& pr);
std::ostream& operator<<(std::ostream& s, const FindMinTimeCandidate& pr);
std::ostream& operator<<(std::ostream& s, const variable_map_t& vm);
std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const asks_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const ConstraintStore& a);
std::ostream& operator<<(std::ostream& s, const change_variables_t& a);
std::ostream& operator<<(std::ostream& s, const pp_time_result_t& n);

} // namespace simulator
} // namespace hydla
