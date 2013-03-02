#include "Simulator.h"
#include "PhaseSimulator.h"
#include "SymbolicValue.h"
#include "ModuleSetContainerInitializer.h"


#include <iostream>
#include <string>
#include <stack>
#include <cassert>

namespace hydla{
namespace simulator{


Simulator::Simulator(Opts& opts):opts_(&opts){}

/**
 * 使用するPhaseSimulatorを設定する．
 * この関数に渡すPhaseSimulatorのインスタンスはnewで作成し，呼び出し側でdeleteしないようにする
 */
void Simulator::set_phase_simulator(phase_simulator_t *ps){
  phase_simulator_.reset(ps);
}

void Simulator::initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  todo_id_ = 0;
  opts_->assertion = parse_tree->get_assertion_node();
  result_root_.reset(new phase_result_t());
  result_root_->step = -1;
  result_root_->id = 0;
  
  parse_tree_ = parse_tree;
  init_variable_map(parse_tree);
  continuity_map_t  cont(parse_tree->get_variable_map());
  phase_simulator_->initialize(variable_set_, parameter_set_,
   variable_map_, cont, msc_no_init_);
  push_initial_state();
}


void Simulator::init_module_set_container(const parse_tree_sptr& parse_tree)
{    
  if(opts_->nd_mode||opts_->interactive_mode) {
    //全解探索モードなど
    ModuleSetContainerInitializer::init<ch::ModuleSetGraph>(
        parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
  else {
    //通常実行モード
    ModuleSetContainerInitializer::init<ch::ModuleSetList>(
        parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
}

void Simulator::init_variable_map(const parse_tree_sptr& parse_tree)
{
  typedef hydla::parse_tree::ParseTree::variable_map_const_iterator vmci;

  vmci it  = parse_tree->variable_map_begin();
  vmci end = parse_tree->variable_map_end();
  for(; it != end; ++it)
  {
    for(int d=0; d<=it->second; ++d) {
      variable_t v;
      v.name             = it->first;
      v.derivative_count = d;
      variable_set_.push_front(v);
      variable_map_[&(variable_set_.front())] = value_t(new symbolic_simulator::SymbolicValue());
    }
  }
}


void Simulator::push_initial_state()
{
  //初期状態を作ってスタックに入れる
  simulation_todo_sptr_t todo(new SimulationTodo());
  todo->elapsed_time = 0;
  todo->phase        = simulator::PointPhase;
  todo->current_time = value_t(new hydla::symbolic_simulator::SymbolicValue("0"));
  todo->module_set_container = msc_original_;
  todo->ms_to_visit = msc_original_->get_full_ms_list();
  todo->parent = result_root_;
  push_simulation_todo(todo);
}

void Simulator::push_simulation_todo(const simulation_todo_sptr_t& todo)
{
  todo->id = todo_id_++;
  todo_stack_.push_front(todo);
}

simulation_todo_sptr_t Simulator::pop_simulation_phase()
{
  simulation_todo_sptr_t state;
  if(opts_->search_method == simulator::DFS){
    state = todo_stack_.front();
    todo_stack_.pop_front();
  }else{
    state = todo_stack_.back();
    todo_stack_.pop_back();
  }
  profile_vector_.push_back(state);
  return state;
}

module_set_sptr Simulator::get_max_ms_no_init() const
{
  return msc_no_init_->get_max_module_set();
}

}
}