#include "SymbolicTrajPrinter.h"
#include <stack>
#include <iostream>
#include "Parameter.h"
#include "Logger.h"
#include "Backend.h"
#include "EpsilonMode.h"


namespace hydla{
namespace io{

using namespace simulator;
using namespace backend;
using namespace std;

SymbolicTrajPrinter::SymbolicTrajPrinter(set<string> vars, std::ostream& ost):
  ostream(ost), output_variables(vars){
}

string SymbolicTrajPrinter::get_state_output(const phase_result_t& result) const{
  stringstream sstr;
  if(result.phase_type == INTERVAL_PHASE){
    sstr << "---------IP---------" << endl;
    sstr << "unadopted modules: " << result.unadopted_ms.get_name() << endl;
    if(!result.end_time.undefined()){
      sstr << "t\t: " << result.current_time << "->" << result.end_time << "\n";
    }else{
      sstr << "t\t: " << result.current_time << "->" << "???" << "\n";
    }
  }else{
    sstr << "---------PP---------" << endl;
    sstr << "unadopted modules: " << result.unadopted_ms.get_name() << endl;
    sstr << "t\t: " << result.current_time << "\n";
  }

  if(epsilon_mode_flag && backend.get()){
    output_limit_of_time(sstr,backend.get(),result);
  }

  output_variable_map(sstr, result.variable_map);

  if(epsilon_mode_flag && backend.get()){
    output_limits_of_variable_map(sstr,backend.get(),result,result.variable_map);
  }
  return sstr.str();
}

void SymbolicTrajPrinter::output_parameter_map(const parameter_map_t& pm) const
{
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  if(it != end){
    ostream << "\n---------parameter condition---------\n";
  }
  for(; it!=end; ++it) {
    ostream << it->first << "\t: " << it->second << "\n";
  }
}

void SymbolicTrajPrinter::output_variable_map(std::
ostream &stream, const variable_map_t& vm) const
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  for(; it!=end; ++it) {
    stream << it->first << "\t: " << it->second << "\n";
  }
}

void SymbolicTrajPrinter::output_one_phase(const phase_result_const_sptr_t& phase) const
{
  ostream << get_state_output(*phase);
  output_parameter_map(phase->parameter_map);
}

void SymbolicTrajPrinter::output_result_tree(const phase_result_const_sptr_t& root) const
{
  if(root->children.size() == 0){
    ostream << "No Result." << endl;
    return;
  }
  int i=1, j=1;
  phase_result_sptrs_t::const_iterator it = root->children.begin(), end = root->children.end();
  for(;it!=end;it++){
    vector<std::string> result;
    output_result_node(*it, result, i, j);
  }
}

void SymbolicTrajPrinter::output_result_node(const phase_result_const_sptr_t &node, vector<std::string> &result, int &case_num, int &phase_num) const{

  if(node->children.size() == 0){
    ostream << "---------Case " << case_num++ << "---------" << endl;
    vector<std::string>::const_iterator r_it = result.begin();
    for(;r_it != result.end(); r_it++){
      ostream << *r_it;
    }

    if(node->simulation_state==simulator::ASSERTION ||
      node->simulation_state==simulator::TIME_LIMIT ||
      node->simulation_state==simulator::NOT_SIMULATED ||
      node->simulation_state==simulator::NONE ||
      node->simulation_state==simulator::STEP_LIMIT ||
      node->simulation_state==simulator::SOME_ERROR ||
      node->simulation_state==simulator::INTERRUPTED)
    {
      ostream << get_state_output(*node);
    }

    output_parameter_map(node->parameter_map);
    // print why the simulation was terminated
    switch(node->simulation_state){
      case simulator::INCONSISTENCY:
        ostream << "# execution stuck\n";
        break;

      case simulator::SOME_ERROR:
        ostream << "# some error occurred\n" ;
        break;

      case simulator::ASSERTION:
        ostream << "# assertion failed\n" ;
        break;

      case simulator::TIME_LIMIT:
        ostream << "# time reached limit\n" ;
        break;

      case simulator::STEP_LIMIT:
        ostream << "# number of phases reached limit\n" ;
        break;

      case simulator::TIME_OUT_REACHED:
        ostream << "# time out\n" ;
        break;

      case simulator::NOT_UNIQUE_IN_INTERVAL:
        ostream << "# some values of variables are not unique in IP\n" ;
        break;

      case simulator::NOT_SIMULATED:
        ostream << "# following phases were not simulated\n" ;
        break;


      case simulator::INTERRUPTED:
        ostream << "# simulation was interrupted\n" ;
        break;

      default:
      case simulator::NONE:
        ostream << "# unknown error occured\n" ;
        break;
    }
    ostream << endl;
  }else{
    if(node->phase_type == simulator::POINT_PHASE){
      stringstream sstr;
      sstr << "---------" << phase_num++ << "---------\n";
      result.push_back(sstr.str());
    }
    result.push_back(get_state_output(*node));

    phase_result_sptrs_t::const_iterator it = node->children.begin(), end = node->children.end();
    for(;it!=end;it++){
      output_result_node(*it, result, case_num, phase_num);
    }
    result.pop_back();
    if(node->phase_type == POINT_PHASE){
      result.pop_back();
      phase_num--;
    }
  }
}

void SymbolicTrajPrinter::set_epsilon_mode(backend_sptr_t back,bool flag){
  backend = back;
  epsilon_mode_flag = flag;
}

void SymbolicTrajPrinter::output_limit_of_time(std::ostream &stream, Backend* backend, const phase_result_t& result) const
{
  simulator::value_t ret_current_time,ret_end_time;
  symbolic_expression::node_sptr tmp_current_time,tmp_end_time;
  int check_result,check_current_time,check_end_time;

  if(result.phase_type == INTERVAL_PHASE)
  {
    if(!result.end_time.undefined() && !result.current_time.undefined())
    {
      tmp_current_time = result.current_time.get_node();
      tmp_end_time = result.end_time.get_node();
      backend->call("checkEpsilon", 1, "en", "i",&tmp_current_time, &check_current_time);
      backend->call("checkEpsilon", 1, "en", "i",&tmp_end_time, &check_end_time);
      check_result = check_current_time * check_end_time;
      if(check_result == 1)
      {
        backend->call("limitEpsilon", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        backend->call("limitEpsilon", 1, "en", "vl",&tmp_end_time, &ret_end_time);
        stream << "Limit(t)\t: " << ret_current_time << "->" << ret_end_time << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        backend->call("limitEpsilonP", 1, "en", "vl",&tmp_end_time, &ret_end_time);
        stream << "Limit(t)\t+ : " << ret_current_time << "->" << ret_end_time << "\n";
        backend->call("limitEpsilonM", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        backend->call("limitEpsilonM", 1, "en", "vl",&tmp_end_time, &ret_end_time);
        stream << "Limit(t)\t- : " << ret_current_time << "->" << ret_end_time << "\n";
      }
    }
    else if(!result.current_time.undefined())
    {
      tmp_current_time = result.current_time.get_node();
      backend->call("checkEpsilon", 1, "en", "i",&tmp_current_time, &check_result);
      if(check_result == 1)
      {
        backend->call("limitEpsilon", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t: " << ret_current_time << "->" << "???" << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t+ : " << ret_current_time << "->" << "???" << "\n";
        backend->call("limitEpsilonM", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(time)\t- : " << ret_current_time << "->" << "???" << "\n";
      }
    }
  }
  else
  {
    if(!result.current_time.undefined())
    {
      tmp_current_time = result.current_time.get_node();
      backend->call("checkEpsilon", 1, "en", "i",&tmp_current_time, &check_result);
      if(check_result == 1)
      {
        backend->call("limitEpsilon", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t: " << ret_current_time << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t+ : " << ret_current_time << "\n";
        backend->call("limitEpsilonM", 1, "en", "vl",&tmp_current_time, &ret_current_time);
        stream << "Limit(t)\t- : " << ret_current_time << "\n";
      }
    }
  }
}

void SymbolicTrajPrinter::output_limits_of_variable_map(std::ostream &stream, Backend* backend, const phase_result_t& result, const variable_map_t& vm) const
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  simulator::value_t ret;
  simulator::value_t tmp;
  int check_result;
  it  = vm.begin();
  for(; it!=end; ++it)
  {
    if(it->second.unique())
    {
      tmp = it->second.get_unique_value();
      backend->call("checkEpsilon", 1, "vln", "i",&tmp, &check_result);
      if(check_result == 1)
      {
        backend->call("limitEpsilon", 1, "vln", "vl", &tmp, &ret);
        stream << "Limit(" << it->first << ")\t  : " << ret << "\n";
      }
      else
      {
        backend->call("limitEpsilonP", 1, "vln", "vl", &tmp, &ret);
        stream << "Limit(" << it->first << ")\t+ : " << ret << "\n";
        backend->call("limitEpsilonM", 1, "vln", "vl", &tmp, &ret);
        stream << "Limit(" << it->first << ")\t- : " << ret << "\n";
      }
    }
    else
    {
      stream << "Limit(" << it->first << ")\t: " << it->second << "\n";
    }
  }
}

} // output
} // hydla
