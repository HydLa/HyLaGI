#include "ParallelSimulator.h"
#include "ParallelSimulatorWorker.h"
#include "Timer.h"
#include "SymbolicPhaseSimulator.h"
#include "SymbolicTrajPrinter.h"
#include "PhaseSimulator.h"
#include "../common/TimeOutError.h"

namespace hydla {
namespace simulator {


using namespace std;

ParallelSimulator::ParallelSimulator(Opts &opts):BatchSimulator(opts)
{

  for(int i = 0;i < opts_->parallel_number; i++)
  {
    boost::shared_ptr<ParallelSimulatorWorker> psw(new ParallelSimulatorWorker(*opts_, this, i));
    workers_.push_back(psw);
  }
}

ParallelSimulator::~ParallelSimulator(){}


phase_result_const_sptr_t ParallelSimulator::simulate()
{
  
  for(unsigned int i = 0; i < workers_.size(); i++){
    thread_group_.create_thread(boost::bind(&ParallelSimulatorWorker::simulate,*workers_[i]));
  }
  thread_group_.join_all();
  
  return result_root_;
}

void ParallelSimulator::initialize(const parse_tree_sptr& parse_tree)
{
  reset_result_root();

  opts_->assertion = parse_tree->get_assertion_node();

  parse_tree_ = parse_tree;
  init_variable_map(parse_tree);
  
  profile_vector_.reset(new entire_profile_t());
  
  todo_stack_.reset(new ParallelTodoContainer(opts_->search_method, profile_vector_, &mutex_));

  for(unsigned int i = 0; i < workers_.size(); i++)
  {
    workers_[i]->initialize(parse_tree);
  }
  simulation_todo_sptr_t todo = workers_[0]->make_initial_todo();
  todo_stack_->push_todo(todo);
}


} // simulator
} // hydla
