#include "SequentialSimulator.h"
#include "../common/TimeOutError.h"
#include "Logger.h"
#include "PhaseSimulator.h"
#include "SignalHandler.h"
#include "Timer.h"

namespace hydla {
namespace simulator {

using namespace std;

SequentialSimulator::SequentialSimulator(Opts &opts)
    : Simulator(opts), printer(backend) {}

SequentialSimulator::~SequentialSimulator() {}

// ここから、メインのシミュレーションが始まる
phase_result_sptr_t SequentialSimulator::simulate() {
  std::string error_str = "";
  make_initial_todo();

  try {
    // dfsが再帰的に呼ばれて、PP/IPのシミュレーションが行われる
    dfs(result_root_);
  } catch (const std::exception &se) {
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

void SequentialSimulator::dfs(phase_result_sptr_t current) {
  auto detail = logger::Detail(__FUNCTION__);

  HYDLA_LOGGER_DEBUG_VAR(*current);
  if (signal_handler::interrupted) {
    current->simulation_state = INTERRUPTED;
    return;
  }

  phase_simulator_->apply_diff(*current);
  // current->todo_listは、次に探索すべきphaseResultのリスト
  while (!current->todo_list.empty()) {
    phase_result_sptr_t todo = current->todo_list.front();
    current->todo_list.pop_front();
    profile_vector_->insert(todo);
    if (todo->simulation_state == NOT_SIMULATED) {
      //ここから、次に探索すべきphaseResultを導出する。
      process_one_todo(todo);
      if (opts_->dump_in_progress) {
        printer.output_one_phase(todo, "------ In Progress ------");
      }
    }

    dfs(todo);
    if (!opts_->nd_mode || (opts_->stop_at_failure && assertion_failed)) {
      omit_following_todos(current);
      break;
    }
  }

  phase_simulator_->revert_diff(*current);
}

void SequentialSimulator::omit_following_todos(phase_result_sptr_t current) {
  while (!current->todo_list.empty()) {
    phase_result_sptr_t not_selected_children = current->todo_list.front();
    current->todo_list.pop_front();
    if (not_selected_children->simulation_state != SIMULATED) {
      current->children.push_back(not_selected_children);
    }
    not_selected_children->simulation_state = NOT_SIMULATED;
  }
}

} // namespace simulator
} // namespace hydla
