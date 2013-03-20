#include "ConstraintAnalyzer.h"
#include "TellCollector.h"
#include "AskCollector.h"
#include "ContinuityMapMaker.h"
#include "AnalysisResultChecker.h"

#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"

using namespace std;
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;
using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;


namespace hydla{
namespace symbolic_simulator{

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
    false_conditions_[tmp.substr(0,index)] = string2node(tmp.substr(index+1));
  }
  /*
  false_map_t::iterator it = false_conditions_.begin();
  for(;it != false_conditions_.end();it++){
    std::cout << (*it).first << "  ///  ";
    if((*it).second != NULL){
      std::cout << TreeInfixPrinter().get_infix_string((*it).second);
    }
    std::cout << std::endl;
  }
  */
}

void AnalysisResultChecker::set_solver(boost::shared_ptr<hydla::vcs::SymbolicVirtualConstraintSolver> solver){
  solver_ = solver;
}

SymbolicSimulator::CalculateVariableMapResult
AnalysisResultChecker::check_false_conditions(
  const module_set_sptr& ms,
  simulation_phase_sptr_t& state,
  const variable_map_t& vm,
  variable_map_t& result_vm,
  todo_and_results_t& result_todo)
{
  phase_result_sptr_t& pr = state->phase_result;
  if(opts_->analysis_mode == "simulate"){
    if(checkd_module_set_.find(ms) != checkd_module_set_.end()){
      if(false_conditions_.find(ms->get_name()) == false_conditions_.end()){
        return SymbolicSimulator::CVM_INCONSISTENT;
      }
    }else{
      checkd_module_set_.insert(ms);
      if(find_false_conditions(ms) == FALSE_CONDITIONS_TRUE){
        return SymbolicSimulator::CVM_INCONSISTENT;
      }
    }
  }
  solver_->reset(vm, pr->parameter_map);
  if(false_conditions_[ms->get_name()] != NULL){
    solver_->change_mode(FalseConditionsMode, opts_->approx_precision);
    solver_->set_false_conditions(false_conditions_[ms->get_name()]);
    SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
    if(check_consistency_result.true_parameter_maps.empty()){
      return SymbolicSimulator::CVM_INCONSISTENT;
    }else if(check_consistency_result.false_parameter_maps.empty()){
    }else{
      CalculateClosureResult result;
      for(int i=0; i<(int)check_consistency_result.true_parameter_maps.size();i++){
        simulation_phase_sptr_t branch_state(new simulation_phase_t(*state));
        branch_state->phase_result.reset(new phase_result_t(*state->phase_result));
        branch_state->phase_result->cause_of_termination = NONE;
        branch_state->phase_result->parameter_map = check_consistency_result.true_parameter_maps[i];
        result.push_back(branch_state);
      }
      for(int i=0; i<(int)check_consistency_result.false_parameter_maps.size();i++){
        simulation_phase_sptr_t branch_state(new simulation_phase_t(*state));
        branch_state->phase_result.reset(new phase_result_t(*state->phase_result));
        branch_state->phase_result->cause_of_termination = NONE;
        branch_state->phase_result->parameter_map = check_consistency_result.false_parameter_maps[i];
        result.push_back(branch_state);
      }

      for(unsigned int i = 0; i < result.size(); i++){
        result_todo.push_back(PhaseSimulator::TodoAndResult(result[i], phase_result_sptr_t()));
      }
      return SymbolicSimulator::CVM_BRANCH;
    }
  }
  return SymbolicSimulator::CVM_CONSISTENT;
}


}
}
