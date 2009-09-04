#ifndef _INCLUDED_MATH_SIMULATOR_H_
#define _INCLUDED_MATH_SIMULATOR_H_

namespace hydla {

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
		const char interlanguage[],
		bool debug_mode,
		bool profile_mode,
		bool parallel_mode,
		OutputFormat output_format);

};
}

#endif //_INCLUDED_MATH_SIMULATOR_H_
