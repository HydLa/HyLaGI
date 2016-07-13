#include "HybridAutomatonConverter.h"
#include "Timer.h"
#include "PhaseSimulator.h"
#include "ValueModifier.h"
#include "SignalHandler.h"
#include "TimeOutError.h"
#include "VariableReplacer.h"
#include "Logger.h"
#include "Utility.h"
#include <limits.h>
#include <string>
#include "Backend.h"
#include "ValueModifier.h"
#include <stdio.h>
#include "SymbolicTrajPrinter.h"

using namespace std;

namespace hydla {
namespace simulator {

using namespace std;
using namespace symbolic_expression;

HybridAutomatonConverter::HybridAutomatonConverter(Opts &opts):Simulator(opts){}

HybridAutomatonConverter::~HybridAutomatonConverter(){}

phase_result_sptr_t HybridAutomatonConverter::simulate()
{
  std::string error_str = "";
  make_initial_todo();
  try
    {
      AutomatonNode* init = new AutomatonNode(result_root_, "init");
      current_automaton.initial_node = init;
      HA_node_list_t created_nodes;
      HA_translate(result_root_, init, created_nodes);
      int automaton_count = 1;
      for(auto automaton: result_automata)
      {
        cout << "===== Automaton" << automaton_count++ << " =====" << endl;
        automaton.dump(cout);
      }
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

  HYDLA_LOGGER_DEBUG("%% simulation ended");
  return result_root_;
}

void HybridAutomatonConverter::HA_translate(phase_result_sptr_t current, AutomatonNode * current_automaton_node, HA_node_list_t created_nodes)
{
  io::SymbolicTrajPrinter printer(backend);
  if(signal_handler::interrupted)
    {
      current->simulation_state = INTERRUPTED;
      return;
    }
  phase_simulator_->apply_diff(*current);
  HYDLA_LOGGER_DEBUG_VAR(*current);
  while(!current->todo_list.empty())
    {
      phase_result_sptr_t todo = current->todo_list.front();
      current->todo_list.pop_front();
      profile_vector_->insert(todo);
      if(todo->simulation_state == NOT_SIMULATED){
        phase_list_t result_list = process_one_todo(todo);
        for(auto result : result_list)
        {
          if(result->todo_list.empty())
          {
            // The simulation for this case is terminated
            AutomatonNode* next_node = create_phase_node(result);
            current_automaton_node->add_edge(next_node);
            Automaton result_automaton = current_automaton.clone();
            next_node->remove(); // remove from original Automaton
            result_automata.push_back(result_automaton);
          }
        }
        if(opts_->dump_in_progress){
          printer.output_one_phase(todo);
        }
      }
      /* TODO: assertion違反が検出された場合の対応 */
      AutomatonNode* next_node = transition(current_automaton_node, todo, created_nodes);
      if(next_node == nullptr)
      {
        HYDLA_LOGGER_DEBUG("A loop is detected");
        HYDLA_LOGGER_DEBUG_VAR(*todo);
        result_automata.push_back(current_automaton.clone());
      }
      else
      {
        HA_translate(todo, next_node, created_nodes);
      }
    }
  phase_simulator_->revert_diff(*current);
  current_automaton_node->remove();
}


AutomatonNode* HybridAutomatonConverter::create_phase_node(phase_result_sptr_t phase)
{
  return new AutomatonNode(phase, "Phase " + to_string(phase->id), phase->id);
}

AutomatonNode* HybridAutomatonConverter::transition(AutomatonNode* current_HA_node,phase_result_sptr_t phase, HA_node_list_t &created_nodes){
  AutomatonNode* next_node = create_phase_node(phase);
  //通常ループの探索
  AutomatonNode* loop_node = detect_loop(next_node, created_nodes);
  //ループの場合
  if(loop_node!=NULL){
    current_HA_node->add_edge(loop_node);
    return nullptr;
  }
  //ループでない場合
  else{
    current_HA_node->add_edge(next_node);
    created_nodes.push_back(next_node);
    return next_node;
  }
}

AutomatonNode* HybridAutomatonConverter::detect_loop(AutomatonNode* new_node, HA_node_list_t path){
  for(auto node: path){
    if(check_including(node,new_node)){
      return node;
    }
  }
  return nullptr;
}

bool HybridAutomatonConverter::check_including(AutomatonNode* larger,AutomatonNode* smaller){
  bool include_ret;

  //comparison of guards
  if(larger->phase->get_all_positive_asks() != smaller->phase->get_all_positive_asks() ||
    larger->phase->get_all_negative_asks() != smaller->phase->get_all_negative_asks())
  {
    return false;
  }


  if(larger->phase->unadopted_ms.compare(smaller->phase->unadopted_ms) != 0)
  {
    return false;
  }

  
  //phase typeの比較
  if(larger->phase->phase_type != smaller->phase->phase_type){
    HYDLA_LOGGER_DEBUG("different phase type :\n\t \"", larger->id, "\" : \"", smaller->id, "\"");
    return false;
  }
  //phase の変数表の大きさの比較
  if(larger->phase->variable_map.size() != smaller->phase->variable_map.size()){
    HYDLA_LOGGER_DEBUG("different size of variable map :\n\t \"", larger->id, "\" : \"", smaller->id, "\"");
    return false;
  }

  HYDLA_LOGGER_DEBUG_VAR(*larger->phase);
  HYDLA_LOGGER_DEBUG_VAR(*smaller->phase);
  VariableReplacer replacer(&larger->phase->variable_map);
  replacer.replace_variable_map(&larger->phase->variable_map);
  replacer.replace_variable_map(&smaller->phase->variable_map);
  


  ConstraintStore larger_cons = larger->phase->get_parameter_constraint();
  ConstraintStore smaller_cons = smaller->phase->get_parameter_constraint();

  // betsuno kaizou
  variable_map_t larger_vm, smaller_vm;
  for (const auto& x : larger->phase->variable_map) {
    if (x.first.name == "s1" || x.first.name == "s2") {
      larger_vm.insert(x);
      cout << x.first.name << " (" << x.first.differential_count << ") " << endl;
    }
  }
  for (const auto& x : smaller->phase->variable_map) {
    if (x.first.name == "s1" || x.first.name == "s2") {
      smaller_vm.insert(x);
      cout << x.first.name << " (" << x.first.differential_count << ") " << endl;
    }
  }
  //
  //compareing set of variables
  /*
  backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
                &(larger->phase->current_time), &(larger->phase->variable_map), &larger_cons,
                &(smaller->phase->current_time), &(smaller->phase->variable_map), &smaller_cons, &include_ret);
  */
  backend->call("checkInclude", true, 6, "vlnmvtcsnvlnmvtcsn", "b",
                &(larger->phase->current_time), &(larger_vm), &larger_cons,
                &(smaller->phase->current_time), &(smaller_vm), &smaller_cons, &include_ret);
  if(include_ret){
    cout << "\n\"" << larger->id << "\" includes \"" << smaller->id << "\"\n" << endl;
  }
  else{
    // cout << "not included :\n\t \"" << larger->id << "\" : \"" << smaller->id << "\"" << endl;
  }
  return include_ret;
}

}//namespace hydla
}//namespace simulator
