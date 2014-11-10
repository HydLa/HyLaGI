#include "Simulator.h"
#include "PhaseSimulator.h"
#include "Backend.h"
#include "ValueRange.h"
#include "ModuleSetContainerInitializer.h"
#include "PhaseResult.h"
#include "AffineApproximator.h"
#include "SymbolicTrajPrinter.h"
#include "Timer.h"
#include "VariableFinder.h"
#include "TimeOutError.h"

#include <iostream>
#include <string>
#include <cassert>

using namespace std;
using namespace hydla::backend;

namespace hydla{
namespace simulator{

Simulator::Simulator(Opts& opts):system_time_("time", 0), opts_(&opts)
{
  affine_transformer_ = interval::AffineApproximator::get_instance();
  affine_transformer_->set_simulator(this);
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
                               original_map_, module_set_container_, result_root_);

  if(opts_->assertion)
  {
    BreakPoint bp;
    bp.condition.reset(new symbolic_expression::Not(opts_->assertion));
    bp.call_back = assert_call_back;
    bp.tag = this;
    phase_simulator_->add_break_point(bp);
  }

  profile_vector_.reset(new entire_profile_t());
}

void Simulator::reset_result_root()
{
  result_root_.reset(new phase_result_t());
  result_root_->step = -1;
  result_root_->id = 0;
  result_root_->end_time = value_t("0");
}


void Simulator::init_module_set_container(const parse_tree_sptr& parse_tree)
{    
  if(opts_->static_generation_of_module_sets){
    if(opts_->nd_mode){
      ModuleSetContainerInitializer::init<hierarchy::ModuleSetGraph>(
          parse_tree, module_set_container_);
    }else{
      ModuleSetContainerInitializer::init<hierarchy::ModuleSetList>(
          parse_tree, module_set_container_);
    }
  }else{ 
    ModuleSetContainerInitializer::init<hierarchy::IncrementalModuleSet>(
        parse_tree, module_set_container_);
  }
}

void Simulator::init_variable_map(const parse_tree_sptr& parse_tree)
{
  typedef hydla::parse_tree::ParseTree::variable_map_const_iterator vmci;

  for(auto entry : parse_tree->get_variable_map())
  {
    for(int d = 0; d <= entry.second; ++d)
    {
      variable_t v;
      v.name               = entry.first;
      v.differential_count = d;
      variable_set_.insert(v);
      original_map_[v] = ValueRange();
    }
  }
}


parameter_t Simulator::introduce_parameter(const variable_t &var,const PhaseResult &phase, const ValueRange &range)
{
  parameter_t param(var, phase);
  return introduce_parameter(param, range);
}


parameter_t Simulator::introduce_parameter(const string &name, int differential_cnt, int id, const ValueRange &range)
{
  parameter_t param(name, differential_cnt, id);
  return introduce_parameter(param, range);
}

parameter_t Simulator::introduce_parameter(const parameter_t &param, const ValueRange &range)
{
  parameter_map_[param] = range;
  backend->call("addParameter", 1, "p", "", &param);
  return param;
}


phase_result_sptr_t Simulator::make_initial_todo()
{
  phase_result_sptr_t todo(new PhaseResult());
  todo->parent = result_root_.get();
  result_root_->todo_list.push_back(todo);
  todo->current_time = value_t("0");
  todo->id = 1;
  todo->phase_type        = POINT_PHASE;
  todo->step = 0;
  return todo;
}

void Simulator::process_one_todo(phase_result_sptr_t& todo)
{
  if( opts_->max_phase >= 0 && todo->step >= opts_->max_phase){
    todo->parent->simulation_state = simulator::STEP_LIMIT;
    return;
  }
  HYDLA_LOGGER_DEBUG("\n--- Current Todo ---\n", *todo);
  HYDLA_LOGGER_DEBUG("\n--- prev map ---\n", todo->prev_map);

  try{
    timer::Timer phase_timer;
    phase_simulator_->process_todo(todo);
    todo->profile["EntirePhase"] += phase_timer.get_elapsed_us();
  }
  catch(const timeout::TimeOutError &te)
  {
    HYDLA_LOGGER_DEBUG(te.what());
    phase_result_sptr_t phase(new PhaseResult(*todo));
    phase->simulation_state = TIME_OUT_REACHED;
    todo->parent->children.push_back(phase);
  }
  HYDLA_LOGGER_DEBUG("\n--- Result Phase ---\n", *todo);
}

bool Simulator::assert_call_back(BreakPoint bp, phase_result_sptr_t phase)
{
  phase->simulation_state = ASSERTION;
  HYDLA_LOGGER_DEBUG_VAR(*phase);
  cout << "Assertion failed!" << endl;
  cout << io::SymbolicTrajPrinter().get_state_output(*phase);
  return false;
}

}
}
