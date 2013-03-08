#include "ConstraintAnalyzer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"

#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"

using namespace std;
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;
using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;


namespace hydla{
namespace symbolic_simulator{

ConstraintAnalyzer::ConstraintAnalyzer(){}

ConstraintAnalyzer::~ConstraintAnalyzer(){}

void ConstraintAnalyzer::output_false_conditions(){
  false_map_t::iterator it = false_conditions_.begin();
  for(;it != false_conditions_.end();it++){
    std::cout << (*it).first << " : ";
    if((*it).second != NULL){
      std::cout << TreeInfixPrinter().get_infix_string((*it).second);
    }
    std::cout << std::endl;
  }
}


simulation_phase_sptr_t ConstraintAnalyzer::create_new_simulation_phase(const simulation_phase_sptr_t& old) const
{
  simulation_phase_sptr_t sim(new simulation_phase_t(*old));
  sim->phase_result.reset(new phase_result_t(*old->phase_result));
  sim->phase_result->cause_of_termination = NONE;
  return sim;
}


void ConstraintAnalyzer::initialize(module_set_container_sptr msc_no_init, boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver)
{
  solver_ = solver;

  msc_no_init->reset();

  while(msc_no_init->go_next()){
    false_conditions_.insert(false_map_t::value_type(msc_no_init->get_module_set()->get_name(),node_sptr()));
    msc_no_init->mark_current_node();
  }
}

void ConstraintAnalyzer::check_all_module_set(module_set_container_sptr& msc_no_init, const Opts& opts){
  msc_no_init->reverse_reset();
  while(msc_no_init->reverse_go_next()){
    FalseConditionsResult result = find_false_conditions(msc_no_init->get_reverse_module_set(), opts);
    /*
    if(result != FALSE_CONDITIONS_FALSE){
        msc_no_init->mark_super_module_set();
    }
    */
    msc_no_init->mark_r_current_node();
  }
}

  ConstraintAnalyzer::FalseConditionsResult ConstraintAnalyzer::find_false_conditions(const module_set_sptr& ms, const Opts& opts){
  solver_->change_mode(FalseConditionsMode, opts.approx_precision);
  ConstraintAnalyzer::FalseConditionsResult ret = FALSE_CONDITIONS_FALSE;

  map<module_set_sptr, node_sptr> tmp_false_conditions;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  expanded_always_t expanded_always;
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms);
  tells_t tell_list;
  constraints_t constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  negative_asks_t tmp_negative;

  variable_map_t vm;
  parameter_map_t pm;
  solver_->reset(vm, pm);

  ask_collector.collect_ask(&expanded_always,
			    &positive_asks,
			    &tmp_negative,
			    &negative_asks);

  node_sptr condition_node;

  for(int i = 0; i < (1 << negative_asks.size()) && ret != FALSE_CONDITIONS_TRUE; i++){
    positive_asks_t tmp_positive_asks;
    solver_->start_temporary();
    
    negative_asks_t::iterator it = negative_asks.begin();
    for(int j = 0; j < (int)negative_asks.size(); j++, it++){
      if((i & (1 << j)) != 0){
        solver_->add_guard((*it)->get_guard());
        tmp_positive_asks.insert(*it);
      }else{
        solver_->add_guard(node_sptr(new Not((*it)->get_guard())));
      }
    }

    tell_collector.collect_all_tells(&tell_list,
				     &expanded_always,
				     &tmp_positive_asks);

    maker.reset();
    constraint_list.clear();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    solver_->add_constraint(constraint_list);
    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map);

    {
      node_sptr tmp_node;
      switch(solver_->find_false_conditions(tmp_node)){
      case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_TRUE:
        ret = FALSE_CONDITIONS_TRUE;
        break;
      case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_FALSE:
        break;
      case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_VARIABLE_CONDITIONS:
        if(condition_node == NULL) condition_node = node_sptr(tmp_node);
        else condition_node = node_sptr(new LogicalOr(condition_node, tmp_node));
        ret = FALSE_CONDITIONS_VARIABLE_CONDITIONS;
        break;
      default:
        assert(0);
        break;
      }
    }
    solver_->end_temporary();
  }
  switch(ret){
  case FALSE_CONDITIONS_VARIABLE_CONDITIONS:
    solver_->node_simplify(condition_node);
    if(condition_node == NULL){
      false_conditions_.erase(ms->get_name());
      /*
      if(opts.optmization_level >= 3){
        false_map_t::iterator it = tmp_false_conditions.begin();
        while(it != tmp_false_conditions.end()){
          if((*it).first->is_super_set(*ms)){
            tmp_false_conditions.erase(it++);
          }else{
            it++;
          }
        }
      }
       */
      ret = FALSE_CONDITIONS_TRUE;
    }else{
      false_conditions_[ms->get_name()] = condition_node;
      /*
      false_map_t::iterator it = tmp_false_conditions.begin();
      while(it != tmp_false_conditions.end() && opts.optimization_level >= 3){
        if((*it).first->is_super_set(*ms) && (*it).first != ms){
          node_sptr tmp = (*it).second;
          if(tmp != NULL) tmp = node_sptr(new LogicalOr(tmp, condition_node));
          else tmp = node_sptr(condition_node);
          switch(solver_->node_simplify(tmp){
	  case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_TRUE;
            tmp_false_conditions.erase(it++);
            break;
          case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_FALSE;
            it++;
            break;
          case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_VARIABLE_CONDITIONS:
            (*it).second = tmp;
            it++;
            break;
          default:
            assert(0);
            break;
          }
        }else{
          it++;
        }
      }
       */
    }
    break;
  case FALSE_CONDITIONS_TRUE:
    false_conditions_.erase(ms->get_name());
    /*
    if(opts.optimization_level >= 3){ 
      false_map_t::iterator it = tmp_false_conditions.begin();
      while(it != tmp_false_conditions.end()){
        if((*it).first->is_super_set(*ms)){
          tmp_false_conditions.erase(it++);
        }else{
          it++;
        }
      }
     }
     */
    break;
  case FALSE_CONDITIONS_FALSE:
    false_conditions_[ms->get_name()] = node_sptr();
    break;
  default:
    assert(0);
    break;
  }
  return ret;
}

SymbolicSimulator::CalculateVariableMapResult
ConstraintAnalyzer::check_false_conditions(
  const Opts& opts,
  const module_set_sptr& ms,
  simulation_phase_sptr_t& state,
  const variable_map_t& vm,
  variable_map_t& result_vm,
  todo_and_results_t& result_todo)
{
  phase_result_sptr_t& pr = state->phase_result;
  if(pr->current_time->get_string() != "0"){
    if(opts.optimization_level == 2){
      if(false_conditions_.find(ms->get_name()) == false_conditions_.end()){
        return SymbolicSimulator::CVM_INCONSISTENT;
      }
      if(checkd_module_set_.find(ms) == checkd_module_set_.end()){
        checkd_module_set_.insert(ms);
        if(find_false_conditions(ms,opts) == FALSE_CONDITIONS_TRUE){
          return SymbolicSimulator::CVM_INCONSISTENT;
        }
      }
    }
    solver_->reset(vm, pr->parameter_map);
    if(false_conditions_[ms->get_name()] != NULL){
      solver_->change_mode(FalseConditionsMode, opts.approx_precision);
      solver_->set_false_conditions(false_conditions_[ms->get_name()]);

      SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
      if(check_consistency_result.true_parameter_maps.empty()){
        return SymbolicSimulator::CVM_INCONSISTENT;
      }else if(check_consistency_result.false_parameter_maps.empty()){
      }else{
        CalculateClosureResult result;
        push_branch_states(state, check_consistency_result, result);
        for(unsigned int i = 0; i < result.size(); i++){
          result_todo.push_back(PhaseSimulator::TodoAndResult(result[i], phase_result_sptr_t()));
        }
        return SymbolicSimulator::CVM_BRANCH;
      }
    }
  }
  return SymbolicSimulator::CVM_CONSISTENT;
}

void ConstraintAnalyzer::push_branch_states(simulation_phase_sptr_t &original, SymbolicVirtualConstraintSolver::check_consistency_result_t &result, CalculateClosureResult &dst){
  for(int i=0; i<(int)result.true_parameter_maps.size();i++){
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->phase_result->parameter_map = result.true_parameter_maps[i];
    dst.push_back(branch_state);
  }
  for(int i=0; i<(int)result.false_parameter_maps.size();i++){
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->phase_result->parameter_map = result.false_parameter_maps[i];
    dst.push_back(branch_state);
  }
}
void ConstraintAnalyzer::add_continuity(const continuity_map_t& continuity_map){
  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){
    if(it->second>=0){
      for(int i=0; i<it->second;i++){
        solver_->set_continuity(it->first, i);
      }
    }else{
      node_sptr lhs(new Variable(it->first));
      for(int i=0; i<=-it->second;i++){
        solver_->set_continuity(it->first, i);
        lhs = node_sptr(new Differential(lhs));
      }
      node_sptr rhs(new Number("0"));
      node_sptr cons(new Equal(lhs, rhs));
      solver_->add_constraint(cons);
    }
  }
}


}
}
