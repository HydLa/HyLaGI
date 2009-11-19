#ifndef _INCLUDED_MATH_SIMULATOR_H_
#define _INCLUDED_MATH_SIMULATOR_H_

#include <string>

#include "mathlink_helper.h"
#include "Simulator.h"

#include "SymbolicVariable.h"
#include "SymbolicValue.h"
#include "SymbolicTime.h"

namespace hydla {
class HydLaParser;

namespace symbolic_simulator {

typedef simulator::VariableMap<SymbolicVariable, SymbolicValue> variable_map_t;
typedef simulator::PhaseState<variable_map_t, SymbolicTime> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef simulator::Simulator<phase_state_t> simulator_t;

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

  MathSimulator();
  virtual ~MathSimulator();

  bool simulate(HydLaParser& parser, 
                boost::shared_ptr<hydla::ch::ModuleSetContainer> msc,
                Opts& opts);

//  virtual bool test_module_set(hydla::ch::module_set_sptr ms);


  /**
   * Point Phaseの処理
   */
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);
  
  /**
   * Interval Phaseの処理
   */
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, phase_state_sptr& state);

private:
  void init(Opts& opts);

  bool debug_mode_;
  MathLink ml_; 
  //boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_;
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_MATH_SIMULATOR_H_
