#include "ParallelSimulator.h"
#include "ParallelSimulatorWorker.h"
#include "Timer.h"
#include "SymbolicSimulator.h"
#include "SymbolicTrajPrinter.h"
#include "SymbolicValue.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

namespace hydla {
namespace simulator {


using namespace std;

ParallelSimulator::ParallelSimulator(Opts &opts):Simulator(opts){
}

ParallelSimulator::~ParallelSimulator(){}

void ParallelSimulator::push_phase(const simulation_phase_sptr_t& state){
  Simulator::push_simulation_phase(state);
  cout<<"pushed. stack size:"<<state_stack_.size()<<endl;
}

simulation_phase_sptr_t ParallelSimulator::pop_phase(){
  simulation_phase_sptr_t spp = Simulator::pop_simulation_phase();
  cout<<"popped. stack size:"<<state_stack_.size()<<endl;
  return spp;
}

/**
 * スレッドを立ち上げ、シミュレーションを開始する
 */
phase_result_const_sptr_t ParallelSimulator::simulate()
{
  for(int i=0; i<opts_->parallel_number; i++){
    thread_group_.create_thread(boost::bind(&ParallelSimulatorWorker::simulate,*workers_[i]));
  }
  thread_group_.join_all();
  return result_root_;
}

phase_result_const_sptr_t ParallelSimulator::get_result_root()
{
  return result_root_;
}

void ParallelSimulator::initialize(const parse_tree_sptr& parse_tree){
  Simulator::initialize(parse_tree);
  for(int i=0;i<opts_->parallel_number; i++){
    boost::shared_ptr<ParallelSimulatorWorker> psw(new ParallelSimulatorWorker(*opts_));
    psw->ParallelSimulatorWorker::initialize(parse_tree,i,this);
    psw->set_result_root(result_root_);
    workers_.push_back(psw);
  }
  //初期状態を作ってスタックに入れる
  simulation_phase_sptr_t state(new simulation_phase_t());
  state->elapsed_time = 0;
  phase_result_sptr_t &pr = state->phase_result;
  pr.reset(new phase_result_t());
  pr->cause_of_termination = NONE;
    
  pr->phase        = simulator::PointPhase;
  pr->step         = 0;
  pr->current_time = value_t(new hydla::symbolic_simulator::SymbolicValue("0"));
//  state->module_set_container = msc_original_;
  pr->parent = result_root_;
  push_simulation_phase(state);
}

bool ParallelSimulator::state_stack_is_empty(){
  return state_stack_.empty();
}

} // simulator
} // hydla
