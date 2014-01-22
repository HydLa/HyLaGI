#include "MathematicaTrajPrinter.h"

#include <stack>

using namespace std;
using namespace hydla::simulator;

namespace hydla{
namespace output{
/*
void MathematicaPrinter::output_result_tree(const phase_result_const_sptr_t& result_root) const{
  if(result_root->children.size() == 0){
    std::cout << "No Result." << std::endl;
    return;
  }
  std::stack< std::pair<phase_result_sptr_t, std::string> > node_stack;
  
  for(phase_result_sptrs_t::const_iterator it = result_root->children.begin(); it != result_root->children.end(); it++){
    node_stack.push(std::pair<phase_result_sptr_t, std::string>(*it, ""));
  }
  
  std::cout << "Show[";
  while(!node_stack.empty()){
    std::pair<phase_result_sptr_t, std::string> now_pair = node_stack.top();
    node_stack.pop();
    phase_result_sptr_t now_node = now_pair.first;
    std::stringstream out_tmp;

    out_tmp << now_pair.second;

    std::string prev_node_time;

    if(now_node->phase==IntervalPhase){
      variable_map_t vm = now_node->variable_map;
      if(out_tmp.str().empty()){
        prev_node_time = "0";
        out_tmp << "Table[";
        out_tmp << "{";
      }else{
        phase_result_sptr_t tmp = now_node->parent;
        while(tmp->phase!=IntervalPhase) tmp = tmp->parent;
        prev_node_time = tmp->end_time.get_string();
        out_tmp << ",";
      }
      typename variable_map_t::const_iterator it = vm.begin();
      while(opts_->output_variables.find(it->first->get_string()) == opts_->output_variables.end()){
        it++;
        if(it == vm.end()) return;
      }
      out_tmp << "Plot[";
      out_tmp << it->second;
      out_tmp << ", {t, ";
      out_tmp << prev_node_time;
      out_tmp << ", ";
      out_tmp << now_node->end_time;
      out_tmp << "} ";
      //        prev_node_time = now_node->end_time.get_string();
      if(now_node->cause_of_termination == simulator::ASSERTION)
        out_tmp << ", PlotStyle -> Dashed";
      out_tmp << "]"; //Plot
    }
    if(now_node->children.size() == 0){//葉に到達
      out_tmp << "}, "; //式のリストここまで
      if(now_node->parameter_map.size() > 0){
        // 定数の条件
        out_tmp << "{";
        typename parameter_map_t::const_iterator it = now_node->parameter_map.begin();
        while(it != now_node->parameter_map.end()){
          if(it->second.undefined()){
            it++;
          }else{
            out_tmp << *it->first << ", " << it->second.get_lower_bound().value.get_string() << ", " << it->second.get_upper_bound().value.get_string() << ", step}";
            break;
          }
        }
        if(it == now_node->parameter_map.end()){
          out_tmp << "{1}";
        }
      }else{
        out_tmp << "{1}";
      }
      out_tmp << "], "; //Table
      out_tmp << std::endl;
      std::cout << out_tmp.str();
    }
    else{
      for(typename phase_result_sptrs_t::const_iterator it = now_node->children.begin(); it != now_node->children.end(); it++)
        node_stack.push(std::pair<phase_result_sptr_t, std::string>(*it, out_tmp.str()));
    }
  }
  std::cout << "PlotRange -> {{0, " << opts_->max_time << "}, {lb, ub}}";
  std::cout << "]" << std::endl; //Show
};
*/

} // output
} // hydla