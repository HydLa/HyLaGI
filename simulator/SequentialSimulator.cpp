#include "SequentialSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "Dumpers.h"

namespace hydla {
namespace simulator {

using namespace std;

SequentialSimulator::SequentialSimulator(Opts &opts):BatchSimulator(opts)
{
}

SequentialSimulator::~SequentialSimulator()
{
}

phase_result_const_sptr_t SequentialSimulator::simulate()
{
  std::string error_str;
  simulation_todo_sptr_t init_todo = make_initial_todo();
  todo_stack_->push_todo(init_todo);
  int error_sum = 0;
  while(!todo_stack_->empty()) 
  {
    if((opts_->max_phase_expanded > 0 && phase_simulator_->get_phase_sum() >= opts_->max_phase_expanded))
    {
      break;
    }
    try
    {
      simulation_todo_sptr_t todo(todo_stack_->pop_todo());
      process_one_todo(todo);
    }
    catch(const std::runtime_error &se)
    {
      error_str += "error ";
      error_str += ('0' + (++error_sum));
      error_str += ": ";
      error_str += se.what();
      error_str += "\n";
      HYDLA_LOGGER_PHASE(se.what());
    }
  }
  
  if(!error_str.empty()){
    std::cout << error_str;
  }
  
  HYDLA_LOGGER_PHASE("%% simulation ended");
  return result_root_;
}


} // simulator
} // hydla
