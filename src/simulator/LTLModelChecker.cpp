#include "LTLModelChecker.h"
#include "Automaton.h"
#include "Backend.h"
#include "Logger.h"
#include "PhaseSimulator.h"
#include "SignalHandler.h"
#include "SymbolicTrajPrinter.h"
#include "TimeOutError.h"
#include "Timer.h"
#include "Utility.h"
#include "ValueModifier.h"
#include "VariableFinder.h"
#include <limits.h>
#include <stdio.h>
#include <string>
#include "../parser/never_claim/NeverClaim.h"


using namespace std;

#include "../parser/never_claim/y.tab.hpp"
#include <memory>
std::shared_ptr<hydla::simulator::Automaton> nc_parse();

namespace hydla {
namespace simulator {

using namespace std;
using namespace symbolic_expression;
using namespace never_claim;

LTLModelChecker::LTLModelChecker(Opts &opts)
    : Simulator(opts) {}

LTLModelChecker::~LTLModelChecker() {}

phase_result_sptr_t LTLModelChecker::simulate() {
  std::string error_str = "";
  make_initial_todo();
  try {
    consistency_checker.reset(new ConsistencyChecker(backend));

    // Property Automaton initialize
    int id = 0;
    id_counter = 0;

    auto property = nc_parse();
    property_init = (PropertyNode *)(property->initial_node);

    cout << "===== Property Automaton =====" << endl;
    property->dump(cout);

    LTLNode *LTL_init =
        new LTLNode("init", result_root_, property_init, id_counter++);
    current_automaton.initial_node = LTL_init;
    current_checking_node_t search_starting_node;
    search_starting_node.node = LTL_init;
    search_starting_node.created_nodes.push_back(LTL_init);
    current_checking_node_list_t search_list;
    search_list.push_back(search_starting_node);
    phase_list_t phase_list;
    LTLsearch(result_root_, search_list, phase_list);

    //dump result
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

  if (signal_handler::interrupted) {
    // // TODO: 各未実行フェーズを適切に処理
    // while(!todo_stack_->empty())
    // {
    //   simulation_job_sptr_t todo(todo_stack_->pop_todo());
    //   todo->parent->simulation_state = INTERRUPTED;
    //   // TODO: restart simulation from each interrupted phase
    // }
  }

  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}

// 再帰型DFSでシミュレーションを実行
/// current: 現在取り扱うフェーズ. これに続く後続フェーズを計算する
/// checking_list: 現在取り扱う, LTLNode とそこに至るパスおよびパス上の受理状態に至るパスのリストの構造体を要素に持つリスト
/// phase_list: これまでのフェーズ計算結果のリスト
void LTLModelChecker::LTLsearch(phase_result_sptr_t current,
                                current_checking_node_list_t checking_list,
                                phase_list_t phase_list) {
  io::SymbolicTrajPrinter printer(backend);
  cout << "########" << endl;
  cout << "current:" << endl;
  printer.output_one_phase(current);
  cout << "checking_list_size: " << checking_list.size() << endl;
  cout << "########" << endl;
  if (signal_handler::interrupted) {
    current->simulation_state = INTERRUPTED;
    return;
  }
  phase_simulator_->apply_diff(*current);
  HYDLA_LOGGER_DEBUG_VAR(*current);
  phase_list.push_back(current);
  if (current->todo_list.empty() & !checking_list.empty()) {
    // TODO
    // The simulation for this case is terminated
    // current_checking_node_list_t next_node_list = transition(checking_list, current);
    // 実装案1: t -> inf の時に自己ループとする？
    cout << "emptyyyy" << endl;
    cout << "checking list size: " << checking_list.size() << endl;
    result_automata.push_back(current_automaton.clone());
  }
  int count = 0;
  while (!current->todo_list.empty()) {
    phase_result_sptr_t todo = current->todo_list.front();
    current->todo_list.pop_front();
    profile_vector_->insert(todo);
    if (todo->simulation_state == NOT_SIMULATED) {
      phase_list_t result_list = process_one_todo(todo);
      cout << "result_list size: " << result_list.size() << endl;
      if (opts_->dump_in_progress) {
        printer.output_one_phase(todo);
      }
    }
    // count は枝を一本辿ったら +1 されるわけだが, count >= 1 だった時に refresh する必要があるのか？（枝分かれはすなわちパラメタの条件が異なること、というのを意図しているのは分かるが）
    // refresh では（transition でもやるが）受理サイクルの探索も行っている
    if (count >= 1) {
      cout << "other branch " << count << endl;
      ConstraintStore par_cons = todo->get_parameter_constraint();
      HYDLA_LOGGER_STANDARD(par_cons);
      checking_list = refresh(phase_list, par_cons, printer);
    }

    /* TODO: assertion違反が検出された場合の対応 */
    current_checking_node_list_t next_node_list =
        transition(checking_list, todo); //遷移関数でLTLモデル検査の挙動を特徴づける
    for(auto next_node_list_elem: next_node_list){
      cout << "created nodes: " << next_node_list_elem.created_nodes.size() << endl;
      for(auto ltlnode: next_node_list_elem.created_nodes){
        cout << "id: " << ltlnode->property->id << endl;
      }
    }
    LTLsearch(todo, next_node_list, phase_list);
    count++;
  }
  phase_simulator_->revert_diff(*current);
  for (auto checked_node : checking_list) {
    checked_node.node->remove();
  }
}

current_checking_node_list_t
LTLModelChecker::transition(current_checking_node_list_t checking_list,
                            phase_result_sptr_t phase) {
  current_checking_node_list_t next_search;
  for (auto current : checking_list) {
    for (auto property_node_edge : current.node->property->edges) {
      // phase と property_node_edge->second(edgeのguard条件) で成否判定する
      if (check_edge_guard_parameter(phase, property_node_edge.second,
                              phase->get_parameter_constraint())) {
        LTLNode *next_node;
        LTLNode *exist_node = (LTLNode *)current_automaton.exist_node(
            "Property" + ((property_node_edge.first)->name) + " Phase" +
            to_string(phase->id));
        if (exist_node != nullptr) {
          next_node = exist_node;
        } else {
          next_node = new LTLNode(
              phase, (PropertyNode *)(property_node_edge.first), id_counter++);
        }

        // acceptance cycleの探索
        LTLNode *loop_node =
            detect_acceptance_cycle(next_node, current.acceptance_path_list);
        if (loop_node != NULL) { // acceptance cycle を発見した場合
          if (!current_automaton.exist_edge(current.node, loop_node)) {
            current.node->add_edge(loop_node);
          }
          // current.node->set_color_to_trace_path("red");
          for (auto path : current.created_nodes) {
            path->set_color("red");
          }
          loop_node->set_color("red");
          result_automata.push_back(current_automaton.clone());
          for (auto path : current.created_nodes) {
            path->set_color("#000000");
          }
          next_node->set_color("#000000");
          next_search.clear();
          return next_search;
        }
        //通常ループの探索
        loop_node = detect_loop_in_path(next_node, current.created_nodes);
        if (loop_node != NULL) { //ループの場合
          if (!current_automaton.exist_edge(current.node, loop_node)) {
            current.node->add_edge(loop_node);
          }
        } else { //ループでない場合
          if (!current_automaton.exist_edge(current.node, next_node)) {
            current.node->add_edge(next_node);
          }
          if (next_node->acceptanceState()) { // acceptance stateの場合
            // next_node->set_color_to_trace_path("red");
            for (auto path : current.created_nodes) {
              path->set_color("red");
            }
            next_node->set_color("red");
            result_automata.push_back(current_automaton.clone());
            for (auto path : current.created_nodes) {
              path->set_color("#000000");
            }
            next_node->set_color("#000000");
            next_search.clear();
            return next_search;
          }
          //次の探索の準備
          //created_nodes: パスを記録する役割を持つので, current.created_nodes に next_node を付け加えたものが新しいパスとなる
          ltl_node_list_t tmp_created_nodes = current.created_nodes;
          tmp_created_nodes.push_back(next_node);
          current_checking_node_t next_search_candidate(next_node, tmp_created_nodes, current.acceptance_path_list);
          if (next_node->acceptanceCycle()) { // acceptance
                                              // cycleの受理状態の場合
            (next_search_candidate.acceptance_path_list)
                .push_back(next_search_candidate.created_nodes);
          }
          next_search.push_back(next_search_candidate);
        }
      }
    }
  }
  if (next_search.empty() & !checking_list.empty()) {
    result_automata.push_back(current_automaton.clone());
  }
  return next_search;
}

bool LTLModelChecker::check_including(LTLNode *larger, LTLNode *smaller) {
  //同期積オートマトンのノード間状態包含関係のチェック
  //1: 性質オートマトンの状態
  //2: フェーズタイプの比較
  //3: 変数表の大きさの比較
  //4: epsilon.m の checkInclude 
  //これらが全て true の時に包含が成立する
  bool include_ret;
  string larger_property = larger->property->name;
  string smaller_property = smaller->property->name;
  if (larger_property != smaller_property) {
    return false;
  }
  if (larger->phase->phase_type != smaller->phase->phase_type) {
    return false;
  }
  if (larger->phase->variable_map.size() != smaller->phase->variable_map.size()) {
    return false;
  }

  ConstraintStore larger_cons = larger->phase->get_parameter_constraint();
  ConstraintStore smaller_cons = smaller->phase->get_parameter_constraint();
  // comparing set of variables
  backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
                &(larger->phase->current_time),  &(larger->phase->variable_map),  &larger_cons, 
                &(smaller->phase->current_time), &(smaller->phase->variable_map), &smaller_cons, 
                &include_ret);
  
  // if (include_ret) cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n" << endl;
  // else             cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
  
  return include_ret;
}

LTLNode *LTLModelChecker::detect_acceptance_cycle(LTLNode *new_node,
                                                  ltl_path_list_t acceptance_path_list) {
  LTLNode *ret = NULL;
  for (ltl_path_list_t::iterator acceptance_path = acceptance_path_list.begin();
       acceptance_path != acceptance_path_list.end(); acceptance_path++) {
    ret = detect_loop_in_path(new_node, *acceptance_path);
    if (ret != NULL) {
      return ret;
    }
  }
  return ret;
}

LTLNode *LTLModelChecker::detect_loop_in_path(LTLNode *new_node,
                                              ltl_node_list_t path) {
  LTLNode *ret = NULL;
  for (ltl_node_list_t::iterator it = path.begin(); it != path.end(); it++) {
    if (check_including((LTLNode *)*it, new_node)) {
      ret = (LTLNode *)*it;
      return ret;
    }
  }
  return ret;
}

// phase_list_t LTLModelChecker::get_path(PhaseResult &phase){
//   phase_list_t ret;
//   if(*phase != result_root_){
//     ret = get_path(phase.parent);
//     ret.push_back(phase);
//   }
//   return ret;
// }


/// phase_list から
current_checking_node_list_t LTLModelChecker::refresh(phase_list_t phase_list, ConstraintStore par_cons, io::SymbolicTrajPrinter printer) {
  cout << "refresh start" << endl;
  // init
  LTLNode *LTL_init =
      new LTLNode("init", result_root_, property_init, id_counter++);
  Automaton new_automaton;
  new_automaton.initial_node = LTL_init;
  current_automaton.initial_node = LTL_init;
  current_checking_node_t search_starting_node;
  search_starting_node.node = LTL_init;
  search_starting_node.created_nodes.push_back(LTL_init);
  current_checking_node_list_t checking_list;
  current_checking_node_list_t next_search;
  checking_list.push_back(search_starting_node);
  // make automaton
  int c = 0;
  for (auto current_phase : phase_list) {
    if (c == 0) {
      c++;
      continue;
    }
    for (auto current_LTL : checking_list) {
      for (auto property_node_edge : current_LTL.node->property->edges) {
        // phase と property_edge->second(edgeのguard条件) で成否判定する
        if (check_edge_guard_parameter(current_phase, property_node_edge.second,
                                par_cons)) {
          LTLNode *next_node;
          LTLNode *exist_node = (LTLNode *)new_automaton.exist_node(
              "Property" + ((property_node_edge.first)->name) + " Phase" +
              to_string(current_phase->id));
          if (exist_node != nullptr) {
            next_node = exist_node;
          } else {
            next_node = new LTLNode(current_phase,
                                    (PropertyNode *)(property_node_edge.first),
                                    id_counter++);
          }
          // acceptance cycleの探索
          LTLNode *loop_node = detect_acceptance_cycle_parameter(
              next_node, current_LTL.acceptance_path_list, par_cons);
          if (loop_node != NULL) { // acceptance cycle を発見した場合
            if (!current_automaton.exist_edge(current_LTL.node, loop_node)) {
              current_LTL.node->add_edge(loop_node);
            }
            // current.node->set_color_to_trace_path("red");
            for (auto path : current_LTL.created_nodes) {
              path->set_color("red");
            }
            loop_node->set_color("red");
            result_automata.push_back(current_automaton.clone());
            for (auto path : current_LTL.created_nodes) {
              path->set_color("#000000");
            }
            next_node->set_color("#000000");
            next_search.clear();
            return next_search;
          }
          //通常ループの探索
          loop_node = detect_loop_in_path_parameter(
              next_node, current_LTL.created_nodes, par_cons);
          if (loop_node != NULL) { //ループの場合
            if (!current_automaton.exist_edge(current_LTL.node, loop_node)) {
              current_LTL.node->add_edge(loop_node);
            }
          } else { //ループでない場合
            if (!current_automaton.exist_edge(current_LTL.node, next_node)) {
              current_LTL.node->add_edge(next_node);
            }
            if (next_node->acceptanceState()) { // acceptance stateの場合
              for (auto path : current_LTL.created_nodes) {
                path->set_color("red");
              }
              next_node->set_color("red");
              result_automata.push_back(current_automaton.clone());
              for (auto path : current_LTL.created_nodes) {
                path->set_color("#000000");
              }
              next_node->set_color("#000000");
              next_search.clear();
              return next_search;
            }
            //次の探索の準備
            current_checking_node_t next_search_candidate;
            next_search_candidate.node = next_node;
            ltl_node_list_t tmp_created_nodes = current_LTL.created_nodes;
            tmp_created_nodes.push_back(next_node);
            next_search_candidate.created_nodes = tmp_created_nodes;
            next_search_candidate.acceptance_path_list =
                current_LTL.acceptance_path_list;
            if (next_node
                    ->acceptanceCycle()) { // acceptance cycleの受理状態の場合
              (next_search_candidate.acceptance_path_list)
                  .push_back(next_search_candidate.created_nodes);
            }
            next_search.push_back(next_search_candidate);
          }
        }
      }
    }
    if (next_search.empty() & !checking_list.empty()) {
      result_automata.push_back(current_automaton.clone());
    }
  }
  return next_search;
}

bool LTLModelChecker::check_including_parameter(LTLNode *larger, LTLNode *smaller,
                                         ConstraintStore par_cons) {
  bool include_ret;
  // property_automaton
  string larger_property = larger->property->name;
  string smaller_property = smaller->property->name;
  if (larger_property != smaller_property) {
    return false;
  }
  if (larger->phase->phase_type != smaller->phase->phase_type) {
    return false;
  }
  if (larger->phase->variable_map.size() != smaller->phase->variable_map.size()) {
    return false;
  }

  ConstraintStore larger_cons = larger->phase->get_parameter_constraint();
  ConstraintStore smaller_cons = smaller->phase->get_parameter_constraint();
  // comparing set of variables
  backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
                &(larger->phase->current_time), &(larger->phase->variable_map), &par_cons, 
                &(smaller->phase->current_time),&(smaller->phase->variable_map), &par_cons, 
                &include_ret);

  // if (include_ret) cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n" << endl;
  // else             cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
  return include_ret;
}

/// 新しい状態 new_node がどこかの acceptance_path の中に含まれているかを判定
LTLNode *LTLModelChecker::detect_acceptance_cycle_parameter(LTLNode *new_node, 
                                                            ltl_path_list_t acceptance_path_list,
                                                            ConstraintStore par_cons) {
  LTLNode *ret = NULL;
  for (ltl_path_list_t::iterator acceptance_path = acceptance_path_list.begin();
       acceptance_path != acceptance_path_list.end(); acceptance_path++) {
    ret = detect_loop_in_path_parameter(new_node, *acceptance_path, par_cons);
    if (ret != NULL) {
      return ret;
    }
  }
  return ret;
}

/// 新しい状態 new_node が包含されるようなノードが path に存在するかを判定
LTLNode *LTLModelChecker::detect_loop_in_path_parameter(LTLNode *new_node,
                                                        ltl_node_list_t path,
                                                        ConstraintStore par_cons) {
  LTLNode *ret = NULL;
  for (ltl_node_list_t::iterator it = path.begin(); it != path.end(); it++) {
    if (check_including_parameter((LTLNode *)*it, new_node, par_cons)) {
      ret = (LTLNode *)*it;
      return ret;
    }
  }
  return ret;
}

bool LTLModelChecker::check_edge_guard_parameter(phase_result_sptr_t phase,
                                          node_sptr guard,
                                          ConstraintStore par_cons) {
  // cout << "check_edge_gurad wp " << endl;
  if (guard->get_node_type_name() == "True") return true;

  bool ret;
  HYDLA_LOGGER_DEBUG("checking guard condition : ", get_infix_string(guard));
  VariableFinder var_finder;
  var_finder.visit_node(guard);
  variable_set_t related_variables = var_finder.get_all_variable_set();
  variable_map_t related_vm;
  variable_map_t vm = phase->variable_map;
  for (auto related_variable : related_variables) {
    auto vm_it = vm.find(related_variable);
    if (vm_it != vm.end())
      related_vm[related_variable] = vm_it->second;
  }
  HYDLA_LOGGER_DEBUG("variable map : ", related_vm);
  HYDLA_LOGGER_DEBUG("parameter condition : ", par_cons);

  if (phase->phase_type == POINT_PHASE) {
    backend->call("checkEdgeGuard", true, 3, "etmvtcsn", "b", &guard,
                  &related_vm, &par_cons, &ret);
  } else {
    backend->call("checkEdgeGuardWt", true, 5, "etmvtcsnvlnvln", "b", &guard,
                  &related_vm, &par_cons, &(phase->current_time),
                  &(phase->end_time), &ret);
  }

  return ret;
}

} // namespace simulator
} // namespace hydla
