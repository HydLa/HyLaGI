#include "PhaseResult.h"
#include "DefaultParameter.h"
#include "Simulator.h"

namespace hydla {
namespace simulator {


PhaseResult::PhaseResult():cause_of_termination(NONE)
{
}



PhaseResult::PhaseResult(const SimulationTodo& todo, const CauseOfTermination& cause):
  phase(todo.phase),
  current_time(todo.current_time),
  parameter_map(todo.parameter_map),
  positive_asks(todo.positive_asks),
  negative_asks(todo.negative_asks),
  step(todo.parent->step + 1),
  changed_variables(todo.changed_variables),
  cause_of_termination(cause),
  parent(todo.parent)
{
}

} // namespace simulator
} // namespace hydla 
