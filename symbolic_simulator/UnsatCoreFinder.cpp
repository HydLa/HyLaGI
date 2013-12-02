#include <iostream>
#include <ostream>
#include <fstream>

#include "UnsatCoreFinder.h"
#include "ModuleSet.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"
#include "Dumpers.h"
#include "TreeInfixPrinter.h"

#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"

using namespace std;
using namespace hydla::simulator::symbolic;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;


namespace hydla{
namespace simulator{
namespace symbolic {

UnsatCoreFinder::UnsatCoreFinder(const Opts& opts):Simulator(const_cast<Opts&>(opts)){}

UnsatCoreFinder::~UnsatCoreFinder(){}


void UnsatCoreFinder::print_unsat_cores(unsat_constraints_t S,unsat_continuities_t S4C){

  cout << "-----unsat core------" << endl;
  for(unsat_constraints_t::iterator it = S.begin();it !=S.end();it++ )
  {
    std::cout << it->first.second << " : " << TreeInfixPrinter().get_infix_string((it->first.first)) << std::endl;
    cout << it->second << endl;
  }
  for(unsat_continuities_t::iterator it = S4C.begin();it !=S4C.end();it++ )
  {
    cout << "continuity : " << it->first.first;
    for(int i=0;i<it->first.second;i++)
    {
      cout << "'";
    }
    cout << endl;
    cout << it->second << endl;
  }
  cout << "---------------------" << endl;

}

void UnsatCoreFinder::initialize()
{
  init_variable_map();
  solver_.reset(new MathematicaVCS(*opts_));
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);
}
void UnsatCoreFinder::init_variable_map()
{
  variable_set_.reset(new variable_set_t());
  original_range_map_.reset(new variable_range_map_t());
  original_parameter_map_.reset(new parameter_map_t());
  parameter_set_.reset(new parameter_set_t());

}
void UnsatCoreFinder::check_all_module_set()
{
  msc_no_init_->reset();
}

void UnsatCoreFinder::find_unsat_core(const module_set_sptr& ms,
    unsat_constraints_t& S,
    unsat_continuities_t& S4C,
    simulation_todo_sptr_t& todo,
    const variable_map_t& vm
)
{

  positive_asks_t positive_asks = todo->positive_asks;
  negative_asks_t negative_asks = todo->negative_asks;
  expanded_always_t expanded_always;
    tells_t tell_list;
  constraints_t constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  negative_asks_t tmp_negative;

  parameter_map_t pm = todo->parameter_map;

  if(todo->phase == PointPhase){
    solver_->change_mode(DiscreteMode, opts_->approx_precision);
  }
  else
  {
    solver_->change_mode(ContinuousMode, opts_->approx_precision);
  }
  solver_->reset(vm, pm);
  variable_map_t::const_iterator vm_it  = vm.begin();
  variable_map_t::const_iterator vm_end = vm.end();
  for(constraints_t::const_iterator it = todo->temporary_constraints.begin(); it != todo->temporary_constraints.end(); it++){
    cout << "temp" << endl;
    cout << *it << endl;
  }

  add_constraints(S,S4C);

  module_list_t::const_iterator ms_it = ms->begin();
  module_list_t::const_iterator ms_end = ms->end();
  for(;ms_it!=ms_end;ms_it++){
    module_set_sptr temp_ms(new hydla::ch::ModuleSet());
    temp_ms->add_module(*ms_it);
    TellCollector tell_collector(temp_ms);
    AskCollector ask_collector(temp_ms);


    ask_collector.collect_ask(&expanded_always,
        &positive_asks,
        &tmp_negative,
        &negative_asks);

    node_sptr condition_node;

    tell_collector.collect_all_tells(&tell_list,
        &expanded_always,
        &positive_asks);

    maker.reset();
    constraint_list.clear();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    //add constraint
    //*
    for(constraints_t::iterator it = constraint_list.begin();
        it != constraint_list.end();
        it++)
    {
      solver_->add_constraint(*it);
      if(check_inconsistency()){
        S.insert(make_pair(make_pair(*it,"constraint"),temp_ms));
        if(check_unsat_core(S,S4C,temp_ms,todo,vm)){
          return ;
        }else{
          find_unsat_core(ms,S,S4C,todo,vm);
          return;
        }

      }
    }
    //*/
    positive_asks_t::iterator it = positive_asks.begin();
    for(int j = 0; j < (int)positive_asks.size(); j++, it++)
    {
      solver_->add_guard((*it)->get_guard());
      if(check_inconsistency()){
        S.insert(make_pair(make_pair(node_sptr((*it)->get_guard()),"guard"),temp_ms));
        if(check_unsat_core(S,S4C,temp_ms,todo,vm)){
          return;
        }else{
          find_unsat_core(ms,S,S4C,todo,vm);
          return;
        }
      }

    }
    //add continuity
    continuity_map = maker.get_continuity_map();
    for(continuity_map_t::const_iterator it = continuity_map.begin();
        it != continuity_map.end();
        it++)
    {
      if(it->second>=0){
        for(int i=0; i<it->second;i++){
          solver_->set_continuity(it->first, i);
          if(check_inconsistency()){
            S4C.insert(make_pair(make_pair(it->first,it->second),temp_ms));
            if(check_unsat_core(S,S4C,temp_ms,todo,vm)){
              return ;
            }else{
              find_unsat_core(ms,S,S4C,todo,vm);
              return;
            }

          }
        }
      }else{
        node_sptr lhs(new Variable(it->first));
        for(int i=0; i<=-it->second;i++){
          solver_->set_continuity(it->first, i);
          if(check_inconsistency()){
            S4C.insert(make_pair(make_pair(it->first,it->second),temp_ms));
            if(check_unsat_core(S,S4C,temp_ms,todo,vm)){
              return ;
            }else{
              find_unsat_core(ms,S,S4C,todo,vm);
              return;
            }

          }
          lhs = node_sptr(new Differential(lhs));
        }
        node_sptr rhs(new Number("0"));
        node_sptr cons(new Equal(lhs, rhs));
        solver_->add_constraint(cons);
        if(check_inconsistency()){
          S.insert(make_pair(make_pair(cons,"constraint"),temp_ms));
          if(check_unsat_core(S,S4C,temp_ms,todo,vm)){
            return ;
          }else{
            find_unsat_core(ms,S,S4C,todo,vm);
            return;
          }

        }
      }
    }
  }

  solver_->check_consistency();
  node_sptr tmp_node;
  solver_->end_temporary();


}

phase_result_const_sptr_t UnsatCoreFinder::simulate(){
  return phase_result_const_sptr_t();
}

bool UnsatCoreFinder::check_inconsistency(){
  CheckConsistencyResult check_consistency_result;
  check_consistency_result= solver_->check_consistency();
  if(check_consistency_result.true_parameter_maps.empty()){
    return true;
  }else{
    return false;
  }
}
bool UnsatCoreFinder::check_unsat_core(unsat_constraints_t S,unsat_continuities_t S4C,const module_set_sptr& ms,simulation_todo_sptr_t& todo, const variable_map_t& vm){
  solver_->end_temporary();
  solver_->start_temporary();
if(todo->phase == PointPhase){
    solver_->change_mode(DiscreteMode, opts_->approx_precision);
  }
  else
  {
    solver_->change_mode(ContinuousMode, opts_->approx_precision);
  }
  solver_->reset(vm,todo->parameter_map);
  add_constraints(S, S4C);
  bool ret = check_inconsistency();
  if(ret){
    //print_unsat_cores(S,S4C);
  }else{
    //print_unsat_cores(S,S4C);
  }
  solver_->end_temporary();
  return ret;
}
void UnsatCoreFinder::add_constraints(unsat_constraints_t S,unsat_continuities_t S4C){
  for(unsat_constraints_t::iterator it = S.begin();it !=S.end();it++ )
  {
    if(it->first.second == "guard"){
      solver_->add_guard(it->first.first);
    }
    else if(it->first.second == "constraint"){
      solver_->add_constraint(it->first.first);
    }
  }
  for(unsat_continuities_t::iterator it = S4C.begin();it !=S4C.end();it++ )
  {
    if(it->first.second>=0){
      for(int i=0; i<it->first.second;i++){
        solver_->set_continuity(it->first.first, i);
      }
    }else{
      node_sptr lhs(new Variable(it->first.first));
      for(int i=0; i<=-it->first.second;i++){
        solver_->set_continuity((it->first).first, i);
      }
      node_sptr rhs(new Number("0"));
      node_sptr cons(new Equal(lhs, rhs));
      solver_->add_constraint(cons);
    }
  }
}

}
}
}
