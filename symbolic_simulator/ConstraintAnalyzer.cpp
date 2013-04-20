#include <iostream>
#include <ostream>
#include <fstream>

#include "ConstraintAnalyzer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"

#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"

using namespace std;
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;


namespace hydla{
namespace symbolic_simulator{

ConstraintAnalyzer::ConstraintAnalyzer(Opts& opts):Simulator(opts){}

ConstraintAnalyzer::~ConstraintAnalyzer(){}

void ConstraintAnalyzer::print_false_conditions()
{
  false_map_t::iterator it = false_conditions_.begin();
  std::ofstream ofs;
  if(opts_->analysis_file != ""){
    ofs.open(opts_->analysis_file.c_str());
  }
  for(;it != false_conditions_.end();it++){
    if(opts_->analysis_file != ""){
      ofs << (*it).first << ":";
      if((*it).second != NULL){
        ofs << *((*it).second);
      }
      ofs << std::endl;
    }else{
      std::cout << (*it).first << ":";
      if((*it).second != NULL){
	      std::cout << *((*it).second);
      }
      std::cout << std::endl;
    }
  }
  if(opts_->analysis_file != ""){
    ofs.close();
  }
}

void ConstraintAnalyzer::initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  init_variable_map(parse_tree);
  //  continuity_map_t cont(parse_tree->get_variable_map());

  solver_.reset(new MathematicaVCS(*opts_));
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);
}

void ConstraintAnalyzer::check_all_module_set()
{
  msc_no_init_->reset();
  while(msc_no_init_->go_next()){
    find_false_conditions(msc_no_init_->get_module_set());
    msc_no_init_->mark_current_node();
  }
  /*
  msc_no_init_->reverse_reset();
  while(msc_no_init_->reverse_go_next()){
    FalseConditionsResult result = find_false_conditions(msc_no_init_->get_reverse_module_set());
    if(result != FALSE_CONDITIONS_FALSE){
        msc_no_init_->mark_super_module_set();
    }
    msc_no_init_->mark_r_current_node();
  }
  */
}

ConstraintAnalyzer::FalseConditionsResult ConstraintAnalyzer::find_false_conditions(const module_set_sptr& ms)
{
  solver_->change_mode(FalseConditionsMode, opts_->approx_precision);
  ConstraintAnalyzer::FalseConditionsResult ret = FALSE_CONDITIONS_FALSE;

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
      //      false_conditions_.erase(ms->get_name());
      /*
      if(opts_->optimization_level >= 3){
        false_map_t::iterator it = false_conditions_.begin();
        while(it != false_conditions_.end()){
          if((*it).first->is_super_set(*ms)){
            false_conditions_.erase(it++);
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
      false_map_t::iterator it = false_conditions_.begin();
      while(it != false_conditions_.end() && opts_->optimization_level >= 3){
        if((*it).first->is_super_set(*ms) && (*it).first != ms){
          node_sptr tmp = (*it).second;
          if(tmp != NULL) tmp = node_sptr(new LogicalOr(tmp, condition_node));
          else tmp = node_sptr(condition_node);
          switch(solver_->node_simplify(tmp)){
	  case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_TRUE:
            false_conditions_.erase(it++);
            break;
          case SymbolicVirtualConstraintSolver::FALSE_CONDITIONS_FALSE:
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
    //    false_conditions_.erase(ms->get_name());
    /*
    if(opts_->optimization_level >= 3){ 
      false_map_t::iterator it = false_conditions_.begin();
      while(it != false_conditions_.end()){
        if((*it).first->is_super_set(*ms)){
          false_conditions_.erase(it++);
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

phase_result_const_sptr_t ConstraintAnalyzer::simulate(){
  return phase_result_const_sptr_t();
}


}
}