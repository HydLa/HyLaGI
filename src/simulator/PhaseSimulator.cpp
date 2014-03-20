#include "PhaseSimulator.h"
#include "AskCollector.h"
#include "NonPrevSearcher.h"
#include "VariableFinder.h"
#include "Exceptions.h"
#include "Backend.h"
#include "PrevReplacer.h"

using namespace std;
using namespace hydla::simulator;
using namespace hydla::backend;

PhaseSimulator::PhaseSimulator(Simulator* simulator,const Opts& opts): breaking(false), simulator_(simulator), opts_(&opts), select_phase_(NULL), break_condition_(node_sptr()) {
}

PhaseSimulator::~PhaseSimulator(){}

void PhaseSimulator::set_backend(backend_sptr_t back)
{
  backend_ = back;
}

PhaseSimulator::result_list_t PhaseSimulator::calculate_phase_result(simulation_todo_sptr_t& todo, todo_container_t* todo_cont)
{
  HYDLA_LOGGER_DEBUG("%% current time:", todo->current_time);
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

  todo->profile["PhaseResult"] += phase_timer.get_elapsed_us();
  return result;
}

PhaseSimulator::result_list_t PhaseSimulator::make_results_from_todo(simulation_todo_sptr_t& todo)
{
  result_list_t result; 
  bool has_next = false;
  variable_map_t time_applied_map;
  boost::shared_ptr<RelationGraph> graph;
  
  if(todo->phase_type == PointPhase)
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
    // make dummy phase and push into tree.
    phase_result_sptr_t phase(new PhaseResult(*todo, simulator::INCONSISTENCY));
    todo->parent->children.push_back(phase);
  }
  return result;
}



PhaseSimulator::result_list_t PhaseSimulator::simulate_ms(const hydla::ch::module_set_sptr& ms,
  boost::shared_ptr<RelationGraph>& graph, const variable_map_t &time_applied_map, simulation_todo_sptr_t& todo)
{
  HYDLA_LOGGER_DEBUG("--- next module set ---\n", ms->get_infix_string());
  graph->set_valid(ms.get());
  result_list_t result;
  // TODO:変数の値による分岐も無視している？
  ConstraintStore store;

  backend_->call("resetConstraint", 0, "", "");
  backend_->call("addParameterConstraint", 1, "mp", "", &todo->parameter_map);
  backend_->call("addPrevConstraint", 1, "mvp", "", &time_applied_map);

  if(todo->module_set_container != msc_no_init_)
  {
    ConstraintStore sub_store =
      calculate_constraint_store(ms, todo);
    if(sub_store.consistent())
    {
      HYDLA_LOGGER_DEBUG("CONSISTENT");
      store.add_constraint_store(sub_store);
    }
    else
    {
      HYDLA_LOGGER_DEBUG("INCONSISTENT");
      todo->module_set_container->mark_nodes(todo->maximal_mss, *ms);
      return result;
    }
  }
  else
  {
    int connected_count = graph->get_connected_count();
    for(int i = 0; i < connected_count; i++){
      module_set_sptr connected_ms = graph->get_component(i);
      HYDLA_LOGGER_DEBUG("\n--- connected module set", i, "/", connected_count, " ---\n", connected_ms->get_infix_string());
      SimulationTodo::ms_cache_t::iterator ms_it = todo->ms_cache.find(*connected_ms);
      if(ms_it != todo->ms_cache.end())
      {
        store.add_constraint_store(ms_it->second);
      }
      else
      {
        ConstraintStore sub_store =
          calculate_constraint_store(connected_ms, todo);
        if(sub_store.consistent())
        {
          HYDLA_LOGGER_DEBUG("CONSISTENT");
          todo->ms_cache.insert(std::make_pair(*connected_ms, sub_store) );
          store.add_constraint_store(sub_store);
        }
        else
        {

          HYDLA_LOGGER_DEBUG("INCONSISTENT");
          //if(opts_->use_unsat_core) mark_nodes_by_unsat_core(ms, todo, time_applied_map);
          //else
          todo->module_set_container->mark_nodes(todo->maximal_mss, *connected_ms);
          return result;
        }
      }
    }
  }
  todo->module_set_container->mark_nodes();
  if(!(opts_->nd_mode || opts_->interactive_mode)) todo->module_set_container->reset(module_set_list_t());
  todo->maximal_mss.push_back(ms);

  phase_result_sptr_t phase = make_new_phase(todo, store);
  phase->module_set = ms;

  // 変数表はここで作成する
  vector<variable_map_t> create_result;
  if(phase->phase_type == PointPhase)
  {
    backend_->call("createVariableMap", 0, "", "cv", &create_result);
  }
  else
  {
    backend_->call("createVariableMapInterval", 0, "", "cv", &create_result);
  }
  if(create_result.size() != 1)
  {
    throw SimulateError("result variable map is not single.");
  }
  phase->variable_map = create_result[0];


/*
if(opts_->reuse && todo->module_set_container == msc_no_init_){
  set_changed_variables(phase);
  if(phase->phase == IntervalPhase && phase->parent.get() && phase->parent->parent.get())
  {
    for(auto var_entry : phase->variable_map)
    {
      bool changed = false;
      for(auto var_name : phase->changed_variables)
      {
        if(var_entry.first.get_name() == var_name)
        {
          bool changed = false;
          for(auto var_name : phase->changed_variables)
          {
            if(var_entry.first.get_name() == var_name)
            {
              changed = true;
            }
          }
          if(!changed)
          {
            value_t val = phase->parent->parent->variable_map[var_entry.first].get_unique();
            value_t ret;
            backend_->call("exprTimeShiftInverse", 2, "vltvlt", "vl", &val, &todo->current_time, &ret);
            phase->variable_map[var_entry.first].set_unique(ret);            
          }
        }
      }    }
  }
}
*/

    if(opts_->assertion || break_condition_.get() != NULL){
      timer::Timer entailment_timer;
      
      backend_->call("resetConstraintForVariable", 0, "","");
      std::string fmt = "mv0";
      fmt += (phase->phase_type==PointPhase)?"n":"t";
      backend_->call("addConstraint", 1, fmt.c_str(), "", &phase->variable_map);
      backend_->call("resetConstraintForParameter", 1, "mp", "", &phase->parameter_map);
     
      if(opts_->assertion)
      {
        HYDLA_LOGGER_DEBUG("%% check_assertion");
        CheckConsistencyResult cc_result;
        switch(check_entailment(cc_result, node_sptr(new parse_tree::Not(opts_->assertion)), continuity_map_t(), todo->phase_type)){
        case ENTAILED:
        case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
          std::cout << "Assertion Failed!" << std::endl;
          HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
          phase->cause_for_termination = ASSERTION;
          break;
        case CONFLICTING:
          break;
        case BRANCH_PAR:
          HYDLA_LOGGER_DEBUG("%% failure of assertion depends on conditions of parameters");
          push_branch_states(todo, cc_result);
          std::cout << "Assertion Failed!" << std::endl;
          phase->parameter_map = todo->parameter_map;
          HYDLA_LOGGER_DEBUG("%% Assertion Failed!");
          phase->cause_for_termination = ASSERTION;
          break;
        }
        todo->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
      }
      if(break_condition_.get() != NULL)
      {
        HYDLA_LOGGER_DEBUG("%% check_break_condition");
        CheckConsistencyResult cc_result;
        switch(check_entailment(cc_result, break_condition_, continuity_map_t(), todo->phase_type)){
        case ENTAILED:
        case BRANCH_VAR: //TODO: 変数の値によるので，分岐はすべき
        case BRANCH_PAR: //TODO: 分岐すべき？要検討
          breaking = true;
          break;
        case CONFLICTING:
          break;
        }
      }
    }
    result.push_back(phase);

  return result;
}


void PhaseSimulator::push_branch_states(simulation_todo_sptr_t &original, CheckConsistencyResult &result){
  simulation_todo_sptr_t branch_state_false(create_new_simulation_phase(original));
  branch_state_false->initial_constraint_store.add_constraint_store(result.inconsistent_store);
  todo_container_->push_todo(branch_state_false);
  original->initial_constraint_store.add_constraint_store(result.consistent_store);
  backend_->call("resetConstraint", 1, "cs", "", &original->initial_constraint_store);
}


phase_result_sptr_t PhaseSimulator::make_new_phase(simulation_todo_sptr_t& todo, const ConstraintStore& store)
{
  phase_result_sptr_t phase(new PhaseResult(*todo));
  phase->id = ++phase_sum_;
  phase->constraint_store = store;
  todo->parent->children.push_back(phase);
  return phase;
}


phase_result_sptr_t PhaseSimulator::make_new_phase(const phase_result_sptr_t& original)
{
  phase_result_sptr_t phase(new PhaseResult(*original));
  phase->id = ++phase_sum_;
  phase->parent->children.push_back(phase);
  phase->cause_for_termination = simulator::NONE;
  return phase;
}

void PhaseSimulator::initialize(variable_set_t &v,
                                parameter_map_t &p,
                                variable_map_t &m,
                                continuity_map_t& c, 
                                parse_tree_sptr pt,
                                const module_set_container_sptr &msc_no_init)
{
  variable_set_ = &v;
  parameter_map_ = &p;
  variable_map_ = &m;
  phase_sum_ = 0;
  parse_tree_ = pt;
  msc_no_init_ = msc_no_init;
  const hydla::simulator::module_set_sptr ms = msc_no_init->get_max_module_set();
  
  // TODO:RelationGraph上では，ASSERT文を無視しているため，制約モジュール中では独立でもASSERT文の中で関係している変数があった場合に正しく動作しない
  // RelationGraphを作る上では，ASSERTを特殊な「モジュールのようなもの」として扱う必要がある．
  pp_relation_graph_ = RelationGraph::new_graph(*ms, *variable_set_, true);
  ip_relation_graph_ = RelationGraph::new_graph(*ms, *variable_set_, true);
  
  if(opts_->dump_relation){
    pp_relation_graph_->dump_graph(std::cout);
    ip_relation_graph_->dump_graph(std::cout);
    exit(EXIT_SUCCESS);
  }
  
  AskCollector ac(ms);
  always_set_t eat;
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
  backend_->set_variable_set(*variable_set_);
}


simulation_todo_sptr_t PhaseSimulator::create_new_simulation_phase(const simulation_todo_sptr_t& old) const
{
  simulation_todo_sptr_t sim(new SimulationTodo(*old));
  return sim;
}

bool PhaseSimulator::check_include_bound(value_t tmp_variable_phase, value_t tmp_variable_past, parameter_map_t pm1, parameter_map_t pm2)
{
  HYDLA_LOGGER_DEBUG("");
  bool ret;
  backend_->call("checkIncludeBound", 4, "vlnvlnmpmp", "b", &tmp_variable_phase, &tmp_variable_past, &pm1, &pm2, &ret);
  return ret;
}

void PhaseSimulator::substitute_parameter_condition(phase_result_sptr_t pr, parameter_map_t pm)
{
  HYDLA_LOGGER_DEBUG("");
	// 変数に代入
	variable_map_t ret;
  variable_map_t &vm = pr->variable_map;
  for(variable_map_t::iterator it = vm.begin();
      it != vm.end(); it++)
  {
    if(it->second.undefined())continue;
    assert(it->second.unique());
    value_t tmp_val = it->second.get_unique();
    backend_->call("substituteParameterCondition",
                   2, "vlnmp", "vl", &tmp_val, &pm, &tmp_val);
    it->second.set_unique(tmp_val);
  }

	// 時刻にも代入
  HYDLA_LOGGER_DEBUG("");
  backend_->call("substituteParameterCondition",
                 2, "vlnmp", "vl", &pr->current_time, &pm, &pr->current_time);
	if(pr->phase_type == IntervalPhase){
    backend_->call("substituteParameterCondition",
                   2, "vlnmp", "vl", &pr->end_time, &pm, &pr->end_time);
	}
}

void PhaseSimulator::replace_prev2parameter(
                                            phase_result_sptr_t &phase,
                                            variable_map_t &vm,
                                            parameter_map_t &parameter_map)
{
  assert(phase->parent.get() != NULL);

  PrevReplacer replacer(parameter_map, phase, *simulator_, opts_->approx);
  for(variable_map_t::iterator it = vm.begin();
      it != vm.end(); it++)
  {
    HYDLA_LOGGER_DEBUG(it->first, it->second);
    ValueRange& range = it->second;
    value_t val;
    if(range.unique())
    {
      val = range.get_unique();
      HYDLA_LOGGER_DEBUG(val);
      replacer.replace_value(val);
      range.set_unique(val);
    }
    else
    {
      if(range.get_upper_cnt()>0)
      {
        val = range.get_upper_bound().value;
        replacer.replace_value(val);
        range.set_upper_bound(val, range.get_upper_bound().include_bound);
      }
      if(range.get_lower_cnt() > 0)
      {
        val = range.get_lower_bound().value;
        replacer.replace_value(val);
        range.set_lower_bound(val, range.get_lower_bound().include_bound);
      }
    }
  }
}

void PhaseSimulator::set_break_condition(node_sptr break_cond)
{
  break_condition_ = break_cond;
}

PhaseSimulator::node_sptr PhaseSimulator::get_break_condition()
{
  return break_condition_;
}

ConstraintStore PhaseSimulator::apply_time_to_constraints(const ConstraintStore &original_store, const value_t &time)
{
  ConstraintStore applied_store;
  backend_->call("applyTime2Expr", 2, "csnvlt", "cs", &original_store, &time, &applied_store);
  return applied_store;
}
