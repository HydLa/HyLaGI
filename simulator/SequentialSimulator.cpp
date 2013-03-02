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

SequentialSimulator::SequentialSimulator(Opts &opts):Simulator(opts){
}

SequentialSimulator::~SequentialSimulator(){}
/**
 * 与えられた解候補モジュール集合を元にシミュレーション実行をおこなう
 */
phase_result_const_sptr_t SequentialSimulator::simulate()
{
  std::string error_str;
  int error_sum = 0;
  bool is_safe = true;
  while(!todo_stack_.empty()) {
    simulation_todo_sptr_t todo(pop_simulation_phase());
    {
      if( opts_->max_phase >= 0 && todo->parent->step >= opts_->max_phase){
        todo->parent->cause_of_termination = simulator::STEP_LIMIT;
        continue;
      }
      HYDLA_LOGGER_PHASE("--- Current Todo ---");
      HYDLA_LOGGER_PHASE(todo);
    }

    try{
      timer::Timer phase_timer;
      PhaseSimulator::result_list_t phases = phase_simulator_->calculate_phase_result(todo);
      
      hydla::output::SymbolicTrajPrinter printer;
      if((opts_->max_phase_expanded <= 0 || phase_simulator_->get_phase_sum() < opts_->max_phase_expanded)){
        for(unsigned int i = 0; i < phases.size(); i++)
        {
          phase_result_sptr_t& phase = phases[i];
          HYDLA_LOGGER_PHASE("--- Result Phase", i+1 , "/", phases.size(), " ---");
          HYDLA_LOGGER_PHASE(phase);

          if(!opts_->nd_mode && i > 0)
          {
            phase->cause_of_termination = NOT_SELECTED;
            continue;
          }
          
          if(phase->cause_of_termination == ASSERTION)
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
              n_todo->parent->cause_of_termination = NOT_SELECTED;
            }
            else
            {
              push_simulation_todo(n_todo);
              HYDLA_LOGGER_PHASE("--- Next Todo", i+1 , "/", phases.size(), ", ", j+1, "/", next_todos.size(), " ---");
              HYDLA_LOGGER_PHASE(n_todo);
            }
          }
        }
      }
      
      if(!is_safe && opts_->stop_at_failure){
        HYDLA_LOGGER_PHASE("%% Failure of assertion is detected");
        // assertion違反の場合が見つかったので，他のシミュレーションを中断して終了する
        while(!todo_stack_.empty()) {
          simulation_todo_sptr_t tmp_state(pop_simulation_phase());
          tmp_state->parent->cause_of_termination = OTHER_ASSERTION;
        }
      }
    }
    catch(const hydla::timeout::TimeOutError &te)
    {
      // タイムアウト発生
      HYDLA_LOGGER_PHASE(te.what());
      todo->parent->cause_of_termination = TIME_OUT_REACHED;
      todo->parent->parent->children.push_back(todo->parent);
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