#include "PhaseSimulator.h"
#include "AskCollector.h"
#include "NonPrevSearcher.h"
#include "VariableFinder.h"
#include "SymbolicValue.h"
#include "Dumpers.h"
#include "Exceptions.h"
#include "SymbolicVirtualConstraintSolver.h"

using namespace hydla::simulator;

PhaseSimulator::PhaseSimulator(const Opts& opts):opts_(&opts), select_phase_(NULL){}

PhaseSimulator::~PhaseSimulator(){}

void PhaseSimulator::init_false_conditions(module_set_container_sptr& msc_no_init){
  msc_no_init->reset();
  while(msc_no_init->go_next()){
    false_conditions_.insert(false_map_t::value_type(msc_no_init->get_module_set(),node_sptr()));
    msc_no_init->mark_current_node();
  }
}

void PhaseSimulator::check_all_module_set(module_set_container_sptr& msc_no_init){
  msc_no_init->reverse_reset();
  while(msc_no_init->reverse_go_next()){
    FalseConditionsResult result = find_false_conditions(msc_no_init->get_reverse_module_set());
    if(result != FALSE_CONDITIONS_FALSE){
        msc_no_init->mark_super_module_set();
    }
    msc_no_init->mark_r_current_node();
  }
}


PhaseSimulator::result_list_t PhaseSimulator::calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont)
{
  HYDLA_LOGGER_PHASE("%% current time:", *todo->current_time);
  timer::Timer phase_timer;
  result_list_t result;
  
  
  todo->module_set_container->reset(todo->ms_to_visit);

  if(todo_cont == NULL)
  {
    todo_container_t tmp_cont;
    todo_container_ = &tmp_cont;
    simulation_todo_sptr_t tmp_todo = todo;
    while(true)
    {
      result_list_t tmp_result = make_results_from_todo(tmp_todo);
      result.insert(result.begin(), tmp_result.begin(), tmp_result.end());
      if(tmp_cont.empty())break;
      tmp_todo = tmp_cont.pop_todo();
      tmp_todo->module_set_container->reset(tmp_todo->ms_to_visit);
    }
  }
  else
  {
    todo_container_ = todo_cont;
    result = make_results_from_todo(todo);
  }

  todo->profile["Phase"] += phase_timer.get_elapsed_us();
  return result;
}

PhaseSimulator::result_list_t PhaseSimulator::make_results_from_todo(simulation_todo_sptr_t& todo)
{
  result_list_t result; 
  bool has_next = false;
  variable_map_t time_applied_map;
  boost::shared_ptr<RelationGraph> graph;
  
  if(todo->phase == PointPhase)
  {
    time_applied_map = apply_time_to_vm(todo->parent->variable_map, todo->current_time);
    graph = pp_relation_graph_;
    set_simulation_mode(PointPhase);
  }else{
    time_applied_map = todo->parent->variable_map;
    graph = ip_relation_graph_;
    set_simulation_mode(IntervalPhase);
  }
  
  while(todo->module_set_container->go_next())
  {
    module_set_sptr ms = todo->module_set_container->get_module_set();
    
    std::string module_sim_string = "\"ModuleSet" + ms->get_name() + "\"";
    timer::Timer ms_timer;
    result_list_t tmp_result = simulate_ms(ms, graph, time_applied_map, todo);
    if(!tmp_result.empty())
    {
      has_next = true;
      result.insert(result.begin(), tmp_result.begin(), tmp_result.end());
    }
    todo->profile[module_sim_string] += ms_timer.get_elapsed_us();
    todo->positive_asks.clear();
    todo->negative_asks.clear();
  }

  //無矛盾な解候補モジュール集合が存在しない場合
  if(!has_next)
  {
    todo->parent->cause_of_termination = simulator::INCONSISTENCY;
  }
  return result;
}


PhaseSimulator::result_list_t PhaseSimulator::simulate_ms(const hydla::ch::module_set_sptr& ms,
  boost::shared_ptr<RelationGraph>& graph, const variable_map_t& time_applied_map, simulation_todo_sptr_t& todo)
{
  HYDLA_LOGGER_MS("--- next module set ---\n", ms->get_infix_string());
  graph->set_valid(ms.get());
  result_list_t result;
  
  // TODO:CalculateVariableMapの結果，変数表が複数作られるような分岐を無視している

  variable_range_map_t vm;
  if(todo->module_set_container != msc_no_init_)
  {
    CalculateVariableMapResult cvm_res =
      calculate_variable_map(ms, todo, time_applied_map, vm);
    switch(cvm_res)
    {
      case CVM_CONSISTENT:
      HYDLA_LOGGER_MS("--- CVM_CONSISTENT ---\n", vm);
      break;
      
      case CVM_INCONSISTENT:
      HYDLA_LOGGER_MS("%% CVM_INCONSISTENT");
      todo->module_set_container->mark_nodes(*ms);
      return result;
      break;
      
      case CVM_ERROR:
      HYDLA_LOGGER_MS("%% CVM_ERROR");
      throw SimulateError("CalculateVariableMap for " + ms->get_name());
      break;
    }
  }
  else
  {
    HYDLA_LOGGER_MS("%% connected module set size:", graph->get_connected_count());
    for(int i = 0; i < graph->get_connected_count(); i++){
      module_set_sptr connected_ms = graph->get_component(i);
      HYDLA_LOGGER_MS("--- next connected module set ---\n", connected_ms->get_infix_string());
      SimulationTodo::ms_cache_t::iterator ms_it = todo->ms_cache.find(*connected_ms);
      if(ms_it != todo->ms_cache.end())
      {
        merge_variable_map(vm, ms_it->second);
      }
      else
      {
        variable_range_map_t tmp_vm;
        CalculateVariableMapResult cvm_res =
          calculate_variable_map(connected_ms, todo, time_applied_map, tmp_vm);
        switch(cvm_res)
        {
          case CVM_CONSISTENT:
          HYDLA_LOGGER_MS("--- CVM_CONSISTENT ---\n", tmp_vm);
          todo->ms_cache.insert(std::make_pair(*connected_ms, tmp_vm) );
          HYDLA_LOGGER_MS("%% merge");
          merge_variable_map(vm, tmp_vm);
          break;
          
          case CVM_INCONSISTENT:
          HYDLA_LOGGER_MS("%% CVM_INCONSISTENT");
          todo->module_set_container->mark_nodes(*connected_ms);
          return result;
          break;
          
          case CVM_ERROR:
          HYDLA_LOGGER_MS("%% CVM_ERROR");
          throw SimulateError("CalculateVariableMap for " + connected_ms->get_name());
          break;
        }
      }
    }
  }
  todo->module_set_container->mark_nodes();
  
  
  phase_result_sptr_t phase = make_new_phase(todo, vm);
  phase->module_set = ms;
  
  if(opts_->assertion){
    timer::Timer entailment_timer;
    solver_->reset_constraint(phase->variable_map, true);
    solver_->reset_parameters(phase->parameter_map);
    HYDLA_LOGGER_MS("%% check_assertion");
    hydla::vcs::CheckConsistencyResult cc_result;
    switch(check_entailment(cc_result, node_sptr(new parse_tree::Not(opts_->assertion)), continuity_map_t())){
      case ENTAILED:
      case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
        std::cout << "Assertion Failed!" << std::endl;
        HYDLA_LOGGER_CLOSURE("%% Assertion Failed!");
        phase->cause_of_termination = ASSERTION;
        break;
      case CONFLICTING:
        break;
        /*
      case BRANCH_VAR:
        HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of variables");
          // 分岐先を生成
        {
          simulation_todo_sptr_t new_state(create_new_simulation_phase(todo));
          new_state->temporary_constraints.push_back(opts_->assertion);
          todo_container_->push_back(new_state);
        }
        {
          node_sptr not_cons(new hydla::parse_tree::Not(opts_->assertion));
          todo->temporary_constraints.push_back(not_cons);
          solver_->add_constraint(not_cons);
        }
        break;
        */
      case BRANCH_PAR:
        HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of parameters");
        push_branch_states(todo, cc_result);
        std::cout << "Assertion Failed!" << std::endl;
        phase->parameter_map = todo->parameter_map;
        HYDLA_LOGGER_CLOSURE("%% Assertion Failed!");
        phase->cause_of_termination = ASSERTION;
        break;
    }
    todo->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
  }
  
  result.push_back(phase);
  return result;
}


void PhaseSimulator::push_branch_states(simulation_todo_sptr_t &original, hydla::vcs::CheckConsistencyResult &result){
  for(int i=1; i<(int)result.true_parameter_maps.size();i++){
    simulation_todo_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->parameter_map = result.true_parameter_maps[i];
    todo_container_->push_todo(branch_state);
  }
  for(int i=0; i<(int)result.false_parameter_maps.size();i++){
    simulation_todo_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->parameter_map = result.false_parameter_maps[i];
    todo_container_->push_todo(branch_state);
  }
  original->parameter_map = result.true_parameter_maps[0];
  solver_->reset_parameters(original->parameter_map);
}


phase_result_sptr_t PhaseSimulator::make_new_phase(simulation_todo_sptr_t& todo, const variable_range_map_t& vm)
{
  phase_result_sptr_t phase(new PhaseResult(*(todo->parent)));
  phase->id = ++phase_sum_;
  phase->variable_map = range_map_to_value_map(phase, vm, todo->parameter_map);
  phase->parameter_map = todo->parameter_map;
  phase->current_time = todo->current_time;
  phase->expanded_always = todo->expanded_always;
  phase->positive_asks = todo->positive_asks;
  phase->negative_asks = todo->negative_asks;
  phase->children.clear();
  phase->parent = todo->parent;
  phase->step = todo->parent->step + 1;
  phase->phase = todo->phase;
  todo->parent->children.push_back(phase);
  return phase;
}


phase_result_sptr_t PhaseSimulator::make_new_phase(const phase_result_sptr_t& original)
{
  phase_result_sptr_t phase(new PhaseResult(*original));
  phase->id = ++phase_sum_;
  phase->parent->children.push_back(phase);
  phase->cause_of_termination = simulator::NONE;
  return phase;
}

void PhaseSimulator::merge_variable_map(variable_range_map_t& lhs, variable_range_map_t& rhs)
{
  variable_range_map_t::iterator it = rhs.begin();
  for(;it != rhs.end(); it++)
  {
    if(!it->second.is_undefined())
    {
      lhs[it->first] = it->second;
    }
  }
}

void PhaseSimulator::initialize(variable_set_t &v,
                                parameter_set_t &p,
                                variable_map_t &m,
                                continuity_map_t& c, 
                                const module_set_container_sptr &msc_no_init)
{
  variable_set_ = &v;
  parameter_set_ = &p;
  variable_map_ = &m;
  phase_sum_ = 0;

  msc_no_init_ = msc_no_init;
  const hydla::simulator::module_set_sptr ms = msc_no_init->get_max_module_set();
  
  // TODO:RelationGraph上では，ASSERT文を無視しているため，制約モジュール中では独立でもASSERT文の中で関係している変数があった場合に正しく動作しない
  // RelationGraphを作る上では，ASSERTを特殊な「モジュールのようなもの」として扱う必要がある．
  pp_relation_graph_ = RelationGraph::new_graph(*ms, *variable_set_, false);
  ip_relation_graph_ = RelationGraph::new_graph(*ms, *variable_set_, true);
  
  if(opts_->dump_relation){
    pp_relation_graph_->dump_graph(std::cout);
    ip_relation_graph_->dump_graph(std::cout);
    exit(EXIT_SUCCESS);
  }
  
  AskCollector ac(ms);
  expanded_always_t eat;
  positive_asks_t pat;
  negative_asks_t nat;

  ac.collect_ask(&eat, &pat, &nat, &prev_guards_);
  NonPrevSearcher searcher;
  for(negative_asks_t::iterator it = prev_guards_.begin(); it != prev_guards_.end();){
    if(searcher.judge_non_prev((*it)->get_guard())){
      prev_guards_.erase(it++);
    }else{
      it++;
    }
  }
  
  if(opts_->optimization_level >= 2){
    init_false_conditions(msc_no_init_);
    if(opts_->optimization_level >= 3){
      check_all_module_set(msc_no_init_);
    }
  }
}


simulation_todo_sptr_t PhaseSimulator::create_new_simulation_phase(const simulation_todo_sptr_t& old) const
{
  simulation_todo_sptr_t sim(new SimulationTodo(*old));
  return sim;
}