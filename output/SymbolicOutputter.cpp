#include "SymbolicOutputter.h"
#include <boost/foreach.hpp>
#include <stack>
#include "DefaultParameter.h"

using namespace std;
using namespace hydla::simulator;

namespace hydla{
namespace output{

SymbolicOutputter::SymbolicOutputter(const std::set<std::string> &output_variables):
   output_variables_(output_variables){
}

void SymbolicOutputter::output_variable_labels(std::ostream &stream, const variable_map_t variable_map) const{
  // 変数のラベルの出力
  // TODO: 未定義の値とかのせいでずれる可能性あり?
  stream << "# time\t";

  BOOST_FOREACH(const variable_map_t::value_type& i, variable_map) {
    if(output_variables_.find(i.first->get_string()) != output_variables_.end()){
      stream << i.first << "\t";
    }
  }
  stream << std::endl;
}

  
std::string SymbolicOutputter::get_state_output(const phase_result_t& result, const bool& numeric, const bool& is_in_progress) const{
  std::stringstream sstr;
    if(!numeric){
      if(result.phase==IntervalPhase){
        sstr << "---------IP---------" << std::endl;
        sstr << "time\t: " << *result.current_time << "->" << *result.end_time << "\n";
      }else{
        if(is_in_progress)
          sstr << "#-------" << result.step + 1 << "-------" << std::endl;
        sstr << "---------PP---------" << std::endl;
        sstr << "time\t: " << *result.current_time << "\n";
      }
      output_variable_map(sstr, result.variable_map, result.current_time, false);
      sstr << "\n" ;
    }
    return sstr.str();
}
  
  

void SymbolicOutputter::output_parameter_map(const parameter_map_t& pm) const
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

void SymbolicOutputter::output_variable_map(std::ostream &stream, const variable_map_t& vm, const value_t& time, const bool& numeric) const
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  for(; it!=end; ++it) {
    stream << *(it->first) << "\t: " << *it->second << "\n";
  }
}


void SymbolicOutputter::output_result_tree(const phase_result_const_sptr_t& root) const
{
    if(root->children.size() == 0){
      std::cout << "No Result." << std::endl;
      return;
    }
    int i=1, j=1;
    phase_result_sptrs_t::const_iterator it = root->children.begin(), end = root->children.end();
    for(;it!=end;it++){
      std::vector<std::string> result;
      output_result_node(*it, result, i, j);
    }
}



void SymbolicOutputter::output_result_node(const phase_result_const_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num) const{

  if(node->children.size() == 0){
  
    std::cout << "#---------Case " << case_num++ << "---------" << std::endl;
    std::vector<std::string>::const_iterator r_it = result.begin(), r_end = result.end();
    for(;r_it!=r_end;r_it++){
      std::cout << *r_it;
    }
    
    if(node->cause_of_termination!=simulator::INCONSISTENCY)
        std::cout << get_state_output(*node, false,false);
    switch(node->cause_of_termination){
      case simulator::INCONSISTENCY:
        std::cout << "# execution stuck\n";
        output_parameter_map(node->parameter_map);
        break;

      case simulator::SOME_ERROR:
        output_parameter_map(node->parameter_map);
        std::cout << "# some error occurred\n" ;
        break;

      case simulator::ASSERTION:
        output_parameter_map(node->parameter_map);
        std::cout << "# assertion failed\n" ;
        break;
        
      case simulator::TIME_LIMIT:
        output_parameter_map(node->parameter_map);
        std::cout << "# time ended\n" ;
        break;
        
      case simulator::NOT_UNIQUE_IN_INTERVAL:
        output_parameter_map(node->parameter_map);
        std::cout << "# some values of variables are not unique in this phase\n" ;
        break;

      default:
      case simulator::NONE:
        output_parameter_map(node->parameter_map);
        std::cout << "# unknown termination occurred\n" ;
        break;
    }
    std::cout << std::endl;
  }else{
    if(node->phase==hydla::simulator::PointPhase){
      std::stringstream sstr;
      sstr << "#---------" << phase_num++ << "---------\n";
      result.push_back(sstr.str());
    }
    result.push_back(get_state_output(*node, false,false));
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