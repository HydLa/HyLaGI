#include <iostream>
#include <ostream>
#include <fstream>

#include "UnsatCoreFinder.h"
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


void UnsatCoreFinder::print_unsat_cores(map<node_sptr,string> S,map<const std::string,int> S4C){

  cout << "-----unsat core------" << endl;
  for(map<node_sptr,string>::iterator it = S.begin();it !=S.end();it++ )
  {
    //cout << it->second << " : " << *(it->first) << endl;
    std::cout << it->second << " : " << TreeInfixPrinter().get_infix_string((it->first)) << std::endl;
  }
  for(map<const string, int>::iterator it = S4C.begin();it !=S4C.end();it++ )
  {
    cout << "continuity : " << it->first;
    for(int i=0;i<it->second;i++)
    {
      cout << "'";
    }
    cout << endl;
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
    map<node_sptr,string> S,
    map<const std::string,int> S4C,
    simulation_todo_sptr_t& todo,
    const variable_map_t& vm
)
{

  positive_asks_t positive_asks = todo->positive_asks;
  negative_asks_t negative_asks = todo->negative_asks;
  expanded_always_t expanded_always;
  TellCollector tell_collector(ms);
  AskCollector ask_collector(ms);
  tells_t tell_list;
  constraints_t constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  negative_asks_t tmp_negative;

  //variable_map_t tvm;
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

  ask_collector.collect_ask(&expanded_always,
      &positive_asks,
      &tmp_negative,
      &negative_asks);

  node_sptr condition_node;
  //map<node_sptr,string> S;
  //map<const std::string,int> S4C;
  add_constraints(S,S4C);


  //for(int i = 0; i < (1 << negative_asks.size()); i++){


    //negative_asks_t::iterator it = negative_asks.begin();
    //for(int j = 0; j < (int)negative_asks.size(); j++, it++){
    //  if((i & (1 << j)) != 0){
    //    solver_->add_guard((*it)->get_guard());
    //    if(check_inconsistency()){
    //      //TODO
    //      S.insert(pair<node_sptr,string>(node_sptr((*it)->get_guard()),"guard"));
    //      if(check_unsat_core(S,S4C,ms,todo,vm)){
    //        return;
    //      }else{
    //        find_unsat_core(ms,S,S4C,todo,vm);
    //      }
    //    }
    //    tmp_positive_asks.insert(*it);
    //  }else{
    //    solver_->add_guard(node_sptr(new Not((*it)->get_guard())));
    //    if(check_inconsistency()){
    //      //TODO
    //      S.insert(pair<node_sptr,string>(node_sptr(new Not((*it)->get_guard())),"guard"));
    //      if(check_unsat_core(S,S4C,ms,todo,vm)){
    //        cout << 111 << endl;
    //        return;
    //      }else{
    //        find_unsat_core(ms,S,S4C,todo,vm);
    //      }

    //    }
    //  }
    //}

    tell_collector.collect_all_tells(&tell_list,
        &expanded_always,
        &positive_asks);

    maker.reset();
    constraint_list.clear();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    for(constraints_t::iterator it = constraint_list.begin();
        it != constraint_list.end();
        it++)
    {
      solver_->add_constraint(*it);
      if(check_inconsistency()){
        S.insert(pair<node_sptr,string>(*it,"constraint"));
        if(check_unsat_core(S,S4C,ms,todo,vm)){
          return ;
        }else{
          find_unsat_core(ms,S,S4C,todo,vm);
          return;
        }

      }
    }
    positive_asks_t::iterator it = positive_asks.begin();
    for(int j = 0; j < (int)positive_asks.size(); j++, it++){
      solver_->add_guard((*it)->get_guard());
      if(check_inconsistency()){
        //TODO
        S.insert(pair<node_sptr,string>(node_sptr((*it)->get_guard()),"guard"));
        if(check_unsat_core(S,S4C,ms,todo,vm)){
          return;
        }else{
          find_unsat_core(ms,S,S4C,todo,vm);
          return;
        }
      }

    }
    continuity_map = maker.get_continuity_map();
    for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){
      if(it->second>=0){
        for(int i=0; i<it->second;i++){
          solver_->set_continuity(it->first, i);
          if(check_inconsistency()){
            S4C.insert(pair<const string,int>(it->first,it->second));
            if(check_unsat_core(S,S4C,ms,todo,vm)){
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
            S4C.insert(pair<const string,int>(it->first,it->second));
            if(check_unsat_core(S,S4C,ms,todo,vm)){
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
          S.insert(pair<node_sptr,string>(cons,"constraint"));
          if(check_unsat_core(S,S4C,ms,todo,vm)){
            return ;
          }else{
            find_unsat_core(ms,S,S4C,todo,vm);
            return;
          }

        }
      }
    }
    
    node_sptr tmp_node;
    solver_->end_temporary();
  //}


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
bool UnsatCoreFinder::check_unsat_core(map<node_sptr,string> S,map<const std::string,int> S4C,const module_set_sptr& ms,simulation_todo_sptr_t& todo, const variable_map_t& vm){
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
  //cout << ret << endl;
  if(ret){
    print_unsat_cores(S,S4C);
  }else{
    //cout << 1111<< endl;
    //print_unsat_cores(S,S4C);
    //cout << 1111<< endl;
  }
  solver_->end_temporary();
  return ret;
}
void UnsatCoreFinder::add_constraints(map<node_sptr,string> S,map<const std::string,int> S4C){
  //cout << "start add constraints" << endl;
  for(map<node_sptr,string>::iterator it = S.begin();it !=S.end();it++ )
  {
    //cout << *(it->first) << endl;
    if(it->second == "guard"){
      solver_->add_guard(it->first);
    }
    else if(it->second == "constraint"){
      solver_->add_constraint(it->first);
    }
  }
  for(map<const string, int>::iterator it = S4C.begin();it !=S4C.end();it++ )
  {
    if(it->second>=0){
      for(int i=0; i<it->second;i++){
        solver_->set_continuity(it->first, i);
      }
    }else{
      node_sptr lhs(new Variable(it->first));
      for(int i=0; i<=-it->second;i++){
        solver_->set_continuity(it->first, i);
      }
      node_sptr rhs(new Number("0"));
      node_sptr cons(new Equal(lhs, rhs));
      solver_->add_constraint(cons);
    }
  }
  //cout << "end add constraints" << endl;
}

}
}
}
