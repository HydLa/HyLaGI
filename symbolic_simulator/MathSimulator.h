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

typedef hydla::simulator::Simulator<SymbolicVariable, SymbolicValue, SymbolicTime> simulator_t;

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

protected:
  virtual bool point_phase(hydla::ch::module_set_sptr& ms, State* state);
  virtual bool interval_phase(hydla::ch::module_set_sptr& ms, State* state);

private:
  void init(Opts& opts);

  bool debug_mode_;
  MathLink ml_; 
  //boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_;
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_MATH_SIMULATOR_H_
