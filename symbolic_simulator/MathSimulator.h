#ifndef _INCLUDED_MATH_SIMULATOR_H_
#define _INCLUDED_MATH_SIMULATOR_H_

#include <string>

#include "mathlink_helper.h"
#include "ModuleSetTester.h"
#include "ModuleSetContainer.h"

namespace hydla {
class HydLaParser;

namespace symbolic_simulator {

class MathSimulator : 
    public hydla::ch::ModuleSetTester
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
  ~MathSimulator();

  bool simulate(HydLaParser& parser, 
                boost::shared_ptr<hydla::ch::ModuleSetContainer> msc,
                Opts& opts);

  virtual bool test_module_set(hydla::ch::module_set_sptr ms);

private:
  void init(Opts& opts);

  MathLink ml_; 
  //boost::shared_ptr<hydla::ch::ModuleSetContainer> module_set_container_;
};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_MATH_SIMULATOR_H_
