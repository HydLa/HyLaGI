#ifndef _INCLUDED_SYMBOLIC_TYPES_H_
#define _INCLUDED_SYMBOLIC_TYPES_H_

#include "ParseTree.h"

#include "Simulator.h"
#include "DefaultVariable.h"

#include "SymbolicValue.h"
#include "SymbolicTime.h"
#include "SymbolicParameter.h"
#include "SymbolicValueRange.h"

#include "Types.h"

using namespace hydla::simulator;

namespace hydla {
namespace symbolic_simulator {

  typedef simulator::DefaultVariable            variable_t;
  typedef SymbolicParameter                     parameter_t;
  typedef SymbolicValue                         value_t;
  typedef SymbolicTime                          time_t;
  typedef SymbolicValue                         value_t;
  typedef SymbolicValueRange                    value_range_t;
  typedef hydla::simulator::VariableMap<variable_t, 
                                        value_t> variable_map_t;
  typedef hydla::simulator::VariableMap<parameter_t, 
                                        value_t> parameter_map_t;

  typedef struct SymbolicPhaseState: public hydla::simulator::PhaseState<variable_t, 
                                       value_t, 
                                       time_t>{
    parameter_map_t parameter_map;
  }phase_state_t;

  typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
  typedef hydla::simulator::Simulator<phase_state_t> simulator_t;
  
  
  typedef enum CalculateClosureResult_ {
    CC_TRUE,
    CC_FALSE,
    CC_BRANCH
  } CalculateClosureResult;

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
  } OutputFormat;
  
  typedef enum OutputStyle_ {
    styleTree,
    styleList,
  } OutputStyle;
  
  typedef enum Mode_{
    ContinuousMode,
    DiscreteMode,
  } Mode;

  typedef struct Opts_ {
    std::string mathlink;
    bool debug_mode;
    std::string max_time; //TODO: time_t‚É‚·‚é
    bool nd_mode;
    OutputStyle output_style;
    bool interactive_mode;
    bool profile_mode;
    bool parallel_mode;
    OutputFormat output_format;
    time_t output_interval;
    int             output_precision;
    int approx_precision;
    std::string solver;
  } Opts;

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_TYPES_H_
