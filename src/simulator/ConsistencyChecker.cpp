#include "ConsistencyChecker.h"
#include "Backend.h"

#include <iostream>
#include <fstream>

#include "Logger.h"
#include "Timer.h"

#include "ContinuityMapMaker.h"

using namespace std;
using namespace boost;

namespace hydla
{
namespace simulator
{

using namespace hierarchy;
using namespace symbolic_expression;
using namespace logger;
using namespace timer;
using namespace backend;
using namespace backend::mathematica;
using namespace backend::reduce;


ConsistencyChecker::ConsistencyChecker(backend_sptr_t back) : backend(back){}
ConsistencyChecker::~ConsistencyChecker(){}

void ConsistencyChecker::set_backend(backend_sptr_t back){
  backend = back;
}

void ConsistencyChecker::add_continuity(const continuity_map_t& continuity_map, const PhaseType &phase){

//  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){

  for(auto continuity : continuity_map){
    std::string fmt = "v";
    if(phase == PointPhase)
    {
      fmt += "n";
    }
    else
    {
      fmt += "z";
    }
    fmt += "vp";
    if(continuity.second>=0){
      for(int i=0; i<continuity.second;i++){
        variable_t var(continuity.first, i);
        backend->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
    }else{
      for(int i=0; i<=-continuity.second;i++){
        variable_t var(continuity.first, i);
        backend->call("addInitEquation", 2, fmt.c_str(), "", &var, &var);
      }
      if(phase == IntervalPhase)
      {
        symbolic_expression::node_sptr rhs(new Number("0"));
        fmt = phase == PointPhase?"vn":"vt";
        fmt += "en";
        variable_t var(continuity.first, -continuity.second + 1);
        backend->call("addEquation", 2, fmt.c_str(), "", &var, &rhs);
      }
    }
  }
}

CheckConsistencyResult ConsistencyChecker::call_backend_check_consistency(const PhaseType& phase)
{
  CheckConsistencyResult ret;
  if(phase == PointPhase)
  {
    backend->call("checkConsistencyPoint", 0, "", "cc", &ret);
  }
  else
  {
    backend->call("checkConsistencyInterval", 0, "", "cc", &ret);
  }
  return ret;
}


CheckConsistencyResult ConsistencyChecker::check_consistency(const ConstraintStore& constraint_store, const PhaseType& phase)
{
  ContinuityMapMaker maker;
  maker.reset();
  for(auto constraint : constraint_store){
    maker.visit_node(constraint, phase == IntervalPhase, false);
  }
  return check_consistency(constraint_store, maker.get_continuity_map(), phase);
}

CheckConsistencyResult ConsistencyChecker::check_consistency(const ConstraintStore& constraint_store, const continuity_map_t& continuity_map, const PhaseType& phase)
{
  add_continuity(continuity_map, phase);

  const char* fmt = (phase == PointPhase)?"csn":"cst";
  backend->call("addConstraint", 1, fmt, "", &constraint_store);
      
  return call_backend_check_consistency(phase);
}

}
}
