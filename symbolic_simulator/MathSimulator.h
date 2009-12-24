#ifndef _INCLUDED_MATH_SIMULATOR_H_
#define _INCLUDED_MATH_SIMULATOR_H_

#include <string>

#include "ParseTree.h"

#include "mathlink_helper.h"
#include "Simulator.h"

#include "SymbolicVariable.h"
#include "SymbolicValue.h"
#include "SymbolicTime.h"

namespace hydla {
namespace symbolic_simulator {

typedef hydla::simulator::VariableMap<SymbolicVariable, SymbolicValue> variable_map_t;
typedef hydla::simulator::PhaseState<SymbolicVariable, SymbolicValue, SymbolicTime> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef hydla::simulator::Simulator<phase_state_t> simulator_t;

class MathSimulator : public simulator_t
{
public:

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
  } OutputFormat;

  typedef struct Opts_ {
    std::string mathlink;
    bool debug_mode;
    std::string max_time;
    bool profile_mode;
    bool parallel_mode;
    OutputFormat output_format;
  } Opts;

  MathSimulator(const Opts& opts);
  virtual ~MathSimulator();

  /**
   * Point PhaseÇÃèàóù
   */
  virtual bool point_phase(const module_set_sptr& ms, 
                           const phase_state_const_sptr& state);
  
  /**
   * Interval PhaseÇÃèàóù
   */
  virtual bool interval_phase(const module_set_sptr& ms, 
                              const phase_state_const_sptr& state);

private:
  /**
   * èâä˙âªèàóù
   */
  virtual void do_initialize();

  Opts     opts_;
  MathLink ml_; 
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_MATH_SIMULATOR_H_
