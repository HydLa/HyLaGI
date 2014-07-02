#include "SequentialSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "SignalHandler.h"

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
  std::string error_str = "";
  simulation_todo_sptr_t init_todo = make_initial_todo();
  todo_stack_->push_todo(init_todo);
  int error_sum = 0;

  while(!todo_stack_->empty() && !signal_handler::interrupted) 
  {
    simulation_todo_sptr_t todo(todo_stack_->pop_todo());
    try
    {
      process_one_todo(todo);
    }
    catch(const std::runtime_error &se)
    {
      error_str += "error ";
      error_str += ('0' + (++error_sum));
      error_str += ": ";
      error_str += se.what();
      error_str += "\n";
      if(signal_handler::interrupted)todo->parent->cause_for_termination = INTERRUPTED;
      HYDLA_LOGGER_DEBUG(se.what());
    }
  }


  if(signal_handler::interrupted){
    while(!todo_stack_->empty())
    {
      simulation_todo_sptr_t todo(todo_stack_->pop_todo());
      todo->parent->cause_for_termination = INTERRUPTED;
      // TODO: restart simulation from each interrupted phase
    }
  }
  
  if(!error_str.empty()){
    std::cout << error_str;
  }

  
  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}


} // simulator
} // hydla
