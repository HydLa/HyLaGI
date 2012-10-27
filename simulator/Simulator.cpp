#include "Simulator.h"
#include "PhaseSimulator.h"

using namespace hydla::simulator;

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
  phase_id_ = 1;
  opts_->assertion = parse_tree->get_assertion_node();
  result_root_.reset(new phase_result_t());
  
  //出力変数無指定な場合の出力制御（全部出力）
  /*
  if(opts_->output_variables.empty()){
    BOOST_FOREACH(const variable_set_t::value_type& i, variable_set_) {
      opts_->output_variables.insert(i.get_string());
    }
  }
  */
  parse_tree_ = parse_tree;
  init_variable_map(parse_tree);
  continuity_map_t  cont(parse_tree->get_variable_map());
  phase_simulator_->initialize(variable_set_, parameter_set_, variable_map_, cont);
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
      variable_map_.set_variable(&(variable_set_.front()), value_t());
    }
  }

  HYDLA_LOGGER_REST(
    "#*** variable map ***\n",
    variable_map_);
}


Simulator::simulation_phase_t Simulator::create_new_simulation_phase() const
{
  simulation_phase_t ph;
  ph.phase_result.reset(new phase_result_t);
  ph.phase_result->cause_of_termination = NONE;
  return ph;
}

/**
 * 与えられたPhaseResultの情報をを引き継いだ，
 * 新たなPhaseResultの作成
 */
Simulator::simulation_phase_t Simulator::create_new_simulation_phase(const simulation_phase_t& old) const
{
  simulation_phase_t sim;
  sim.phase_result.reset(new phase_result_t(*old.phase_result));
  sim.phase_result->cause_of_termination = NONE;
  return sim;
}
