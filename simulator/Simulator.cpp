#include "Simulator.h"
#include "PhaseSimulator.h"
#include "Backend.h"
#include "SymbolicValue.h"
#include "ValueRange.h"
#include "ModuleSetContainerInitializer.h"

#include <iostream>
#include <string>
#include <stack>
#include <cassert>

namespace hydla{
namespace simulator{


Simulator::Simulator(Opts& opts):system_time_("time", 0), opts_(&opts){}

void Simulator::set_phase_simulator(phase_simulator_t *ps){
  phase_simulator_.reset(ps);
}

void Simulator::initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  opts_->assertion = parse_tree->get_assertion_node();
  reset_result_root();

  parse_tree_ = parse_tree;
  init_variable_map(parse_tree);
  hydla::parse_tree::ParseTree::variable_map_t vm = parse_tree_->get_variable_map();
  phase_simulator_->initialize(*variable_set_, *parameter_set_,
   *original_map_, vm, msc_no_init_);
  profile_vector_.reset(new entire_profile_t());

  //  if(opts_->analysis_mode == "simulate"||opts_->analysis_mode == "cmmap") phase_simulator_->init_arc(parse_tree);
}

void Simulator::reset_result_root()
{
  result_root_.reset(new phase_result_t());
  result_root_->step = -1;
  result_root_->id = 0;
}


void Simulator::init_module_set_container(const parse_tree_sptr& parse_tree)
{    
  if(opts_->nd_mode||opts_->interactive_mode)
  {
    //全解探索モードなど
    ModuleSetContainerInitializer::init<ch::ModuleSetGraph>(
        parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
  else
  {
    //通常実行モード
    ModuleSetContainerInitializer::init<ch::ModuleSetList>(
        parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
}

void Simulator::init_variable_map(const parse_tree_sptr& parse_tree)
{
  typedef hydla::parse_tree::ParseTree::variable_map_const_iterator vmci;
  variable_set_.reset(new variable_set_t());
  original_map_.reset(new variable_map_t());
  original_parameter_map_.reset(new parameter_map_t());
  parameter_set_.reset(new parameter_set_t());

  vmci it  = parse_tree->variable_map_begin();
  vmci end = parse_tree->variable_map_end();
  for(; it != end; ++it)
  {
    for(int d=0; d<=it->second; ++d)
    {
      variable_t v;
      v.name             = it->first;
      v.derivative_count = d;
      variable_set_->push_front(v);
      (*original_map_)[v] = ValueRange();
    }
  }
}


parameter_t Simulator::introduce_parameter(variable_t var, phase_result_sptr_t& phase, ValueRange& range)
{
  parameter_t param(var, phase);
  parameter_set_->push_front(param);
  (*original_parameter_map_)[parameter_set_->front()] = range;
  // TODO: バックエンド側での対応
  backend_->call("addParameter", 1, "p", "", &param);
  return parameter_set_->front();
}

simulation_todo_sptr_t Simulator::make_initial_todo()
{
  simulation_todo_sptr_t todo(new SimulationTodo());
  todo->elapsed_time = 0;
  todo->phase        = simulator::PointPhase;
  todo->current_time = value_t(new hydla::simulator::symbolic::SymbolicValue("0"));
  todo->module_set_container = msc_original_;
  todo->ms_to_visit = msc_original_->get_full_ms_list();
  todo->parent = result_root_;
  return todo;
}



}
}
