#ifndef _INCLUDED_MATH_SIMULATOR_H_
#define _INCLUDED_MATH_SIMULATOR_H_

namespace hydla {

class MathSimulator
{
 public:
  MathSimulator();
  ~MathSimulator();

  bool simulate(const char mathlink[], 
		const char interlanguage[],
		bool debug_mode,
		bool profile_mode);

};
}

#endif //_INCLUDED_MATH_SIMULATOR_H_
