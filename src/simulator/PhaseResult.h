#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "DefaultVariable.h"
#include "ValueRange.h"
#include "ModuleSet.h"
#include "ModuleSetContainer.h"
#include "ConstraintStore.h"

#include "Timer.h"

namespace hydla {
namespace simulator {

class DefaultParameter;
class PhaseResult;
struct SimulationTodo;

class ParameterComparator{
  public:
  bool operator()(const DefaultParameter x,const DefaultParameter y) const;
};

/**
 * type for cause of termination of simulation
 */
typedef enum{
  TIME_LIMIT,
  STEP_LIMIT,
  SOME_ERROR,
  INCONSISTENCY,
  ASSERTION,
  OTHER_ASSERTION,
  TIME_OUT_REACHED,
  NOT_UNIQUE_IN_INTERVAL,
  NOT_SELECTED,
  NONE
}CauseForTermination;


/**
 * type of a phase
 */
typedef enum {
  PointPhase,
  IntervalPhase,
} PhaseType;


typedef std::vector<boost::shared_ptr<hydla::parse_tree::Tell> > tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Tell> >    collected_tells_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Always> >  expanded_always_t;
typedef std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     ask_set_t;
typedef ask_set_t                                                positive_asks_t;
typedef ask_set_t                                                negative_asks_t;
typedef std::vector<tells_t>                                     not_adopted_tells_list_t;


typedef boost::shared_ptr<PhaseResult>                    phase_result_sptr_t;
typedef boost::shared_ptr<const PhaseResult>              phase_result_const_sptr_t;
typedef std::vector<phase_result_sptr_t >                 phase_result_sptrs_t;

typedef Value                                             value_t;
typedef ValueRange                                        range_t;
typedef DefaultVariable                                   variable_t;
typedef DefaultParameter                                  parameter_t;

typedef std::map<variable_t, range_t, VariableComparator>                    variable_map_t;

typedef std::map<parameter_t, range_t, ParameterComparator>                   parameter_map_t;

typedef boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_sptr;

typedef std::set<std::string> change_variables_t;

/**
 * A class to express the result of each phase.
 */
class PhaseResult {
public:
  PhaseType                 phase_type;
  int id;
  value_t                   current_time, end_time;

  ConstraintStore             original_constraint_store;
  ConstraintStore             reduced_constraint_store;

  variable_map_t           variable_map;
  parameter_map_t           parameter_map;
  expanded_always_t         expanded_always;
  positive_asks_t           positive_asks;
  negative_asks_t           negative_asks;
  int step;
  hydla::ch::module_set_sptr module_set;

  change_variables_t changed_variables;
  module_set_container_sptr module_set_container;

  CauseForTermination cause_for_termination;
  /// A set of succeeding phases
  phase_result_sptrs_t children;
  /// A preceding phase
  phase_result_sptr_t parent;

  PhaseResult();
  PhaseResult(const SimulationTodo& todo, const CauseForTermination& cause = NONE);
};

std::ostream& operator<<(std::ostream& s, const hydla::simulator::PhaseResult& pr);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::variable_map_t& vm);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::parameter_map_t& pm);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::tells_t& a);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::expanded_always_t& a);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::change_variables_t& a);

} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_
