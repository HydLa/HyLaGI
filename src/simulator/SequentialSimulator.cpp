#include "SequentialSimulator.h"
#include "Timer.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"
#include "../common/HydLaError.h"
#include "SignalHandler.h"
#include "Logger.h"
#include "../interval/IntervalNewtonError.h"

namespace hydla {
namespace simulator {

using namespace std;

SequentialSimulator::SequentialSimulator(Opts &opts)
  : Simulator(opts), printer(backend)
{
}

SequentialSimulator::~SequentialSimulator()
{
}

phase_result_sptr_t SequentialSimulator::simulate()
{
  std::string error_str = "";
  make_initial_todo();

  try
  {
    dfs(result_root_);
  }
  catch(const std::exception &se)
  {
    error_str += "error ";
    error_str += ": ";
    error_str += se.what();
    error_str += "\n";
    HYDLA_LOGGER_DEBUG_VAR(error_str);
    std::cout << error_str;
    exit_status = EXIT_FAILURE;
  }

  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}

void SequentialSimulator::dfs(phase_result_sptr_t current)
{
  auto detail = logger::Detail(__FUNCTION__);

  HYDLA_LOGGER_DEBUG_VAR(*current);
  if (signal_handler::interrupted)
  {
    current->simulation_state = INTERRUPTED;
    return;
  }

  phase_simulator_->apply_diff(*current);

  while (!current->todo_list.empty())
  {
    phase_result_sptr_t todo = current->todo_list.front();
    current->todo_list.pop_front();

    phase_result_sptr_t todo_clone = boost::make_shared<PhaseResult>();
    *todo_clone = *todo;

    profile_vector_->insert(todo_clone);
    if (todo_clone->simulation_state == NOT_SIMULATED)
    {
      try
      {
        process_one_todo(todo_clone);
      }
      catch(const interval::IntervalNewtonError& e)
      {
	ConstraintStore constraintsA, constraintsB;

	HYDLA_LOGGER_DEBUG("Invalid Interval Newton Error: ");
	for(const auto& parameterMap : todo_clone->get_parameter_maps())
	{
	  for(const auto& paramRange: parameterMap)
	  {
            parameter_t param = paramRange.first;
	    ValueRange range = paramRange.second;
            if(param.get_name() == "l")
	    {
              const Value m = (range.get_lower_bound().value + range.get_upper_bound().value) / Value(2);
	      ValueRange rangeA = range, rangeB = range;
	      rangeA.set_upper_bound(m, true);
	      //rangeB.set_lower_bound(m, false);
	      rangeB.set_lower_bound(m, true);

	      ConstraintStore par_consA = rangeA.create_range_constraint(node_sptr(new symbolic_expression::Parameter(param.get_name(), param.get_differential_count(), param.get_phase_id())));
	      constraintsA.add_constraint_store(par_consA);

	      ConstraintStore par_consB = rangeB.create_range_constraint(node_sptr(new symbolic_expression::Parameter(param.get_name(), param.get_differential_count(), param.get_phase_id())));
	      constraintsB.add_constraint_store(par_consB);
	    }
	    else
	    {
	      ConstraintStore par_cons = range.create_range_constraint(node_sptr(new symbolic_expression::Parameter(param.get_name(), param.get_differential_count(), param.get_phase_id())));
	      constraintsA.add_constraint_store(par_cons);
	      constraintsB.add_constraint_store(par_cons);
	    }
	  }
	}
	HYDLA_LOGGER_DEBUG("Par Cons A: ", constraintsA);
	HYDLA_LOGGER_DEBUG("Par Cons B: ", constraintsB);
	CheckConsistencyResult branchConstraints{constraintsA, constraintsB};
	phase_simulator_->push_branch_states(todo, branchConstraints);
	current->todo_list.push_front(todo);

	continue;
      }
      
      if (opts_->dump_in_progress){
        printer.output_one_phase(todo, "------ In Progress ------");
      }
    }
    dfs(todo_clone);

    if (!opts_->nd_mode || (opts_->stop_at_failure && assertion_failed))
    {
      omit_following_todos(current);
      break;
    }
  }

  phase_simulator_->revert_diff(*current);
}

void SequentialSimulator::omit_following_todos(phase_result_sptr_t current)
{
  while (!current->todo_list.empty())
  {
    phase_result_sptr_t not_selected_children = current->todo_list.front();
    current->todo_list.pop_front();
    if (not_selected_children->simulation_state != SIMULATED)
    {
      current->children.push_back(not_selected_children);
    }
    not_selected_children->simulation_state = NOT_SIMULATED;
  }
}

} // namespace simulator
} // namespace hydla
