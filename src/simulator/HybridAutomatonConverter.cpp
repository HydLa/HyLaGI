#include "HybridAutomatonConverter.h"
#include "Backend.h"
#include "Logger.h"
#include "PhaseSimulator.h"
#include "SignalHandler.h"
#include "SymbolicTrajPrinter.h"
#include "TimeOutError.h"
#include "Timer.h"
#include "Utility.h"
#include "ValueModifier.h"
#include <limits.h>
#include <stdio.h>
#include <string>

#include <boost/algorithm/string.hpp>

using namespace std;

namespace hydla {
namespace simulator {

using namespace std;
using namespace symbolic_expression;

HybridAutomatonConverter::HybridAutomatonConverter(Opts &opts)
    : Simulator(opts) {}

HybridAutomatonConverter::~HybridAutomatonConverter() {}

phase_result_sptr_t HybridAutomatonConverter::simulate() {
  if(opts_->init_abstraction){
    engine = mt19937(seed_gen());
  }

  std::string error_str = "";
  make_initial_todo();
  try {
    AutomatonNode *init = new AutomatonNode(result_root_, "init");
    current_automaton.initial_node = init;
    HA_node_list_t created_nodes;
    HA_translate(result_root_, init, created_nodes);
    int automaton_count = 1;
    for (auto automaton : result_automata) {
      cout << "===== Automaton" << automaton_count++ << " =====" << endl;
      automaton.dump(cout);
    }
  } catch (const std::runtime_error &se) {
    error_str += "error ";
    error_str += ": ";
    error_str += se.what();
    error_str += "\n";
    HYDLA_LOGGER_DEBUG_VAR(error_str);
    std::cout << error_str;
  }

  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}

void HybridAutomatonConverter::HA_translate(
    phase_result_sptr_t current, AutomatonNode *current_automaton_node,
    HA_node_list_t created_nodes) {
  io::SymbolicTrajPrinter printer(backend);
  if (signal_handler::interrupted) {
    current->simulation_state = INTERRUPTED;
    return;
  }
  phase_simulator_->apply_diff(*current);
  HYDLA_LOGGER_DEBUG_VAR(*current);
  if (current->todo_list.empty()) {
    // The simulation for this case is terminated
    AutomatonNode *next_node = create_phase_node(current);
    current_automaton_node->add_edge(next_node);
    Automaton result_automaton = current_automaton.clone();
    next_node->remove(); // remove from original Automaton
    result_automata.push_back(result_automaton);
  }
  while (!current->todo_list.empty()) {
    phase_result_sptr_t todo = current->todo_list.front();
    current->todo_list.pop_front();
    profile_vector_->insert(todo);
    if (todo->simulation_state == NOT_SIMULATED) {
      phase_list_t result_list = process_one_todo(todo);
      if (opts_->dump_in_progress) {
        printer.output_one_phase(todo);
      }
    }
    /* TODO: assertion違反が検出された場合の対応 */
    AutomatonNode *next_node =
        transition(current_automaton_node, todo, created_nodes);
    if (next_node == nullptr) {
      HYDLA_LOGGER_DEBUG("A loop is detected");
      HYDLA_LOGGER_DEBUG_VAR(*todo);
      result_automata.push_back(current_automaton.clone());
    } else {
      HA_translate(todo, next_node, created_nodes);
    }
  }
  phase_simulator_->revert_diff(*current);
  current_automaton_node->remove();
}

AutomatonNode *
HybridAutomatonConverter::create_phase_node(phase_result_sptr_t phase) {
  return new AutomatonNode(phase, "Phase " + to_string(phase->id), phase->id);
}

AutomatonNode *
HybridAutomatonConverter::transition(AutomatonNode *current_HA_node,
                                     phase_result_sptr_t phase,
                                     HA_node_list_t &created_nodes) {
  AutomatonNode *next_node = create_phase_node(phase);
  //通常ループの探索
  AutomatonNode *loop_node = detect_loop(next_node, created_nodes);
  //ループの場合
  if (loop_node != NULL) {
    current_HA_node->add_edge(loop_node);
    return nullptr;
  }
  //ループでない場合
  else {
    current_HA_node->add_edge(next_node);
    created_nodes.push_back(next_node);
    return next_node;
  }
}

AutomatonNode *HybridAutomatonConverter::detect_loop(AutomatonNode *new_node,
                                                     HA_node_list_t path) {
  for (auto node : path) {
    if(opts_->init_abstraction){
      if (check_including_abstraction(node, new_node)){
        return node;
      }
    }
    else if (check_including(node, new_node)) {
      return node;
    }
  } 
  return nullptr;               
}

bool HybridAutomatonConverter::check_including(AutomatonNode *larger,
                                               AutomatonNode *smaller) {
  bool include_ret;
  // phase typeの比較

  if (larger->phase->phase_type != smaller->phase->phase_type) {
    // cout << "different phase type :\n\t \"" << larger->id << "\" : \"" <<
    // smaller->id << "\"" << endl;
    return false;
  }
  // phase の変数表の大きさの比較
  if (larger->phase->variable_map.size() !=
      smaller->phase->variable_map.size()) {
    // cout << "different size of variable map :\n\t \"" << larger->id << "\" :
    // \"" << smaller->id << "\"" << endl;
    return false;
  }

  ConstraintStore larger_cons = larger->phase->get_parameter_constraint();
  ConstraintStore smaller_cons = smaller->phase->get_parameter_constraint();
  // compareing set of variables
  backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
                &(larger->phase->current_time), &(larger->phase->variable_map),
                &larger_cons, &(smaller->phase->current_time),
                &(smaller->phase->variable_map), &smaller_cons, &include_ret);
  if (include_ret) {
    // cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n"
    // << endl;
  } else {
    // cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id
    // << "\"" << endl;
  }
  return include_ret;
}


bool HybridAutomatonConverter::check_including_abstraction(AutomatonNode *larger,
                                               AutomatonNode *smaller) {
  // phase typeの比較
  const double included = 1.0;
  if (larger->phase->phase_type != smaller->phase->phase_type) {
    // cout << "different phase type :\n\t \"" << larger->id << "\" : \"" <<
    // smaller->id << "\"" << endl;
    return false;
  }
  // phase の変数表の大きさの比較
  if (larger->phase->variable_map.size() !=
      smaller->phase->variable_map.size()) {
    // cout << "different size of variable map :\n\t \"" << larger->id << "\" :
    // \"" << smaller->id << "\"" << endl;
    return false;
  }

  double inclusion_score = maximize_inclusion(smaller, larger, 2.0, 2000, 600);

  if (inclusion_score >= included) {
    // cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n"
    // << endl;
  } else {
    // cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id
    // << "\"" << endl;
  }
  return inclusion_score >= included;
}

double HybridAutomatonConverter::maximize_inclusion(
                                                    AutomatonNode *current, 
                                                    AutomatonNode *past, 
                                                    double abstractTL, 
                                                    double T0, double T1
                                                   ) {
  double inclusion_score = 0.0;
  const double included = 1.0;
  uniform_real_distribution<> dist(0.0, 1.0);
  ConstraintStore past_cons = past->phase->get_parameter_constraint();
  ConstraintStore current_cons = current->phase->get_parameter_constraint();

  //------------------- 仮置き, ここを焼きなましにする -----------------//
  // ConstraintStore abstracted_parameter_constraint = abstract_cp(current, current->phase->get_parameter_constraint());
  // inclusion_score = max(inclusion_score, calculate_inclusion_score(current, abstracted_parameter_constraint, past, abstracted_parameter_constraint));
  //------------------- 仮置き, ここを焼きなましにする -----------------//

  // 焼きなましによる包含スコアの最大化
  auto abstract_start = chrono::system_clock::now();
  auto elapsed = 0;
  do{
    /// 経過時間を測定
    auto now = chrono::system_clock::now();
    elapsed = chrono::duration_cast<chrono::milliseconds>(now - abstract_start).count();

    double t = elapsed/abstractTL;
    double temp = std::pow(T0, 1-t) * std::pow(T1, t);

    ConstraintStore updated_cons = abstract_cp(current, current_cons);
    double new_inclusion_score = calculate_inclusion_score(current, updated_cons, past, past_cons);

    if(new_inclusion_score >= included){
      return new_inclusion_score;
    }
    double delta = new_inclusion_score - inclusion_score;
    if(delta > 0){
      current_cons = updated_cons;
    }
    else{
      if(exp(delta/temp) > dist(engine)){ ///スコアが悪くなる抽象化を一定の確率で受け入れる
        current_cons = updated_cons;
      }
    }
    inclusion_score = max(inclusion_score, new_inclusion_score);
  } while(inclusion_score < included && elapsed <= abstractTL*1000);
  
  return inclusion_score;
}

double HybridAutomatonConverter::calculate_inclusion_score(
                                  AutomatonNode *current, ConstraintStore current_param_cons,
                                  AutomatonNode *past, ConstraintStore past_param_cons 
                                ){
  double ret = 0.0;
  backend->call("calculateInclusionScore", true, 6, "vlnmvtcsnvlnmvtcsn", "db",
                &(past->phase->current_time), &(past->phase->variable_map),
                &past_param_cons, &(current->phase->current_time),
                &(current->phase->variable_map), &current_param_cons, &ret);
  return ret;
}

ConstraintStore HybridAutomatonConverter::abstract_cp(
                                                      AutomatonNode *state, 
                                                      ConstraintStore param_cons
                                                    ){
  variable_map_t vmap = state->phase->variable_map;
  vector<variable_t> abstractCandidate;
  ConstraintStore ret_param_cons;
  
  if(param_cons.size() >= 1)
    backend->call("abstractCP", true, 1, "csn", "cs", &param_cons, &ret_param_cons);
  return param_cons; //後で ret_param_cons に書き換え
}

} // namespace simulator
} // namespace hydla
