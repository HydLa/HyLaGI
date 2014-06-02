/*
#include "TimePrinter.h"
#include <boost/foreach.hpp>
#include <stack>
#include "DefaultParameter.h"

using namespace std;
using namespace hydla::simulator;

namespace hydla{
namespace io{

void Printer::output_variable_labels(std::ostream &stream, const variable_map_t variable_map) const{
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

  
std::string Printer::get_state_output(const phase_result_t& result, const bool& numeric, const bool& is_in_progress) const{
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
    }else{
      if(result.phase==IntervalPhase){
        sstr << "#---------IP---------" << std::endl;
        output_variable_labels(sstr, result.variable_map);
        variable_map_t output_vm;
        
        value_t* elapsed_time("0");
        value_t* limit_time = result.end_time-result.current_time;
        
        
        //TODO:できればPrinterからソルバは見たくないが，そうしないと数値に変換できないのでどうしましょう
        //solver_->simplify(limit_time);
        
        do{
          //solver_->apply_time_to_vm(result.variable_map, output_vm, elapsed_time);
          output_vm = result.variable_map;
          output_variable_map(sstr, output_vm, (elapsed_time+result.parent->current_time), true);
          elapsed_time += time_value_t(opts_->output_interval);
          //solver_->simplify(elapsed_time);
        }while(solver_->less_than(elapsed_time, limit_time));
        
        //solver_->apply_time_to_vm(result.variable_map, output_vm, limit_time);
        output_vm = result.variable_map;
        output_variable_map(sstr, output_vm, result.current_time, true);
        sstr << std::endl;
      }else{
        sstr << "#---------PP---------" << std::endl;
        output_variable_labels(sstr, result.variable_map);
        output_variable_map(sstr, result.variable_map, result.current_time, true);
      }
      sstr << std::endl;
    }
    return sstr.str();
}
  
  

void Printer::output_parameter_map(const parameter_map_t& pm) const
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

void Printer::output_variable_map(std::ostream &stream, const variable_map_t& vm, const value_t& time, const bool& numeric) const
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  if(numeric){
  
    stream << std::endl;
    stream << solver_->get_real_val(time, opts_->output_precision, opts_->output_format) << "\t";
    for(; it!=end; ++it) {
      if(opts_->output_variables.find(it->first->get_string()) != opts_->output_variables.end())
        stream << solver_->get_real_val(it->second, opts_->output_precision, opts_->output_format) << "\t";
    }
    
  }else{
    for(; it!=end; ++it) {
      stream << *(it->first) << "\t: " << *it->second << "\n";
    }
  }
}


void Printer::output_result_tree(const phase_result_const_sptr_t& root) const
{
    if(root->children.size() == 0){
      std::cout << "No Result." << std::endl;
      return;
    }
    int i=1, j=1;
    phase_result_sptrs_t::const_iterator it = root->children.begin(), end = root->children.end();
    std::cout << "#------Simulation Time------\n";
    if(time_measurement_ == tFmtCsv){
      std::stack<std::pair<phase_result_sptr_t,int> > tmp;
      phase_result_sptrs_t::const_iterator check_it = it;
      for(;check_it!=end;check_it++) tmp.push(std::pair<phase_result_sptr_t,int>(*it,1));
      while(!tmp.empty()){
        std::pair<phase_result_sptr_t,int> check_node = tmp.top();
        tmp.pop();
        if(check_node.first->children.size() == 0){
          if(j < check_node.second) j = check_node.second;
        }else{
          phase_result_sptrs_t::iterator tmp_it = check_node.first->children.begin(), tmp_end = check_node.first->children.end();
          int plus = 0;
          if(check_node.first->phase == PointPhase) plus = 1;
          for(;tmp_it!=tmp_end;tmp_it++) tmp.push(std::pair<phase_result_sptr_t,int>(*tmp_it,check_node.second+plus));
        }
      }
    }else{
      for(;it!=end;it++){
        std::vector<std::string> result;
        output_result_node(*it, result, i, j);
        // output_result_node_time(*it, result, i, j);
      }
    }
}



void Printer::output_result_node_time(const phase_result_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num) const{

  std::stringstream sstr;

  if(node->children.size() == 0){
  
    switch(time_measurement_){
    case tFmtStd:
      std::cout << "#---------Case " << case_num++ << "---------" << std::endl;
      break;
    case tFmtCsv:
      if(case_num == 1){
        std::cout << "Simulation Time,";
        for(int i = 1; i < phase_num; i++){
          std::cout << "PP" << i << "-Calculate Closure Time,";
          std::cout << "PP" << i << "-Phase Time,";
          std::cout << "IP" << i << "-Calculate Closure Time,";
          std::cout << "IP" << i << "-Phase Time,";
        }
        std::cout << "\n";
      }
      std::cout << "Case " << case_num++ << ",";
      break;
    default:
      break;
    }

    std::vector<std::string>::const_iterator r_it = result.begin(), r_end = result.end();
    for(;r_it!=r_end;r_it++){
      std::cout << *r_it;
    }

    switch(time_measurement_){
    case tFmtStd:
      sstr << "---------IP---------\n";
      sstr << "Calculate Closure Time : " << node->calculate_closure_timer.get_time_string() << " s\n";
      sstr << "Phase Time             : " << node->phase_timer.get_time_string() << " s\n";
      break;
    case tFmtCsv:
      sstr << node->calculate_closure_timer.get_time_string() << ",";
      sstr << node->phase_timer.get_time_string();
      break;
    default:
      break;
    }

    sstr << std::endl;
    std::cout << sstr.str();
    
  }else{
    switch(time_measurement_){
    case tFmtStd:
      if(node->phase==PointPhase) {
        sstr << "#---------" << phase_num++ << "---------\n";
        sstr << "---------PP---------\n";
      } else {
        sstr << "---------IP---------\n";
      }
      sstr << "Calculate Closure Time : " << node->calculate_closure_timer.get_time_string() << " s\n";
      sstr << "Phase Time             : " << node->phase_timer.get_time_string() << " s\n\n";
      break;
    case tFmtCsv:
      sstr << node->calculate_closure_timer.get_time_string() << ",";
      sstr << node->phase_timer.get_time_string() << ",";
      break;
    default:
      break;
    }
    result.push_back(sstr.str());
    phase_result_sptrs_t::const_iterator it = node->children.begin(), end = node->children.end();
    for(;it!=end;it++){
      output_result_node_time(*it, result, case_num, phase_num);
    }
    result.pop_back();
    if(node->phase==PointPhase){
      phase_num--;
    }
  }
}

void Printer::output_result_node(const phase_result_const_sptr_t &node, std::vector<std::string> &result, int &case_num, int &phase_num) const{

  if(node->children.size() == 0){
  
    std::cout << "#---------Case " << case_num++ << "---------" << std::endl;
    std::vector<std::string>::const_iterator r_it = result.begin(), r_end = result.end();
    for(;r_it!=r_end;r_it++){
      std::cout << *r_it;
    }
    
    if(node->cause_for_termination!=simulator::INCONSISTENCY)
        std::cout << get_state_output(*node, false,false);
    switch(node->cause_for_termination){
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

*/