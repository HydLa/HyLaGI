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
namespace simulator {

class PhaseResult;
struct SimulationJob;

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
}SimulationState;


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
typedef std::list<phase_result_sptr_t >                   todo_list_t;

typedef Value                                             value_t;
typedef ValueRange                                        range_t;
typedef Variable                                          variable_t;
typedef Parameter                                         parameter_t;

typedef std::map<variable_t, range_t, VariableComparator>                    variable_map_t;

typedef std::map<parameter_t, range_t, ParameterComparator>                   parameter_map_t;

typedef boost::shared_ptr<hierarchy::ModuleSetContainer> module_set_container_sptr;

typedef std::set<std::string> change_variables_t;


typedef std::map<constraint_t, bool> constraint_diff_t;
typedef std::map<module_set_t::module_t, bool>     module_diff_t;
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
  parameter_map_t parameter_map;
};

typedef std::list<FindMinTimeCandidate> find_min_time_result_t;

struct DCCandidate{
  value_t                       time;
  std::map<ask_t, bool>         discrete_asks;
  parameter_map_t               parameter_map;

  DCCandidate(const value_t                &t,
              const std::map<ask_t, bool> &d,
//              const std::map<constraint_t, bool> &g,
              const parameter_map_t &p)
                :time(t),
                 discrete_asks(d),
//                 discrete_guards(g),
                 parameter_map(p)
    {}
  DCCandidate(){}
};

typedef std::list<DCCandidate>            pp_time_result_t;
typedef std::map<constraint_t, constraint_t> guard_time_map_t;
/// map from variables to candidates of next PP whose time is minimum
typedef std::map<ask_t, find_min_time_result_t> next_pp_candidate_map_t;

typedef std::map<std::string, unsigned int>       profile_t;

/**
 * A class to express the result of each phase.
 */
class PhaseResult {

public:
  PhaseType                    phase_type;
  int                          id,
                               step;
  
  value_t                      current_time, end_time;

  variable_map_t               variable_map;
  variable_map_t               prev_map; /// variable map for left-hand limit (for PP) or initial values (for IP)
  parameter_map_t              parameter_map;
  asks_t                       diff_positive_asks, diff_negative_asks;
  constraints_t                diff_positive_guards, diff_negative_guards;
  ConstraintStore              initial_constraint_store; /// 暫定的に場合分けとかで使う.TODO:別の方法を考える
  ConstraintStore              diff_sum;
  
  module_diff_t                module_diff;

  module_set_t                 unadopted_ms;
  module_set_set_t             unadopted_mss;
  next_pp_candidate_map_t      next_pp_candidate_map;
  guard_time_map_t             guard_time_map;
  ConstraintStore              always_list;

  SimulationState              simulation_state;
  PhaseResult                 *parent;
  phase_result_sptrs_t         children;
  todo_list_t                  todo_list;

  // trigger conditions
  std::map<ask_t, bool>        discrete_asks;
  std::list<constraint_t>      discrete_guards;
  
  profile_t                    profile;

  PhaseResult();  
  ~PhaseResult();

  asks_t                    get_all_positive_asks();
  asks_t                    get_all_negative_asks();
  constraints_t             get_all_positive_guards();
  constraints_t             get_all_negative_guards();

  void                         set_full_information(FullInformation &info);
  inline bool                  in_following_step(){return parent && parent->parent && parent->parent->parent;}
private:
  void generate_full_information();

  boost::optional<FullInformation>             full_information;
};

std::ostream& operator<<(std::ostream& s, const PhaseResult& pr);
std::ostream& operator<<(std::ostream& s, const variable_map_t& vm);
std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const asks_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const ConstraintStore& a);
std::ostream& operator<<(std::ostream& s, const change_variables_t& a);
std::ostream& operator<<(std::ostream& s, const pp_time_result_t& n);

} // namespace simulator
} // namespace hydla
