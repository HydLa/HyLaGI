#include "SymbolicSimulator.h"

#include <iostream>
#include <fstream>
#include <stack>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"
#include "Timer.h"

//���ǉ�
#include "RTreeVisitor.h"
#include "TellCollector.h"
#include "AskCollector.h"

#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
//#include "../virtual_constraint_solver/reduce/REDUCEVCS.h"
#include "ContinuityMapMaker.h"

#include "PrevSearcher.h"

#include "SimulateError.h"

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;

//using namespace hydla::vcs::reduce;

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::symbolic_simulator;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;
using namespace hydla::timer;

using hydla::simulator::TellCollector;
using hydla::simulator::AskCollector;
using hydla::simulator::ContinuityMapMaker;
using hydla::simulator::IntervalPhase;
using hydla::simulator::PointPhase;

namespace hydla {
namespace symbolic_simulator {

SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
  simulator_t(opts)
{
}

SymbolicSimulator::~SymbolicSimulator()
{
}

void SymbolicSimulator::initialize(variable_set_t &v, parameter_set_t &p, variable_map_t &m, continuity_map_t& c)
{
  simulator_t::initialize(v, p, m, c);
  variable_derivative_map_ = c;

  // �g�p����\���o�̌���
  /*  if(opts_->solver == "r" || opts_->solver == "Reduce") {
      solver_.reset(new REDUCEVCS(opts_, variable_map_));
      }else{*/
  solver_.reset(new MathematicaVCS(*opts_));
  //}
  solver_->set_variable_set(*variable_set_);
  solver_->set_parameter_set(*parameter_set_);
}

void SymbolicSimulator::set_parameter_set(parameter_t param){
  parameter_set_->push_front(param);
  //std::cout << "ss sps size : " << parameter_set_->size() << std::endl;
  solver_->set_parameter_set(*parameter_set_);
}
parameter_set_t SymbolicSimulator::get_parameter_set(){
  return *parameter_set_;
}


void SymbolicSimulator::push_branch_states(simulation_phase_sptr_t &original, SymbolicVirtualConstraintSolver::check_consistency_result_t &result, CalculateClosureResult &dst){
  for(int i=0; i<(int)result.true_parameter_maps.size();i++){
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->phase_result->parameter_map = result.true_parameter_maps[i];
    dst.push_back(branch_state);
  }
  for(int i=0; i<(int)result.false_parameter_maps.size();i++){
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(original));
    branch_state->phase_result->parameter_map = result.false_parameter_maps[i];
    dst.push_back(branch_state);
  }
}


void SymbolicSimulator::add_continuity(const continuity_map_t& continuity_map){
  for(continuity_map_t::const_iterator it = continuity_map.begin(); it != continuity_map.end();it++){
    if(it->second>=0){
      for(int i=0; i<it->second;i++){
        solver_->set_continuity(it->first, i);
      }
    }else{
      node_sptr lhs(new Variable(it->first));
      for(int i=0; i<=-it->second;i++){
        solver_->set_continuity(it->first, i);
        lhs = node_sptr(new Differential(lhs));
      }
      node_sptr rhs(new Number("0"));
      node_sptr cons(new Equal(lhs, rhs));
      solver_->add_constraint(cons);
    }
  }
}

SymbolicSimulator::TestResult SymbolicSimulator::simple_test(const module_set_sptr& ms){

  solver_->change_mode(TestMode, opts_->approx_precision);

  SymbolicSimulator::TestResult ret = TEST_FALSE;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  expanded_always_t expanded_always;
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  tells_t         tell_list;
  constraints_t   constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;

  variable_map_t vm;
  parameter_map_t pm;
  solver_->reset(vm,pm);

  // ask������W�߂� 
  ask_collector.collect_ask(&expanded_always, 
      &positive_asks, 
      &negative_asks);

  for(int i = 0; i < (1 << negative_asks.size()); i++){
    positive_asks_t tmp_positive_asks;
    solver_->start_temporary();

    negative_asks_t::iterator it = negative_asks.begin();
    for(int j = 0; j < (int)negative_asks.size(); j++, it++){
      if((i & (1 << j)) != 0){
        solver_->add_guard((*it)->get_guard());
	//	std::cout << "guard on : " << (*(*it)->get_guard()) << std::endl;
        tmp_positive_asks.insert(*it);
      }else{
	//	std::cout << "guard off : " << (*(*it)->get_guard()) << std::endl;
        solver_->add_guard(node_sptr(new Not((*it)->get_guard())));
      }
    }
    //    continuity_map = maker.get_continuity_map();
    //    add_continuity(continuity_map);

    tell_collector.collect_all_tells(&tell_list,
        &expanded_always, 
        &tmp_positive_asks);

    maker.reset();
    constraint_list.clear();

    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), false, false);
    }

    solver_->add_constraint(constraint_list);
    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map);

    {
      HYDLA_LOGGER_CLOSURE("--in_simple_test_check_consistency");
      HYDLA_LOGGER_CLOSURE(ms->get_name());
      node_sptr tmp_node;
      switch(solver_->test_consistency(tmp_node)){
        case SymbolicVirtualConstraintSolver::TEST_TRUE:
          // ���̐��񃂃W���[���W���͕K������
          return TEST_TRUE;
          break;
        case SymbolicVirtualConstraintSolver::TEST_FALSE:
          break;
        case SymbolicVirtualConstraintSolver::TEST_UNKNOWN:
          // ���̐��񃂃W���[���W���͖�������ꍇ������
          HYDLA_LOGGER_CLOSURE("receive false conditions : ", *tmp_node);
          ms->set_false_conditions(tmp_node);
          ret = TEST_UNKNOWN;
          break;
        default:
          assert(0);
          break;
      }
    }
    //    std::cout << std::endl;
    solver_->end_temporary();
    //    std::cout << std::endl;
  }
  //  std::cout << std::endl;
  return ret;
}


SymbolicSimulator::CheckEntailmentResult SymbolicSimulator::check_entailment(
  SymbolicVirtualConstraintSolver::check_consistency_result_t &cc_result,
  const node_sptr& guard,
  const continuity_map_t& cont_map
  )
{
  CheckEntailmentResult ce_result;
  solver_->start_temporary();
  add_continuity(cont_map);
  solver_->add_guard(guard);
  cc_result = solver_->check_consistency();
  if(!cc_result.true_parameter_maps.empty()){
    HYDLA_LOGGER_CLOSURE("%% entailable");
    if(!cc_result.false_parameter_maps.empty()){
      HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
      ce_result = BRANCH_PAR;
    }else
    {
      solver_->end_temporary();
      solver_->start_temporary();
      add_continuity(cont_map);
      solver_->add_guard(node_sptr(new Not(guard)));
      cc_result = solver_->check_consistency();
      if(!cc_result.true_parameter_maps.empty()){
        HYDLA_LOGGER_CLOSURE("%% entailment branches");
        if(!cc_result.false_parameter_maps.empty()){
          HYDLA_LOGGER_CLOSURE("%% inevitable entailment depends on conditions of parameters");
          ce_result = BRANCH_PAR;
        }
        ce_result = BRANCH_VAR;
      }else{
        ce_result = ENTAILED;
      }
    }
  }else{
    ce_result = CONFLICTING;
  }
  solver_->end_temporary();
  return ce_result;
}

CalculateClosureResult SymbolicSimulator::calculate_closure(simulation_phase_sptr_t& state,
    const module_set_sptr& ms)
{    
  HYDLA_LOGGER_CLOSURE("#*** Begin SymbolicSimulator::calculate_closure ***\n");

  phase_result_sptr_t &pr = state->phase_result;
  //�O����
  positive_asks_t& positive_asks = pr->positive_asks;
  negative_asks_t& negative_asks = pr->negative_asks;
  ask_set_t unknown_asks;
  expanded_always_t& expanded_always = pr->expanded_always;
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms);
  tells_t         tell_list;
  constraints_t   constraint_list;

  continuity_map_t continuity_map;
  ContinuityMapMaker maker;

  bool expanded;
  
  // ask������W�߂�
  ask_collector.collect_ask(&expanded_always, 
      &positive_asks, 
      &unknown_asks);
  
  do{
    // tell������W�߂�
    tell_collector.collect_new_tells(&tell_list,
        &expanded_always, 
        &positive_asks);

    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_consistency in calculate_closure\n");
    timer::Timer consistency_timer;
    //tell����Ȃ��Đ��񕔕��̂ݑ���
    constraint_list.clear();
    
    maker.reset();

    // �����ǉ����C����X�g�A���������������Ă��Ȃ����ǂ������ׂ�
    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
      maker.visit_node((*it), pr->phase == IntervalPhase, false);
    }

    continuity_map = maker.get_continuity_map();
    add_continuity(continuity_map);
    
    for(constraints_t::const_iterator it = state->temporary_constraints.begin(); it != state->temporary_constraints.end(); it++){
      constraint_list.push_back(*it);
    }

    solver_->add_constraint(constraint_list);

    {
      SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
      if(check_consistency_result.true_parameter_maps.empty()){
        // �K����������ꍇ
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return CalculateClosureResult();
      }else if (check_consistency_result.false_parameter_maps.empty()){
        // �K���[���\�ȏꍇ
        // �������Ȃ�
      }else{
        // �L���萔�̏����ɂ���ď[���\�����ω�����ꍇ
        HYDLA_LOGGER_CLOSURE("%% consistency depends on conditions of parameters\n");
        CalculateClosureResult result;
        push_branch_states(state, check_consistency_result, result);
        state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();
        return result;
      }
    }
    
    state->profile["CheckConsistency"] += consistency_timer.get_elapsed_us();


    // ask����̃G���e�[������
    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_entailment in calculate_closure\n");
    
    timer::Timer entailment_timer;
    
    {
      expanded = false;
      ask_set_t::iterator it  = unknown_asks.begin();
      ask_set_t::iterator end = unknown_asks.end();
      while(it!=end) {
      
        // ����0�ł͍��Ɍ��l�Ɋւ����������ɋU�Ƃ���
        if(pr->phase == PointPhase && pr->current_time->get_string() == "0" && PrevSearcher().search_prev((*it)->get_guard())){
          unknown_asks.erase(it++);
          continue;
        }
        maker.visit_node((*it)->get_child(), pr->phase == IntervalPhase, true);
        
        SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result;
        switch(check_entailment(check_consistency_result, (*it)->get_guard(), maker.get_continuity_map())){
          case ENTAILED:
            HYDLA_LOGGER_CLOSURE("--- entailed ask ---\n", *((*it)->get_guard()));
            positive_asks.insert(*it);
            //erase�ƌ�u�C���N�������g�͓����ɂ��Ȃ��ƃC�e���[�^������̂ŁC����
            unknown_asks.erase(it++);
            expanded = true;
            break;
          case CONFLICTING:
            HYDLA_LOGGER_CLOSURE("--- conflicted ask ---\n", *((*it)->get_guard()));
            negative_asks.insert(*it);
            unknown_asks.erase(it++);
            break;
          case BRANCH_VAR:
            HYDLA_LOGGER_CLOSURE("--- branched ask ---\n", *((*it)->get_guard()));
            it++;
            break;
          case BRANCH_PAR:
            HYDLA_LOGGER_CLOSURE("%% entailablity depends on conditions of parameters\n");
            CalculateClosureResult result;
            push_branch_states(state, check_consistency_result, result);
            state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
            return result;
            break;
        }
        maker.set_continuity_map(continuity_map);
      }
    }
    state->profile["CheckEntailment"] += entailment_timer.get_elapsed_us();
  }while(expanded);

  add_continuity(continuity_map);

  if(!unknown_asks.empty()){
    boost::shared_ptr<hydla::parse_tree::Ask> branched_ask = *unknown_asks.begin();
    // TODO: �ɑ含�ɑ΂��Č����Ȃ��̂ɂȂ��Ă��Ȃ��i���s�A���S���Y��������������ĂȂ��j
    HYDLA_LOGGER_CLOSURE("%% branched_ask:", TreeInfixPrinter().get_infix_string(branched_ask));
    CalculateClosureResult result;
    {
      // �����𐶐��i���o����Ȃ����j
      simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
      new_state->temporary_constraints.push_back(node_sptr(new Not((branched_ask)->get_guard())));
      result.push_back(new_state);
    }
    {
      // �����𐶐��i���o�������j
      simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
      new_state->temporary_constraints.push_back((branched_ask)->get_guard());
      result.push_back(new_state);
    }
    return result;
  }

  if(opts_->assertion){
    HYDLA_LOGGER_CLOSURE("%% SymbolicSimulator::check_assertion");
    SymbolicVirtualConstraintSolver::check_consistency_result_t cc_result;
    CalculateClosureResult ret;
    switch(check_entailment(cc_result, node_sptr(new Not(opts_->assertion)), continuity_map_t())){
      case ENTAILED:
        std::cout << "Assertion Failed!" << std::endl;
        HYDLA_LOGGER_CLOSURE("%% Assertion Failed!");
        is_safe_ = false;
        ret.push_back(state);
        return ret;
        break;
      case CONFLICTING:
        break;
      case BRANCH_VAR:
        HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of variables");
          // �����𐶐�
        {
          simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
          new_state->temporary_constraints.push_back(opts_->assertion);
          ret.push_back(new_state);
        }
        {
          simulation_phase_sptr_t new_state(create_new_simulation_phase(state));
          new_state->temporary_constraints.push_back(node_sptr(new Not(opts_->assertion)));
          ret.push_back(new_state);
        }
        return ret;
        break;
      case BRANCH_PAR:
        HYDLA_LOGGER_CLOSURE("%% failure of assertion depends on conditions of parameters");
        push_branch_states(state, cc_result, ret);
        return ret;
        break;
    }
  }
  HYDLA_LOGGER_CLOSURE("#*** End SymbolicSimulator::calculate_closure ***\n");
  return CalculateClosureResult(1, state);
}



SymbolicSimulator::simulation_phases_t SymbolicSimulator::simulate_ms_point(const module_set_sptr& ms, 
    simulation_phase_sptr_t& state,
    variable_map_t& vm,
    bool& consistent)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::simulate_ms_point***");

  //�O����
  phase_result_sptr_t& pr = state->phase_result;

  solver_->reset(vm, pr->parameter_map);
  
  if(opts_->optimization_level == 3){
    //   std::cout << ms->get_name() << " : " << std::endl;
    if(ms->get_false_conditions() != NULL){
      solver_->change_mode(TestMode, opts_->approx_precision);
      HYDLA_LOGGER_MS("#*** check_false_conditions***");
      HYDLA_LOGGER_MS(ms->get_name() , " : " , *ms->get_false_conditions());

      //      std::cout << ms->get_name() << " : " << *ms->get_false_conditions() << std::endl;
      
      solver_->set_false_conditions(ms->get_false_conditions());
      
      SymbolicVirtualConstraintSolver::check_consistency_result_t check_consistency_result = solver_->check_consistency();
      if(check_consistency_result.true_parameter_maps.empty()){
        // �K����������ꍇ
        consistent = false;
        return CalculateClosureResult();
      }else if (check_consistency_result.false_parameter_maps.empty()){
        // �K���[���\�ȏꍇ
        // �������Ȃ�
      }else{
        // �L���萔�̏����ɂ���ď[���\�����ω�����ꍇ
        consistent = false;
        CalculateClosureResult result;
        push_branch_states(state, check_consistency_result, result);
        return result;
      }
      
    }
  }

  solver_->change_mode(DiscreteMode, opts_->approx_precision);

  timer::Timer cc_timer;

  //��v�Z
  CalculateClosureResult result = calculate_closure(state, ms);
  
  state->profile["CalculateClosure"] += cc_timer.get_elapsed_us();


  if(result.size() != 1){
    consistent = false;
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_point(result.size() != 1)***\n");
    return result;
  }
  
  timer::Timer create_timer;
  // Interval Phase�ֈڍs�i����Ԃ̐����j
  SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
  state->profile["CreateMap"] += create_timer.get_elapsed_us();

  assert(create_result.result_maps.size()>0);
  
  pr->module_set = ms;

  simulation_phase_sptr_t new_state_original(create_new_simulation_phase());
  phase_result_sptr_t& new_pr_original = new_state_original->phase_result;
  new_pr_original->step         = pr->step+1;
  new_pr_original->phase        = IntervalPhase;
  new_pr_original->current_time = pr->current_time;
  new_pr_original->expanded_always = pr->expanded_always;
  new_pr_original->module_set = ms;

  simulation_phases_t phases;

  for(unsigned int create_it = 0; create_it < create_result.result_maps.size() && (opts_->nd_mode||create_it==0); create_it++)
  {
    simulation_phase_sptr_t new_state(create_new_simulation_phase(new_state_original));
    simulation_phase_sptr_t branch_state(create_new_simulation_phase(state));
    phase_result_sptr_t& npr = new_state->phase_result, &bpr = branch_state->phase_result;

    bpr->variable_map = range_map_to_value_map(bpr, create_result.result_maps[create_it], bpr->parameter_map);
    npr->parameter_map = bpr->parameter_map;
    
    bpr->parent->children.push_back(bpr);
    npr->parent = bpr;

    if(is_safe_){
      phases.push_back(new_state);
    }else{
      bpr->cause_of_termination = simulator::ASSERTION;
    }
  }

  HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_point***\n");
  consistent = true;
  return phases;
}

SymbolicSimulator::simulation_phases_t SymbolicSimulator::simulate_ms_interval(const module_set_sptr& ms, 
    simulation_phase_sptr_t& state,
    bool& consistent)
{
  HYDLA_LOGGER_MS("#*** Begin SymbolicSimulator::simulate_ms_interval***");
  //�O����
  phase_result_sptr_t& pr = state->phase_result;
  solver_->change_mode(ContinuousMode, opts_->approx_precision);
  solver_->reset(pr->parent->variable_map, pr->parameter_map);

  timer::Timer cc_timer;

  //��v�Z
  CalculateClosureResult result = calculate_closure(state, ms);

  state->profile["CalculateClosure"] += cc_timer.get_elapsed_us();
  
  if(result.size() != 1){
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_interval(result.size() != 1 && consistent = false)***\n");
    consistent = false;
    return result;
  }

  timer::Timer create_timer;
  SymbolicVirtualConstraintSolver::create_result_t create_result = solver_->create_maps();
  state->profile["CreateMap"] += create_timer.get_elapsed_us();
  SymbolicVirtualConstraintSolver::create_result_t::result_maps_t& results = create_result.result_maps;

  if(results.size() != 1){
    // �敪�I�ɘA���Ŗ������O�����܂ށD���f�D
    simulation_phase_sptr_t phase(create_new_simulation_phase(state));
    phase->phase_result->cause_of_termination = simulator::NOT_UNIQUE_IN_INTERVAL;
    phase->phase_result->parent->children.push_back(phase->phase_result);
    consistent = true;
    HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_interval(result.size() != 1 && consistent = true)***\n");
    return simulation_phases_t();
  }

  simulation_phase_sptr_t new_state_original(create_new_simulation_phase());
  phase_result_sptr_t &npr_original = new_state_original->phase_result;

  npr_original->step         = pr->step+1;
  npr_original->phase        = PointPhase;
  npr_original->expanded_always = pr->expanded_always;
  
  pr->variable_map = range_map_to_value_map(pr, results[0], pr->parameter_map);
  pr->variable_map = shift_variable_map_time(pr->variable_map, pr->current_time);

  pr->module_set = ms;
    
  simulation_phases_t phases;
  
  if(is_safe_){

    /*
    // MaxModule�̓��o
    module_set_sptr max_module_set = (*msc_no_init_).get_max_module_set();
    HYDLA_LOGGER_DEBUG("#** interval_phase: ms: **\n",
     *ms,
     "\n#** interval_phase: max_module_set: ##\n",
     *max_module_set);


    // �̗p���Ă��Ȃ����W���[���̃��X�g���o
    hydla::ch::ModuleSet::module_list_t diff_module_list(max_module_set->size() - ms->size());

    std::set_difference(
    max_module_set->begin(),
    max_module_set->end(),
    ms->begin(),
    ms->end(),
    diff_module_list.begin());


    // ���ꂼ��̃��W���[����singleton�ȃ��W���[���W���Ƃ���
    std::vector<module_set_sptr> diff_module_set_list;

    hydla::ch::ModuleSet::module_list_const_iterator diff_it = diff_module_list.begin();
    hydla::ch::ModuleSet::module_list_const_iterator diff_end = diff_module_list.end();
    for(; diff_it!=diff_end; ++diff_it){
    module_set_sptr diff_ms(new ModuleSet((*diff_it).first, (*diff_it).second));
    diff_module_set_list.push_back(diff_ms);
    }

    assert(diff_module_list.size() == diff_module_set_list.size());


    // diff_module_set_list���̊e���W���[���W�����ɂ�������Ȃ���������ꂼ�꓾��
    not_adopted_tells_list_t not_adopted_tells_list;

    std::vector<module_set_sptr>::const_iterator diff_ms_list_it = diff_module_set_list.begin();
    std::vector<module_set_sptr>::const_iterator diff_ms_list_end = diff_module_set_list.end();
    for(; diff_ms_list_it!=diff_ms_list_end; ++diff_ms_list_it){
    TellCollector not_adopted_tells_collector(*diff_ms_list_it);
    tells_t       not_adopted_tells;
    not_adopted_tells_collector.collect_all_tells(&not_adopted_tells,
    &expanded_always, 
    &positive_asks);
    not_adopted_tells_list.push_back(not_adopted_tells);
    }


    //���ݍ̗p����Ă��Ȃ�����𗣎U�ω������Ƃ��Ēǉ�
    for(not_adopted_tells_list_t::const_iterator it = not_adopted_tells_list.begin(); it != not_adopted_tells_list.end(); it++){
    tells_t::const_iterator na_it = it -> begin();
    tells_t::const_iterator na_end = it -> end();
    for(; na_it != na_end; na_it++){
    disc_cause.push_back((*na_it)->get_child());
    }
    }
    */
    timer::Timer next_pp_timer;
    constraints_t disc_cause;
    //���ݓ��o����Ă���K�[�h������Not���������̂𗣎U�ω������Ƃ��Ēǉ�
    for(positive_asks_t::const_iterator it = pr->positive_asks.begin(); it != pr->positive_asks.end(); it++){
      disc_cause.push_back(node_sptr(new Not((*it)->get_guard() ) ) );
    }
    //���ݓ��o����Ă��Ȃ��K�[�h�����𗣎U�ω������Ƃ��Ēǉ�
    for(negative_asks_t::const_iterator it = pr->negative_asks.begin(); it != pr->negative_asks.end(); it++){
      disc_cause.push_back((*it)->get_guard());
    }

    //assertion�̔ے��ǉ�
    if(opts_->assertion){
      disc_cause.push_back(node_sptr(new Not(opts_->assertion)));
    }
    
    time_t max_time(new SymbolicValue(node_sptr(new hydla::parse_tree::Number(opts_->max_time))));

    SymbolicVirtualConstraintSolver::PPTimeResult 
      time_result = solver_->calculate_next_PP_time(disc_cause, pr->current_time, max_time);

    for(unsigned int time_it=0; time_it<time_result.candidates.size() && (opts_->nd_mode||time_it==0); time_it++){
      simulation_phase_sptr_t branch_state(create_new_simulation_phase(state));
      phase_result_sptr_t &bpr = branch_state->phase_result;
      SymbolicVirtualConstraintSolver::PPTimeResult::NextPhaseResult &candidate = time_result.candidates[time_it];
      
      // ���ڑ������ƁC�l�̏�����������Ȃ��L���萔�ɂ��Ă̘g�������Ȃ��Ă��܂��̂ŁC�ǉ��݂̂��s���D
      for(parameter_map_t::iterator it = candidate.parameter_map.begin(); it != candidate.parameter_map.end(); it++){
        bpr->parameter_map[it->first] = it->second;
      }
      bpr->parent->children.push_back(bpr);
      simulation_phase_sptr_t new_state(create_new_simulation_phase(new_state_original));
      phase_result_sptr_t& npr = new_state->phase_result;
      
      bpr->end_time = candidate.time;
      if(!candidate.is_max_time ) {
        npr->current_time = candidate.time;
        solver_->simplify(npr->current_time);
        npr->parameter_map = bpr->parameter_map;
        npr->parent = bpr;
        phases.push_back(new_state);
      }else{
        bpr->cause_of_termination = simulator::TIME_LIMIT;
      }
    }
    state->profile["NextPP"] += next_pp_timer.get_elapsed_us();
  }else{
    pr->parent->children.push_back(pr);
    pr->cause_of_termination = simulator::ASSERTION;
  }

  HYDLA_LOGGER_MS("#*** End SymbolicSimulator::simulate_ms_interval***");
  consistent = true;
  return phases;
}

SymbolicSimulator::variable_map_t SymbolicSimulator::range_map_to_value_map(
  phase_result_sptr_t& state,
  const hydla::vcs::SymbolicVirtualConstraintSolver::variable_range_map_t& rm,
  parameter_map_t &parameter_map)
{
  variable_map_t ret = *variable_map_;
  for(vcs::SymbolicVirtualConstraintSolver::variable_range_map_t::const_iterator r_it = rm.begin(); r_it != rm.end(); r_it++){
    variable_t* variable = get_variable(r_it->first->get_name(), r_it->first->get_derivative_count());
    if(r_it->second.is_unique()){
      ret[variable] = r_it->second.get_lower_bound().value;
    }else{
      parameter_t param(r_it->first, state);
      parameter_set_->push_front(param);
      parameter_map[&(parameter_set_->front())] = r_it->second;
      ret[variable] = value_t(new SymbolicValue(node_sptr(
        new Parameter(variable->get_name(), variable->get_derivative_count(), state->id))));
      // TODO:�L���萔������C�e�ϐ��̐����ɏo������ϐ����L���萔�ɒu��������
    }
  }
  return ret;
}


SymbolicSimulator::variable_map_t SymbolicSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
variable_map_t shifted_vm;
variable_map_t::const_iterator it  = vm.begin();
variable_map_t::const_iterator end = vm.end();
for(; it!=end; ++it) {
  if(it->second->is_undefined())
    shifted_vm[it->first] = it->second;
  else
    shifted_vm[it->first] = solver_->shift_expr_time(it->second, time);
}
return shifted_vm;
}



} //namespace symbolic_simulator
} //namespace hydla
