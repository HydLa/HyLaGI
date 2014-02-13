#include "BatchSimulator.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

namespace hydla {
namespace simulator {

using namespace std;

BatchSimulator::BatchSimulator(Opts &opts):Simulator(opts)
{
}

BatchSimulator::~BatchSimulator(){}

void BatchSimulator::initialize(const parse_tree_sptr& parse_tree)
{
  Simulator::initialize(parse_tree);
  todo_stack_.reset(new BatchTodoContainer(opts_->search_method, profile_vector_));
}

void BatchSimulator::process_one_todo(simulation_todo_sptr_t& todo)
{
  bool is_safe = true;
  hydla::output::SymbolicTrajPrinter printer(opts_->output_variables, std::cerr);
  if( opts_->max_phase >= 0 && todo->parent->step >= opts_->max_phase){
    todo->parent->cause_for_termination = simulator::STEP_LIMIT;
    return;
  }

  HYDLA_LOGGER_DEBUG("--- Current Todo ---\n", *todo);

  try{
    timer::Timer phase_timer;
    PhaseSimulator::result_list_t phases = phase_simulator_->calculate_phase_result(todo);
    
    for(unsigned int i = 0; i < phases.size(); i++)
    {
      phase_result_sptr_t& phase = phases[i];
      HYDLA_LOGGER_DEBUG("--- Result Phase", i+1 , "/", phases.size(), " ---\n", *phase);

      if(!opts_->nd_mode && i > 0)
      {
        phase->cause_for_termination = NOT_SELECTED;
        continue;
      }

      if(phase->cause_for_termination == ASSERTION)
      {
        is_safe = false;
        if(opts_->stop_at_failure)break;
        else continue;
      }

      PhaseSimulator::todo_list_t next_todos = phase_simulator_->make_next_todo(phase, todo);
      for(unsigned int j = 0; j < next_todos.size(); j++)
      {
        simulation_todo_sptr_t& n_todo = next_todos[j];
        n_todo->elapsed_time = phase_timer.get_elapsed_us() + todo->elapsed_time;
        if(opts_->dump_in_progress){
          printer.output_one_phase(n_todo->parent);
        }
        if(!opts_->nd_mode && j > 0)
        {
          n_todo->parent->cause_for_termination = NOT_SELECTED;
        }
        else
        {
          todo_stack_->push_todo(n_todo);
          HYDLA_LOGGER_DEBUG("--- Next Todo", i+1 , "/", phases.size(), ", ", j+1, "/", next_todos.size(), " ---");
          HYDLA_LOGGER_DEBUG(*n_todo);
        }
      }
    }

    todo->profile["EntirePhase"] += phase_timer.get_elapsed_us();

    if(!is_safe && opts_->stop_at_failure){
      HYDLA_LOGGER_DEBUG("%% Failure of assertion is detected");
      // assertion違反の場合が見つかったので，他のシミュレーションを中断して終了する
      while(!todo_stack_->empty()) {
        simulation_todo_sptr_t tmp_state(todo_stack_->pop_todo());
        phase_result_sptr_t phase(new PhaseResult(*todo, simulator::OTHER_ASSERTION));
        todo->parent->children.push_back(phase);
      }
    }
  }
  catch(const hydla::timeout::TimeOutError &te)
  {
    HYDLA_LOGGER_DEBUG(te.what());
    phase_result_sptr_t phase(new PhaseResult(*todo, simulator::TIME_OUT_REACHED));
    todo->parent->children.push_back(phase);
  }
}


} // simulator
} // hydla