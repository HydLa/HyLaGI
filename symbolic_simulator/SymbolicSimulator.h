#ifndef _INCLUDED_SYMBOLIC_SIMULATOR_H_
#define _INCLUDED_SYMBOLIC_SIMULATOR_H_

#include <string>

#include "ParseTree.h"

#include "Simulator.h"

#include "../virtual_constraint_solver/mathematica/mathlink_helper.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
#include "../virtual_constraint_solver/mathematica/MathVariable.h"
#include "../virtual_constraint_solver/mathematica/MathValue.h"
#include "../virtual_constraint_solver/mathematica/MathTime.h"

namespace hydla {
namespace symbolic_simulator {

typedef hydla::vcs::mathematica::MathVariable symbolic_variable_t;
typedef hydla::vcs::mathematica::MathValue    symbolic_value_t;
typedef hydla::vcs::mathematica::MathTime     symbolic_time_t;

typedef hydla::simulator::VariableMap<symbolic_variable_t, 
                                      symbolic_value_t> variable_map_t;
typedef hydla::simulator::PhaseState<symbolic_variable_t, 
                                     symbolic_value_t, 
                                     symbolic_time_t> phase_state_t;
typedef boost::shared_ptr<phase_state_t> phase_state_sptr;
typedef hydla::simulator::Simulator<phase_state_t> simulator_t;

class SymbolicSimulator : public simulator_t
{
public:

  typedef enum OutputFormat_ {
    fmtTFunction,
    fmtNumeric,
  } OutputFormat;

  typedef struct Opts_ {
    std::string mathlink;
    bool debug_mode;
    std::string max_time; //TODO: symbolic_time_tÇ…Ç∑ÇÈ
    bool nd_mode;
    bool profile_mode;
    bool parallel_mode;
    OutputFormat output_format;
    symbolic_time_t output_interval;
    int             output_precision;
    int approx_precision;
  } Opts;

  SymbolicSimulator(const Opts& opts);
  virtual ~SymbolicSimulator();

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
  virtual void do_initialize(const parse_tree_sptr& parse_tree);

  void init_module_set_container(const parse_tree_sptr& parse_tree);
  void init_mathlink();

  void output(const symbolic_time_t& time, 
              const variable_map_t& vm);

  /**
   * 
   */
  module_set_container_sptr msc_original_;

  module_set_container_sptr msc_no_init_;
  module_set_container_sptr msc_no_init_discreteask_;

  Opts     opts_;
  MathLink ml_; 

//  hydla::vcs::mathematica::MathematicaVCS vcs_;
};

} // namespace symbolic_simulator
} // namespace hydla

#endif //_INCLUDED_SYMBOLIC_SIMULATOR_H_
