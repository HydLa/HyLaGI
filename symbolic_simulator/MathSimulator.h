#ifndef _INCLUDED_MATH_SIMULATOR_H_
#define _INCLUDED_MATH_SIMULATOR_H_

#include <string>

namespace hydla {
class HydLaParser;

namespace symbolic_simulator {

class MathSimulator
{
 public:
  MathSimulator();
  ~MathSimulator();

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
  } OutputFormat;

  bool simulate(const char mathlink[], 
		HydLaParser& parser,
		bool debug_mode,
    std::string max_time,
		bool profile_mode,
		bool parallel_mode,
		OutputFormat output_format);

};

} //namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_MATH_SIMULATOR_H_
