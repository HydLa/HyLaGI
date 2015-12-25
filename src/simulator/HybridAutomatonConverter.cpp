#include "HybridAutomatonConverter.h"
#include "Timer.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "ValueModifier.h"
#include "SignalHandler.h"
#include "TimeOutError.h"
#include "Logger.h"
#include "Utility.h"
#include <limits.h>
#include <string>
#include "Backend.h"
#include "ValueModifier.h"
#include "Automaton.h"
#include "HybridAutomaton.h"
#include <stdio.h>

using namespace std;

namespace hydla {
namespace simulator {

using namespace std;
using namespace symbolic_expression;

HybridAutomatonConverter::HybridAutomatonConverter(Opts &opts):Simulator(opts), printer(backend){}

HybridAutomatonConverter::~HybridAutomatonConverter(){}

phase_result_sptr_t HybridAutomatonConverter::simulate()
{
  std::string error_str = "";
  make_initial_todo();
  try
    {
      consistency_checker.reset(new ConsistencyChecker(backend));
      HybridAutomaton* init = new HybridAutomaton("init",result_root_);
      HA_node_list_t start;
      start.push_back(init);
      HA_translate(result_root_,start);
      init->dump();
    }
  catch(const std::runtime_error &se)
    {
      error_str += "error ";
      error_str += ": ";
      error_str += se.what();
      error_str += "\n";
      HYDLA_LOGGER_DEBUG_VAR(error_str);
      std::cout << error_str;
    }


  if(signal_handler::interrupted){
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

void HybridAutomatonConverter::HA_translate(phase_result_sptr_t current, HA_node_list_t current_automaton_node)
{
  if(signal_handler::interrupted)
    {
      current->simulation_state = INTERRUPTED;
      return;
    }
  phase_simulator_->apply_diff(*current);
  while(!current->todo_list.empty())
    {
      phase_result_sptr_t todo = current->todo_list.front();
      current->todo_list.pop_front();
      profile_vector_->insert(todo);

      if(todo->simulation_state == NOT_SIMULATED){
        process_one_todo(todo);
        /* TODO: assertion違反が検出された場合の対応 */
        current_automaton_node = transition(current_automaton_node,todo,consistency_checker);
      }
      if(opts_->dump_in_progress){
        printer.output_one_phase(todo);
      }
      HA_translate(todo,current_automaton_node);
    }
  phase_simulator_->revert_diff(*current);
}

HA_node_list_t HybridAutomatonConverter::transition(HA_node_list_t current,phase_result_sptr_t phase,consistency_checker_t consistency_checker){
  HA_node_list_t next_search;
  for(HA_node_list_t::iterator current_HA_node = current.begin();current_HA_node != current.end();current_HA_node++){
    HybridAutomaton* nextNode = new HybridAutomaton(phase);
    //通常ループの探索
    HybridAutomaton* loop_node = detect_loop_in_path(nextNode,(*current_HA_node)->trace_path);
    //ループの場合
    if(loop_node!=NULL){
      (*current_HA_node)->add_next_edge(loop_node);
    }
    //ループでない場合
    if(loop_node == NULL){
      (*current_HA_node)->add_next_edge(nextNode);
      next_search.push_back(nextNode);
    }
  }
  return next_search;
}

HybridAutomaton* HybridAutomatonConverter::detect_loop_in_path(HybridAutomaton* new_node, automaton_node_list_t path){
  HybridAutomaton* ret = NULL;
  for(automaton_node_list_t::iterator it = path.begin();it != path.end();it++){
    if(check_including((HybridAutomaton*)*it,new_node)){
      ret = (HybridAutomaton*)*it;
      return ret;
    }
  }
  return ret;
}

bool HybridAutomatonConverter::check_including(HybridAutomaton* larger,HybridAutomaton* smaller){
  bool include_ret;
  //phase typeの比較
  if(larger->phase->phase_type != smaller->phase->phase_type){
    // cout << "different phase type :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
    return false;
  }
  //phase の変数表の大きさの比較
  if(larger->phase->variable_map.size() != smaller->phase->variable_map.size()){
    // cout << "different size of variable map :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
    return false;
  }

  ConstraintStore larger_cons = larger->phase->get_parameter_constraint();
  ConstraintStore smaller_cons = smaller->phase->get_parameter_constraint();
  //compareing set of variables
  backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
                &(larger->phase->current_time), &(larger->phase->variable_map), &larger_cons,
                &(smaller->phase->current_time), &(smaller->phase->variable_map), &smaller_cons, &include_ret);
  if(include_ret){
    // cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n" << endl;
  }
  else{
    // cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
  }
  return include_ret;
}

}//namespace hydla
}//namespace simulator
