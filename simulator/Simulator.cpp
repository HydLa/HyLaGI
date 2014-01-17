#include "Simulator.h"
#include "PhaseSimulator.h"
#include "Backend.h"
#include "MathematicaLink.h"
#include "REDUCELinkFactory.h"
#include "ValueRange.h"
#include "ModuleSetContainerInitializer.h"
#include "PhaseResult.h"
#include "AffineTranslator.h"

#include <iostream>
#include <string>
#include <stack>
#include <cassert>

using namespace std;
using namespace hydla::backend;
using namespace hydla::backend::mathematica;
using namespace hydla::backend::reduce;

namespace hydla{
namespace simulator{

Simulator::Simulator(Opts& opts):system_time_("time", 0), opts_(&opts)
{
  affine_translator_ = interval::AffineTranslator::get_instance();
}

Simulator::~Simulator()
{
}

void Simulator::set_phase_simulator(phase_simulator_t *ps){
  phase_simulator_.reset(ps);
  phase_simulator_->set_backend(backend);
}

void Simulator::set_backend(backend_sptr_t back)
{
  backend = back;
}

void Simulator::initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  opts_->assertion = parse_tree->get_assertion_node();
  reset_result_root();

  parse_tree_ = parse_tree;
  init_variable_map(parse_tree);
  hydla::parse_tree::ParseTree::variable_map_t vm = parse_tree_->get_variable_map();
  phase_simulator_->initialize(variable_set_, parameter_map_,
                               original_map_, vm, parse_tree_, msc_no_init_);
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

/*  ModuleSetContainerInitializer::init<ch::IncrementalModuleSet>(
      parse_tree, msc_original_, msc_no_init_, parse_tree_);
*/
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

  vmci it  = parse_tree->variable_map_begin();
  vmci end = parse_tree->variable_map_end();
  for(; it != end; ++it)
  {
    for(int d=0; d<=it->second; ++d)
    {
      variable_t v;
      v.name             = it->first;
      v.derivative_count = d;
      variable_set_.insert(v);
      original_map_[v] = ValueRange();
    }
  }
}


parameter_t Simulator::introduce_parameter(variable_t var, phase_result_sptr_t& phase, ValueRange& range)
{
  parameter_t param(var, phase);
  parameter_map_[param] = range;

  backend->call("addParameter", 1, "p", "", &param);
  return param;
}

simulation_todo_sptr_t Simulator::make_initial_todo()
{
  simulation_todo_sptr_t todo(new SimulationTodo());
  todo->elapsed_time = 0;
  todo->phase        = simulator::PointPhase;
  todo->current_time = value_t("0");
  todo->module_set_container = msc_original_;
  todo->ms_to_visit = msc_original_->get_full_ms_list();
  todo->maximal_mss.clear();
  todo->parent = result_root_;
  return todo;
}



std::ostream& operator<<(std::ostream& s, const SimulationTodo& todo)
{
  s << "%% PhaseType: " << todo.phase << std::endl;
  s << "%% id: " <<  todo.id          << std::endl;
  s << "%% time: " << todo.current_time << std::endl;
  s << "--- parent phase result ---" << std::endl;
  s << *(todo.parent) << std::endl;
  s << "--- temporary_constraints ---"  << std::endl; 
  s << todo.temporary_constraints      << std::endl;
  s << "--- parameter map ---"          << std::endl;
  s << todo.parameter_map << std::endl;
  
  return s;
}



}
}
