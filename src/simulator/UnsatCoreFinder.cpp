#include <iostream>
#include <ostream>
#include <fstream>

#include "UnsatCoreFinder.h"
#include "ModuleSet.h"
#include "ContinuityMapMaker.h"

#include "../backend/Backend.h"
#include "Simulator.h"

using namespace std;
using namespace hydla::simulator;
using namespace hydla::symbolic_expression;
using namespace hydla::backend;
using namespace hydla::hierarchy;

namespace hydla{
namespace simulator{

UnsatCoreFinder::UnsatCoreFinder(backend_sptr_t back):backend_(back){}

UnsatCoreFinder::UnsatCoreFinder(){}

UnsatCoreFinder::~UnsatCoreFinder(){}

void UnsatCoreFinder::reset(PhaseType phase, const variable_map_t &vm, const parameter_map_t &pm)
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
    cout << it->first.second << " : " << get_infix_string((it->first.first)) << endl;
    cout << it->second.get_name() << endl;
  }
  for(unsat_continuities_t::iterator it = S4C.begin();it !=S4C.end();it++ )
  {
    cout << "continuity : " << it->first.first;
    for(int i=0;i<it->first.second;i++)
    {
      cout << "'";
    }
    cout << endl;
    cout << it->second.get_name() << endl;
  }
  cout << "---------------------" << endl;

}

/* TODO: implement
void UnsatCoreFinder::find_unsat_core(const module_set_sptr& ms,
    unsat_constraints_t& S,
    unsat_continuities_t& S4C,
    simulation_job_sptr_t& todo,
    const variable_map_t& vm
)
{
  find_unsat_core(ms, S, S4C, todo->positive_asks, todo->negative_asks, vm, todo->parameter_map, todo->phase_type);
}
*/

/* TODO: implement
void UnsatCoreFinder::find_unsat_core(
  const module_set_sptr& ms,
  unsat_constraints_t& S,
  unsat_continuities_t& S4C,
  const asks_t &positive_asks,
  const asks_t &negative_asks,
  const variable_map_t& vm,
  const parameter_map_t &pm,
  PhaseType phase_type
)
{
  HYDLA_LOGGER_DEBUG("");
  always_set_t expanded_always;
  tells_t tell_list;
  ConstraintStore constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;
  asks_t tmp_negative;

  reset(phase_type, vm, pm);

  add_constraints(S, S4C, phase_type);

  for(auto module : *ms){
    module_set_sptr temp_ms(new hydla::hierarchy::ModuleSet());
    temp_ms->add_module(module);
    TellCollector tell_collector(temp_ms);

    symbolic_expression::node_sptr condition_node;

    tell_collector.collect_all_tells(&tell_list,
        &expanded_always,
        &positive_asks);

    maker.reset();
    constraint_list.clear();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.add_constraint((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    for(auto constraint : constraint_list)
    {
      const char* fmt = (phase_type == POINT_PHASE)?"en":"et";
      backend_->call("addConstraint", 1, fmt, "", &(constraint));
      if(check_inconsistency(phase_type)){
        S.insert(make_pair(make_pair(constraint,"constraint"),temp_ms));
        if(check_unsat_core(S,S4C,temp_ms,phase_type, vm, pm)){
          return ;
        }else{
          find_unsat_core(ms,S,S4C,positive_asks, negative_asks, vm, pm, phase_type);
          return;
        }

      }
    }

    asks_t::iterator it = positive_asks.begin();
    for(int j = 0; j < (int)positive_asks.size(); j++, it++)
    {
      const char* fmt = (phase_type == POINT_PHASE)?"en":"et";
      backend_->call("addConstraint", 1, fmt, "", &(*it)->get_guard());
      if(check_inconsistency(phase_type)){
        S.insert(make_pair(make_pair(symbolic_expression::node_sptr((*it)->get_guard()),"guard"),temp_ms));
        if(check_unsat_core(S,S4C,temp_ms,phase_type, vm, pm)){
          return;
        }else{
          find_unsat_core(ms,S,S4C,positive_asks, negative_asks, vm, pm, phase_type);
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
      if(phase_type == POINT_PHASE)
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
          if(check_inconsistency(phase_type)){
            S4C.insert(make_pair(make_pair(it->first,it->second),temp_ms));
            if(check_unsat_core(S,S4C,temp_ms,phase_type, vm, pm)){
              return ;
            }else{
              find_unsat_core(ms,S,S4C,positive_asks, negative_asks, vm, pm, phase_type);
              return;
            }
          }
        }
      }else{
        symbolic_expression::node_sptr lhs(new symbolic_expression::Variable(it->first));
        for(int i=0; i<=-it->second;i++){
          variable_t var(it->first, i);
          backend_->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
          if(check_inconsistency(phase_type)){
            S4C.insert(make_pair(make_pair(it->first,it->second),temp_ms));
            if(check_unsat_core(S,S4C,temp_ms, phase_type, vm, pm)){
              return ;
            }else{
              find_unsat_core(ms,S,S4C,positive_asks, negative_asks, vm, pm, phase_type);
              return;
            }
          }
          lhs = symbolic_expression::node_sptr(new Differential(lhs));
        }
        symbolic_expression::node_sptr rhs(new Number("0"));
        symbolic_expression::node_sptr cons(new Equal(lhs, rhs));
        std::string fmt = "v";
        if(phase_type == POINT_PHASE)
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
        if(check_inconsistency(phase_type)){
          S.insert(make_pair(make_pair(cons,"constraint"),temp_ms));
          if(check_unsat_core(S,S4C,temp_ms,phase_type, vm, pm)){
            return ;
          }else{
            find_unsat_core(ms,S,S4C,positive_asks, negative_asks, vm, pm, phase_type);
            return;
          }
        }
      }
    }
  }
  backend_->call("endTemporary", 0, "", "");
}
*/


bool UnsatCoreFinder::check_inconsistency(PhaseType phase_type){
  CheckConsistencyResult check_consistency_result;
  std::string func_name;
  if(phase_type == POINT_PHASE)
  {
    func_name = "checkConsistencyPoint";
  }
  else
  {
    func_name = "checkConsistencyInterval";
  }
  backend_->call(func_name.c_str(), 0, "", "cc", &check_consistency_result);
  if(!check_consistency_result.consistent_store.consistent()){
    return true;
  }else{
    return false;
  }
}

/* TODO: implement
bool UnsatCoreFinder::check_unsat_core(unsat_constraints_t S,unsat_continuities_t S4C,const module_set_sptr& ms, PhaseType phase_type, const variable_map_t& vm, const parameter_map_t& pm){
  backend_->call("endTemporary", 0, "", "");
  backend_->call("startTemporary", 0, "", "");
  reset(phase_type, vm, pm);
  add_constraints(S, S4C, phase_type);
  bool ret = check_inconsistency(phase_type);

  backend_->call("endTemporary", 0, "", "");
  return ret;
}
*/

void UnsatCoreFinder::set_backend(backend_sptr_t back){
  backend_ = back;
}


void UnsatCoreFinder::add_constraints(unsat_constraints_t S,unsat_continuities_t S4C, PhaseType phase){
  for(unsat_constraints_t::iterator it = S.begin();it !=S.end();it++ )
  {
    const char* fmt = (phase == POINT_PHASE)?"en":"et";
    if(it->first.second == "guard" || it->first.second == "constraint"){
      backend_->call("addConstraint", 1, fmt, "", &it->first.first);
    }
  }
  for(unsat_continuities_t::iterator it = S4C.begin();it !=S4C.end();it++ )
  {
    std::string fmt = "v";
    if(phase == POINT_PHASE)fmt += "n";
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
      symbolic_expression::node_sptr rhs(new Number("0"));
      std::string fmt = "v";
      if(phase == POINT_PHASE) fmt += "n";
      else fmt += "z";
      fmt += "vp"; 
      variable_t var(name, diff_cnt + 1);
      backend_->call("addEquation", 2, fmt.c_str(), "", &var, &rhs);
    }
  }
}

}
}
