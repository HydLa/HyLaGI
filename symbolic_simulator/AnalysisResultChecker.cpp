#include "ConstraintAnalyzer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"
#include "AnalysisResultChecker.h"

#include "../solver/mathematica/MathematicaSolver.h"

using namespace std;
using namespace hydla::simulator::symbolic;
using namespace hydla::simulator;
using namespace hydla::solver;
using namespace hydla::solver::mathematica;
using namespace hydla::parse_tree;

namespace hydla{
namespace simulator{
namespace symbolic{

AnalysisResultChecker::AnalysisResultChecker(const Opts& opts):ConstraintAnalyzer(const_cast<Opts&>(opts)){}

AnalysisResultChecker::~AnalysisResultChecker(){}

node_sptr AnalysisResultChecker::string2node(std::string s){
  unsigned int index = s.find('<',0);
  std::string tmp = s.substr(0,index);
  if(tmp == "E"){
    return node_sptr(new hydla::parse_tree::E());
  }
  if(tmp == "Pi"){
    return node_sptr(new hydla::parse_tree::Pi());
  }
  index = s.find('[',index+1);
  std::string arg = s.substr(index+1);

  if(tmp == "Equal" ||
     tmp == "UnEqual" ||
     tmp == "Plus" ||
     tmp == "Substract" ||
     tmp == "Times" ||
     tmp == "Divide" ||
     tmp == "Power" ||
     tmp == "LogicalAnd" ||
     tmp == "LogicalOr" ||
     tmp == "Less" ||
     tmp == "LessEqual" ||
     tmp == "Greater" ||
     tmp == "GreaterEqual"){
    int parmit = 0;
    for(unsigned int i = 0, count_l = 0, count_r = 0; i < arg.length() && parmit == 0; i++){
      switch(arg[i]){
      case '[':
	count_l++;
	break;
      case ']':
	count_r++;
	break;
      case ',':
	if(count_l == count_r) parmit = i;
	break;
      default:
	break;
      }
    }
    if(parmit == 0){
      std::cout << "syntax error" << std::endl;
      return node_sptr();
    }
    if(tmp == "Equal"){
      return node_sptr(new hydla::parse_tree::Equal(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "UnEqual"){
      return node_sptr(new hydla::parse_tree::UnEqual(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "Plus"){
      return node_sptr(new hydla::parse_tree::Plus(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "Subtract"){
      return node_sptr(new hydla::parse_tree::Subtract(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "Times"){
      return node_sptr(new hydla::parse_tree::Times(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "Divide"){
      return node_sptr(new hydla::parse_tree::Divide(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "Power"){
      return node_sptr(new hydla::parse_tree::Power(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "LogicalAnd"){
      return node_sptr(new hydla::parse_tree::LogicalAnd(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "LogicalOr"){
      return node_sptr(new hydla::parse_tree::LogicalOr(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "Less"){
      return node_sptr(new hydla::parse_tree::Less(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "LessEqual"){
      return node_sptr(new hydla::parse_tree::LessEqual(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "Greater"){
      return node_sptr(new hydla::parse_tree::Greater(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
    if(tmp == "GreaterEqual"){
      return node_sptr(new hydla::parse_tree::GreaterEqual(string2node(arg.substr(0,parmit)),string2node(arg.substr(parmit+1))));
    }
  }
  if(tmp == "Differential"){
    return node_sptr(new hydla::parse_tree::Differential(string2node(arg)));
  }
  if(tmp == "Previous"){
    return node_sptr(new hydla::parse_tree::Previous(string2node(arg)));
  }
  if(tmp == "Negative"){
    return node_sptr(new hydla::parse_tree::Negative(string2node(arg)));
  }
  if(tmp == "Variable"){
    unsigned int end = arg.find(']',0);
    return node_sptr(new hydla::parse_tree::Variable(arg.substr(0,end)));
  }
  if(tmp == "Number"){
    unsigned int end = arg.find(']',0);
    return node_sptr(new hydla::parse_tree::Number(arg.substr(0,end)));
  }
  return node_sptr();
}

void AnalysisResultChecker::parse(){
  if(opts_->analysis_file == "") return;
  std::string filename(opts_->analysis_file);
  std::ifstream in(filename.c_str());
  if(!in){
    std::cout << "cannot open \"" << opts_->analysis_file << "\"" << std::endl;
    return;
  }
  std::string tmp;
  while(std::getline(in,tmp)){
    unsigned int index = tmp.find(':',0);
    conditions_[tmp.substr(0,index)] = string2node(tmp.substr(index+1));
  }
}

void AnalysisResultChecker::set_solver(boost::shared_ptr<hydla::solver::SymbolicSolver> solver){
  solver_ = solver;
}


module_set_list_t AnalysisResultChecker::calculate_mms(
  simulator::simulation_todo_sptr_t& state,
  const variable_map_t& vm,
  todo_container_t* todo_container)
{
  module_set_list_t ret;
  //  std::cout << "******************************" << std::endl;
  for(hydla::ch::cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
    if((*it)->is_searched()) continue;
    if(check_conditions((*it)->get_condition(), state, vm, todo_container)){
      module_set_list_t ms_list = (*it)->get_module_set_list();
      for(module_set_list_t::iterator ms_it = ms_list.begin(); ms_it != ms_list.end(); ms_it++){
	ret.push_back(*ms_it);
	//	std::cout << (*ms_it)->get_name() << std::endl;
      }
      (*it)->mark_children();
    }
  }
  for(hydla::ch::cm_map_list_t::iterator it = cm_list_.begin(); it != cm_list_.end(); it++){
    (*it)->unsearched();
  }
  //  std::cout << "******************************" << std::endl;
  return ret;
}

bool AnalysisResultChecker::check_conditions(
  const hydla::parse_tree::node_sptr& cond,
  simulation_todo_sptr_t& state,
  const variable_map_t& vm,
  todo_container_t* todo_container)
{
  solver_->reset(vm, state->parameter_map);
  solver_->change_mode(ConditionsMode, opts_->approx_precision);
  solver_->set_conditions(cond);
  // check_conditionとか作った方が良さそうな気がするよ
  CheckConsistencyResult check_consistency_result = solver_->check_consistency();
  if(check_consistency_result.true_parameter_maps.empty()){
    return false;
  }else if(check_consistency_result.false_parameter_maps.empty()){
    return true;
  }else{
    for(int i=1; i<(int)check_consistency_result.true_parameter_maps.size();i++){
      simulation_todo_sptr_t branch_state(new SimulationTodo(*state));
      branch_state->parameter_map = check_consistency_result.true_parameter_maps[i];
      todo_container->push_todo(branch_state);
    }
    for(int i=0; i<(int)check_consistency_result.false_parameter_maps.size();i++){
      simulation_todo_sptr_t branch_state(new SimulationTodo(*state));
      branch_state->parameter_map = check_consistency_result.false_parameter_maps[i];
      todo_container->push_todo(branch_state);
    }
    state->parameter_map = check_consistency_result.true_parameter_maps[0];
    solver_->reset_parameters(state->parameter_map);

    return true;
  }
}

bool 
AnalysisResultChecker::check_conditions(
  const module_set_sptr& ms,
  simulation_todo_sptr_t& state,
  const variable_map_t& vm,
  bool b,
  todo_container_t* todo_container)
{
  /*
  if(opts_->analysis_mode == "simulate"){
    if(checkd_module_set_.find(ms) != checkd_module_set_.end()){
      if(conditions_.find(ms->get_name()) == conditions_.end()){
        return CVM_INCONSISTENT;
      }
    }else{
      checkd_module_set_.insert(ms);
      if(find_conditions(ms,b) == CONDITIONS_TRUE){
        return CVM_INCONSISTENT;
      }
    }
  }
  */
  return check_conditions(conditions_[ms->get_name()],state,vm,todo_container);
}


}
}
}
