#ifndef _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_
#define _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_

#include <vector>

#include <boost/shared_ptr.hpp>

#include "DefaultVariable.h"
#include "ValueRange.h"
#include "ModuleSet.h"

#include "Timer.h"

namespace hydla {
namespace simulator {

struct DefaultParameter;
struct PhaseResult;
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
}CauseOfTermination;


/**
 * type of a phase
 */
typedef enum Phase_ {
  PointPhase,
  IntervalPhase,
} Phase;


typedef std::vector<boost::shared_ptr<hydla::parse_tree::Node> > constraints_t;
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

typedef boost::shared_ptr<Value>                          value_t;
typedef ValueRange                                        range_t;
typedef DefaultVariable                                   variable_t;
typedef DefaultParameter                                  parameter_t;
typedef value_t                                           time_t;

typedef std::map<variable_t, range_t, VariableComparator>                    variable_map_t;

typedef std::map<parameter_t, range_t, ParameterComparator>                   parameter_map_t;

/**
 * A struct to express the result of each phase.
 */
struct PhaseResult {
  Phase                     phase;
  int id;
  time_t                    current_time, end_time;
  variable_map_t            variable_map;
  parameter_map_t           parameter_map;
  expanded_always_t         expanded_always;
  positive_asks_t           positive_asks;
  negative_asks_t           negative_asks;
  int step;
  hydla::ch::module_set_sptr module_set;

  std::set<std::string> changed_variables;

  CauseOfTermination cause_of_termination;
  /// A set of succeeding phases
  phase_result_sptrs_t children;
  /// A preceding phase
  phase_result_sptr_t parent;

  PhaseResult();
  PhaseResult(const SimulationTodo& todo, const CauseOfTermination& cause = NONE);
};

std::ostream& operator<<(std::ostream& s, const hydla::simulator::PhaseResult& pr);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::variable_map_t& vm);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::parameter_map_t& pm);


std::ostream& operator<<(std::ostream& s, const hydla::simulator::constraints_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::ask_set_t& a);
std::ostream& operator<<(std::ostream& s, const hydla::simulator::tells_t& a);

std::ostream& operator<<(std::ostream& s, const hydla::simulator::expanded_always_t& a);


} // namespace simulator
} // namespace hydla 

#endif // _INCLUDED_HYDLA_SIMULATOR_PHASE_RESULT_H_
