#include "SymbolicTrajPrinter.h"
#include <boost/foreach.hpp>
#include <stack>
#include "DefaultParameter.h"

using namespace std;
using namespace hydla::simulator;

namespace hydla{
namespace output{

SymbolicTrajPrinter::SymbolicTrajPrinter(const std::set<std::string> &output_variables):
   output_variables_(output_variables){
}

SymbolicTrajPrinter::SymbolicTrajPrinter(){}

std::string SymbolicTrajPrinter::get_state_output(const phase_result_t& result) const{
  std::stringstream sstr;
    if(result.phase==IntervalPhase){
      sstr << "---------IP " << result.id << "---------" << endl;
      sstr << result.module_set->get_name() << endl;
      sstr << "time\t: " << *result.current_time << "->" << *result.end_time << "\n";
    }else{
      sstr << "---------PP " << result.id << "---------" << endl;
      sstr << result.module_set->get_name() << endl;
      sstr << "time\t: " << *result.current_time << "\n";
    }
    output_variable_map(sstr, result.variable_map);
    return sstr.str();
}

void SymbolicTrajPrinter::output_parameter_map(const parameter_map_t& pm) const
{
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  if(it != end){
    std::cout << "\n#---------parameter condition---------\n";
  }
  for(; it!=end; ++it) {
    std::cout << *(it->first) << "\t: " << it->second << "\n";
  }
}

void SymbolicTrajPrinter::output_variable_map(std::ostream &stream, const variable_map_t& vm) const
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  for(; it!=end; ++it) {
    stream << *(it->first) << "\t: " << *it->second << "\n";
  }
}


void SymbolicTrajPrinter::output_one_phase(const phase_result_const_sptr_t& phase) const
{
    cout << get_state_output(*phase);
}


void SymbolicTrajPrinter::output_result_tree(const phase_result_const_sptr_t& root) const
{
    if(root->children.size() == 0){
      cout << "No Result." << endl;
      return;
    }
    int i=1, j=1;
    phase_result_sptrs_t::const_iterator it = root->children.begin(), end = root->children.end();
    for(;it!=end;it++){
      std::vector<std::string> result;
      output_result_node(*it, result, i, j);
    }
}



void SymbolicTrajPrinter::output_result_node(const phase_result_const_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num) const{

  if(node->children.size() == 0){
  
    cout << "#---------Case " << case_num++ << "---------" << endl;
    std::vector<std::string>::const_iterator r_it = result.begin(), r_end = result.end();
    for(;r_it!=r_end;r_it++){
      cout << *r_it;
    }
    
    if(node->cause_of_termination==simulator::ASSERTION ||
      node->cause_of_termination==simulator::TIME_LIMIT ||
      node->cause_of_termination==simulator::STEP_LIMIT)
        cout << get_state_output(*node);
    switch(node->cause_of_termination){
      case simulator::INCONSISTENCY:
        cout << "# execution stuck\n";
        output_parameter_map(node->parameter_map);
        break;

      case simulator::SOME_ERROR:
        output_parameter_map(node->parameter_map);
        cout << "# some error occurred\n" ;
        break;

      case simulator::ASSERTION:
        output_parameter_map(node->parameter_map);
        cout << "# assertion failed\n" ;
        break;
        
      case simulator::TIME_LIMIT:
        output_parameter_map(node->parameter_map);
        cout << "# time ended\n" ;
        break;
        
      case simulator::STEP_LIMIT:
        output_parameter_map(node->parameter_map);
        cout << "# step ended\n" ;
        break;
        
      case simulator::NOT_UNIQUE_IN_INTERVAL:
        output_parameter_map(node->parameter_map);
        cout << "# some values of variables are not unique in IP\n" ;
        break;

      default:
      case simulator::NONE:
        output_parameter_map(node->parameter_map);
        cout << "# unknown termination occurred\n" ;
        break;
    }
    cout << endl;
  }else{
    if(node->phase==hydla::simulator::PointPhase){
      std::stringstream sstr;
      sstr << "#---------" << phase_num++ << "---------\n";
      result.push_back(sstr.str());
    }
    result.push_back(get_state_output(*node));
    phase_result_sptrs_t::const_iterator it = node->children.begin(), end = node->children.end();
    for(;it!=end;it++){
      output_result_node(*it, result, case_num, phase_num);
    }
    result.pop_back();
    if(node->phase==PointPhase){
      result.pop_back();
      phase_num--;
    }
  }
}
  



} // output
} // hydla