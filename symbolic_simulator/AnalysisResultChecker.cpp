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
