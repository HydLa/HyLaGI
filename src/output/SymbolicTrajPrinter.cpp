#include "SymbolicTrajPrinter.h"
#include <boost/foreach.hpp>
#include <stack>
#include "Parameter.h"
#include "Logger.h"
#include "Backend.h"

using namespace hydla::simulator;
using namespace std;

using namespace hydla::backend;

namespace hydla{
// namespace backend{
namespace output{

SymbolicTrajPrinter::SymbolicTrajPrinter(const std::set<std::string> &output_variables, std::ostream& ostream):
   ostream_(ostream), output_variables_(output_variables){
}


SymbolicTrajPrinter::SymbolicTrajPrinter(const std::set<std::string> &output_variables):
   ostream_(cout), output_variables_(output_variables){
}

SymbolicTrajPrinter::SymbolicTrajPrinter():ostream_(cout){}

std::string SymbolicTrajPrinter::get_state_output(const phase_result_t& result) const{
  std::stringstream sstr;
  if(result.phase_type == IntervalPhase){
    sstr << "---------IP " << result.id << "---------" << endl;
    sstr << result.module_set->get_name() << endl;
    if(!result.end_time.undefined()){
      sstr << "time\t: " << result.current_time << "->" << result.end_time << "\n";
      //aho
      if(opts_->epsilon_mode){
        simulator::value_t ret1,ret2;
        parse_tree::node_sptr tmp1,tmp2;
        int limitflag,tmps,tmpe;
        tmp1 = result.current_time.get_node();
        tmp2 = result.end_time.get_node();
        backend_->call("checkEpsilon", 1, "en", "i",&tmp1, &tmps);
        backend_->call("checkEpsilon", 1, "en", "i",&tmp2, &tmpe);
        limitflag = tmps * tmpe;
        if(limitflag == 1){
          backend_->call("limitEpsilon", 1, "en", "vl",&tmp1, &ret1);
          backend_->call("limitEpsilon", 1, "en", "vl",&tmp2, &ret2);
          sstr << "Limit(time)\t: " << ret1 << "->" << ret2 << "\n";
        }else{
          backend_->call("limitEpsilonP", 1, "en", "vl",&tmp1, &ret1);
          backend_->call("limitEpsilonP", 1, "en", "vl",&tmp2, &ret2);
          sstr << "Limit(time)\t+ : " << ret1 << "->" << ret2 << "\n";
          backend_->call("limitEpsilonM", 1, "en", "vl",&tmp1, &ret1);
          backend_->call("limitEpsilonM", 1, "en", "vl",&tmp2, &ret2);
          sstr << "Limit(time)\t- : " << ret1 << "->" << ret2 << "\n";
        }
      }
      //aho
    }else{
      sstr << "time\t: " << result.current_time << "->" << "???" << "\n";
      //aho
      if(opts_->epsilon_mode){
        simulator::value_t ret1,ret2;
        parse_tree::node_sptr tmp1,tmp2;
        int limitflag,tmps,tmpe;
        tmp1 = result.current_time.get_node();
        backend_->call("checkEpsilon", 1, "en", "i",&tmp1, &limitflag);
        if(limitflag == 1){
          backend_->call("limitEpsilon", 1, "en", "vl",&tmp1, &ret1);
          sstr << "Limit(time)\t: " << ret1 << "->" << "???" << "\n";
        }else{
          backend_->call("limitEpsilonP", 1, "en", "vl",&tmp1, &ret1);
          sstr << "Limit(time)\t+ : " << ret1 << "->" << "???" << "\n";
          backend_->call("limitEpsilonM", 1, "en", "vl",&tmp1, &ret1);
          sstr << "Limit(time)\t- : " << ret1 << "->" << "???" << "\n";
        }
      }
      //aho
    }
  }else{
    sstr << "---------PP " << result.id << "---------" << endl;
    sstr << result.module_set->get_name() << endl;
    sstr << "time\t: " << result.current_time << "\n";
    //aho
    if(opts_->epsilon_mode){
      simulator::value_t ret1,ret2;
      parse_tree::node_sptr tmp1,tmp2;
      int limitflag,tmps,tmpe;
      tmp1 = result.current_time.get_node();
      backend_->call("checkEpsilon", 1, "en", "i",&tmp1, &limitflag);
      if(limitflag == 1){
        backend_->call("limitEpsilon", 1, "en", "vl",&tmp1, &ret1);
        sstr << "Limit(time)\t: " << ret1 << "\n";
      }else{
        backend_->call("limitEpsilonP", 1, "en", "vl",&tmp1, &ret1);
        sstr << "Limit(time)\t+ : " << ret1 << "\n";
        backend_->call("limitEpsilonM", 1, "en", "vl",&tmp1, &ret1);
        sstr << "Limit(time)\t- : " << ret1 << "\n";
      }
    }
      //aho
  }
  output_variable_map(sstr, result.variable_map);

  return sstr.str();
}

void SymbolicTrajPrinter::output_parameter_map(const parameter_map_t& pm) const
{
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  if(it != end){
    ostream_ << "\n#---------parameter condition---------\n";
  }
  for(; it!=end; ++it) {
    ostream_ << it->first << "\t: " << it->second << "\n";
  }
}


void SymbolicTrajPrinter::output_variable_map(std::ostream &stream, const variable_map_t& vm) const
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  for(; it!=end; ++it) {
    stream << it->first << "\t: " << it->second << "\n";
  }
  //aho
  if(opts_->epsilon_mode){
    simulator::value_t ret;
    simulator::value_t tmp;
    int limitflag;
    it  = vm.begin();
    for(; it!=end; ++it) {
      if(it->second.unique()){
        tmp = it->second.get_unique();
        backend_->call("checkEpsilon", 1, "vln", "i",&tmp, &limitflag);
        if(limitflag == 1){
          backend_->call("limitEpsilon", 1, "vln", "vl", &tmp, &ret);
          stream << "Limit(" << it->first << ")\t  : " << ret << "\n";
        }else{
          backend_->call("limitEpsilonP", 1, "vln", "vl", &tmp, &ret);
          stream << "Limit(" << it->first << ")\t+ : " << ret << "\n";
          backend_->call("limitEpsilonM", 1, "vln", "vl", &tmp, &ret);
          stream << "Limit(" << it->first << ")\t- : " << ret << "\n";
        }
      }else {
        stream << "Limit(" << it->first << ")\t: " << it->second << "\n";
      }
    }
  }
  //aho

}

void SymbolicTrajPrinter::output_one_phase(const phase_result_const_sptr_t& phase) const
{
  ostream_ << get_state_output(*phase);
  output_parameter_map(phase->parameter_map);
}

void SymbolicTrajPrinter::output_result_tree(const phase_result_const_sptr_t& root) const
{
  if(root->children.size() == 0){
    ostream_ << "No Result." << endl;
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
    ostream_ << "#---------Case " << case_num++ << "---------" << endl;
    std::vector<std::string>::const_iterator r_it = result.begin();
    for(;r_it != result.end(); r_it++){
      ostream_ << *r_it;
    }

    if(node->cause_for_termination==simulator::ASSERTION ||
      node->cause_for_termination==simulator::OTHER_ASSERTION ||
      node->cause_for_termination==simulator::TIME_LIMIT ||
      node->cause_for_termination==simulator::NOT_SELECTED ||
      node->cause_for_termination==simulator::NONE ||
      node->cause_for_termination==simulator::STEP_LIMIT)
    {
      ostream_ << get_state_output(*node);
    }

    output_parameter_map(node->parameter_map);
    switch(node->cause_for_termination){
      case simulator::INCONSISTENCY:
        ostream_ << "# execution stuck\n";
        break;

      case simulator::SOME_ERROR:
        ostream_ << "# some error occurred\n" ;
        break;

      case simulator::ASSERTION:
        ostream_ << "# assertion failed\n" ;
        break;

      case simulator::OTHER_ASSERTION:
        ostream_ << "# terminated by failure of assertion in another case\n" ;
        break;

      case simulator::TIME_LIMIT:
        ostream_ << "# time reached limit\n" ;
        break;

      case simulator::STEP_LIMIT:
        ostream_ << "# number of phases reached limit\n" ;
        break;

      case simulator::TIME_OUT_REACHED:
        ostream_ << "# time out\n" ;
        break;

      case simulator::NOT_UNIQUE_IN_INTERVAL:
        ostream_ << "# some values of variables are not unique in IP\n" ;
        break;

      case simulator::NOT_SELECTED:
        ostream_ << "# this case is not selected to be simulated\n" ;
        break;

      default:
      case simulator::NONE:
        ostream_ << "# unknown termination occurred\n" ;
        break;
    }
    ostream_ << endl;
  }else{
    if(node->phase_type == hydla::simulator::PointPhase){
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
    if(node->phase_type == PointPhase){
      result.pop_back();
      phase_num--;
    }
  }
}

void SymbolicTrajPrinter::set_epsilon_mode(backend_sptr_t back,Opts *op){
  backend_ = back;
  opts_ = op;
}

} // output
// } // backend
} // hydla
