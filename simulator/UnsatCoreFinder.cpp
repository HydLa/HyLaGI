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

#include "../backend/Backend.h"

using namespace std;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::backend;
using namespace hydla::ch;

namespace hydla{
namespace simulator{

UnsatCoreFinder::UnsatCoreFinder(hydla::backend::Backend *back):backend_(back){}

UnsatCoreFinder::UnsatCoreFinder(){}

UnsatCoreFinder::~UnsatCoreFinder(){}

void UnsatCoreFinder::reset(Phase phase, const variable_map_t &vm, const parameter_map_t &pm)
{
  backend_->call("resetConstraintForVariable", 0, "","");
  std::string fmt = "mvp";
  backend_->call("addPrevConstraint", 1, fmt.c_str(), "", &vm);
  backend_->call("resetConstraintForParameter", 1, "mp", "", &pm);     
}

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

void UnsatCoreFinder::find_unsat_core(const module_set_sptr& ms,
    unsat_constraints_t& S,
    unsat_continuities_t& S4C,
    simulation_todo_sptr_t& todo,
    const variable_map_t& vm
)
{
  HYDLA_LOGGER_LOCATION(MS);
  positive_asks_t positive_asks = todo->positive_asks;
  negative_asks_t negative_asks = todo->negative_asks;
  expanded_always_t expanded_always;
    tells_t tell_list;
  constraints_t constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  negative_asks_t tmp_negative;

  parameter_map_t pm = todo->parameter_map;

  reset(todo->phase, vm, pm);


  variable_map_t::const_iterator vm_it  = vm.begin();
  variable_map_t::const_iterator vm_end = vm.end();
  for(constraints_t::const_iterator it = todo->temporary_constraints.begin(); it != todo->temporary_constraints.end(); it++){
    cout << "temp" << endl;
    cout << *it << endl;
  }

  add_constraints(S, S4C, todo->phase);

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

    for(constraints_t::iterator it = constraint_list.begin();
        it != constraint_list.end();
        it++)
    {
      const char* fmt = (todo->phase == PointPhase)?"en":"et";
      backend_->call("addConstraint", 1, fmt, "", &(*it));
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

    positive_asks_t::iterator it = positive_asks.begin();
    for(int j = 0; j < (int)positive_asks.size(); j++, it++)
    {
      const char* fmt = (todo->phase == PointPhase)?"en":"et";
      backend_->call("addConstraint", 1, fmt, "", &(*it)->get_guard());
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

      std::string fmt = "v";
      if(todo->phase == PointPhase)
      {
        fmt += "n";
      }
      else
      {
        fmt += "z";
      }
      fmt += "vp"; 
      if(it->second>=0){
        for(int i = 0; i < it->second; i++){
          variable_t var(it->first, i);
          backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
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
          variable_t var(it->first, i);
          backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
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
        std::string fmt = "v";
        if(todo->phase == PointPhase)
        {
          fmt += "n";
        }
        else
        {
          fmt += "z";
        }
        fmt += "vp"; 
        variable_t var(it->first, -it->second + 1);
        backend_->call("addEquation", 2, fmt.c_str(), "", &var, &rhs);
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
  backend_->call("endTemporary", 0, "", "");
}

bool UnsatCoreFinder::check_inconsistency(){
  CheckConsistencyResult check_consistency_result;
  backend_->call("checkConsistencyPoint", 0, "", "cc", &check_consistency_result);
  if(check_consistency_result[0].empty()){
    return true;
  }else{
    return false;
  }
}


bool UnsatCoreFinder::check_unsat_core(unsat_constraints_t S,unsat_continuities_t S4C,const module_set_sptr& ms,simulation_todo_sptr_t& todo, const variable_map_t& vm){
  backend_->call("endTemporary", 0, "", "");
  backend_->call("startTemporary", 0, "", "");
  reset(todo->phase, vm, todo->parameter_map);
  add_constraints(S, S4C, todo->phase);
  bool ret = check_inconsistency();

  backend_->call("endTemporary", 0, "", "");
  return ret;
}

void UnsatCoreFinder::set_backend(Backend *back){
  backend_ = back;
}


void UnsatCoreFinder::add_constraints(unsat_constraints_t S,unsat_continuities_t S4C, Phase phase){
  for(unsat_constraints_t::iterator it = S.begin();it !=S.end();it++ )
  {
    const char* fmt = (phase == PointPhase)?"en":"et";
    if(it->first.second == "guard" || it->first.second == "constraint"){
      backend_->call("addConstraint", 1, fmt, "", &it->first.first);
    }
  }
  for(unsat_continuities_t::iterator it = S4C.begin();it !=S4C.end();it++ )
  {
    std::string fmt = "v";
    if(phase == PointPhase)fmt += "n";
    else fmt += "z";
    fmt += "vp";
    const string &name = it->first.first;
    int diff_cnt= it->first.second;
    if(diff_cnt >= 0){
      for(int i = 0; i < diff_cnt; i++){
        variable_t var(name, i);
        backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
    }else{
      for(int i = 0; i <= -diff_cnt;i++){
        variable_t var(name, i);
        backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
      node_sptr rhs(new Number("0"));
      std::string fmt = "v";
      if(phase == PointPhase) fmt += "n";
      else fmt += "z";
      fmt += "vp"; 
      variable_t var(name, diff_cnt + 1);
      backend_->call("addEquation", 2, fmt.c_str(), "", &var, &rhs);
    }
  }
}

}
}