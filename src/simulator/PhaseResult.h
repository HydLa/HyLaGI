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
  INTERRUPTED,
  NONE
}CauseForTermination;


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
typedef std::set<ask_t >                                           ask_set_t;
typedef std::set<boost::shared_ptr<symbolic_expression::Always> >  always_set_t;
typedef hierarchy::ModuleSet                              module_set_t;

typedef boost::shared_ptr<PhaseResult>                    phase_result_sptr_t;
typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;

typedef boost::shared_ptr<SimulationJob>                 simulation_job_sptr_t;
typedef std::list<simulation_job_sptr_t>                 todo_list_t;

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

struct FullInformation
{
  always_set_t                 expanded_always;
  ask_set_t                    positive_asks;
  ask_set_t                    negative_asks;
};


struct DiscreteCause{
  ask_t ask;
  bool on_time;
  DiscreteCause(const ask_t &a, bool on):ask(a), on_time(on){}
};

struct CandidateOfNextPP{
  std::vector<DiscreteCause >   causes;
  value_t                       pp_time;
  parameter_map_t               parameter_map;
};

/// map from variables to candidates of next PP whose time is minimum
typedef std::map<std::string, CandidateOfNextPP> next_pp_candidate_map_t;

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
  parameter_map_t              parameter_map;
  ask_set_t                    diff_positive_asks, diff_negative_asks;

  constraint_diff_t            expanded_diff;
  module_diff_t                adopted_module_diff;

  module_set_t                 module_set;
  next_pp_candidate_map_t      next_pp_candidate_map;

  CauseForTermination          cause_for_termination;

  PhaseResult                 *parent;
  phase_result_sptrs_t         children;
  
  todo_list_t                  todo_list;

  PhaseResult();
  
  PhaseResult(const SimulationJob& todo, const CauseForTermination& cause = NOT_SIMULATED);
  ~PhaseResult();

  ask_set_t                    get_all_positive_asks();
  ask_set_t                    get_all_negative_asks();

  void                         set_full_information(FullInformation &info);

private:
  void generate_full_information();

  boost::optional<FullInformation>             full_information;
};

std::ostream& operator<<(std::ostream& s, const PhaseResult& pr);
std::ostream& operator<<(std::ostream& s, const variable_map_t& vm);
std::ostream& operator<<(std::ostream& s, const parameter_map_t& pm);
std::ostream& operator<<(std::ostream& s, const ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const tells_t& a);
std::ostream& operator<<(std::ostream& s, const ConstraintStore& a);
std::ostream& operator<<(std::ostream& s, const change_variables_t& a);

} // namespace simulator
} // namespace hydla 

