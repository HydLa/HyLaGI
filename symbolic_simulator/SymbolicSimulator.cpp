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

//���ǉ�
#include "RTreeVisitor.h"
#include "TellCollector.h"
#include "AskCollector.h"

#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "StateResult.h"
#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "DiscreteAskRemover.h"
#include "AskTypeAnalyzer.h"
#include "../virtual_constraint_solver/mathematica/MathematicaVCS.h"
#include "../virtual_constraint_solver/reduce/REDUCEVCS.h"

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;

using namespace hydla::vcs::reduce;

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::simulator;
using namespace hydla::parse_tree;
using namespace hydla::logger;

namespace hydla {
namespace symbolic_simulator {

SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
  opts_(opts), is_safe_(true)
{
}

SymbolicSimulator::~SymbolicSimulator()
{
}

void SymbolicSimulator::do_initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);
  
  opts_.assertion = parse_tree->get_assertion_node();
  result_root_.reset(new StateResult());
  //������Ԃ�����ăX�^�b�N�ɓ����
  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->step        = 0;
  state->current_time = time_t("0");
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;
  state->parent_state_result = result_root_;
  push_phase_state(state);


  //solver_����
  if(opts_.solver == "r" || opts_.solver == "Reduce") {
    solver_.reset(new REDUCEVCS(opts_, variable_map_));   //�g�p����\���o������
  }else{
    solver_.reset(new MathematicaVCS(opts_));   //�g�p����\���o������
  }
}

namespace {
struct ModuleSetContainerInitializer {
  typedef boost::shared_ptr<ParseTree> parse_tree_sptr;
  template<typename MSCC>
  static void init(
    const parse_tree_sptr& parse_tree,
    module_set_container_sptr& msc_original, 
    module_set_container_sptr& msc_no_init,
    parse_tree_sptr& member_parse_tree)
  {
    ModuleSetContainerCreator<MSCC> mcc;
    {
      parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
      AskDisjunctionFormatter().format(pt_original.get());
      AskDisjunctionSplitter().split(pt_original.get());
      //AskTypeAnalyzer().analyze(pt_original.get());
      msc_original = mcc.create(pt_original);
    }

    {
      parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
      InitNodeRemover().apply(pt_no_init.get());
      AskDisjunctionFormatter().format(pt_no_init.get());
      AskDisjunctionSplitter().split(pt_no_init.get());
      //AskTypeAnalyzer().analyze(pt_no_init.get());
      msc_no_init = mcc.create(pt_no_init);

      // �œK�����ꂽ�`�̃p�[�X�c���[�𓾂�
      member_parse_tree = pt_no_init;
    }
  }
};

}

void SymbolicSimulator::init_module_set_container(const parse_tree_sptr& parse_tree)
{  
  HYDLA_LOGGER_DEBUG("#*** create module set list ***\n",
                     "nd_mode=", opts_.nd_mode);
  
  if(opts_.nd_mode||opts_.interactive_mode) {
    //�S��T�����[�h�Ȃ�
    ModuleSetContainerInitializer::init<ModuleSetGraph>(
      parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
  else {
    //�ʏ���s���[�h
    ModuleSetContainerInitializer::init<ModuleSetList>(
      parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
}

void SymbolicSimulator::simulate()
{
  while(!state_stack_.empty() && is_safe_) {
    phase_state_sptr state(pop_phase_state());
    bool has_next = false;
    if( opts_.max_step >= 0 && state->step > opts_.max_step)
      continue;
    state->module_set_container->reset(state->visited_module_sets);
    do{
      if(simulate_phase_state(state->module_set_container->get_module_set(), state)){
        state->module_set_container->mark_nodes();
        has_next = true;
        if(!opts_.nd_mode)break;
      }
      else{
        state->module_set_container->mark_current_node();
      }
      state->positive_asks.clear();
      
      //�������ȉ���⃂�W���[���W�������݂��Ȃ��ꍇ
      if(!state->module_set_container->go_next()){
        state->parent_state_result->cause_of_termination = StateResult::INCONSISTENCY;
      }
    }while( state->module_set_container->go_next() && is_safe_);
  }
  output_result_tree();
}


CalculateClosureResult SymbolicSimulator::calculate_closure(const phase_state_const_sptr& state,
                                          const module_set_sptr& ms, expanded_always_t &expanded_always,
                                          positive_asks_t &positive_asks, negative_asks_t &negative_asks){

  //�O����
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
                              AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
                              AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);
  tells_t         tell_list;
  constraints_t   constraint_list;
  boost::shared_ptr<hydla::parse_tree::Ask>  const *branched_ask;                           //UNKNOWN�Ԃ��ꂽAsk�����

  
  bool expanded;
  do{
    branched_ask=NULL;
    // tell������W�߂�
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    HYDLA_LOGGER_DEBUG("#** calculate_closure: expanded always after collect_new_tells: **\n",
                       expanded_always);
    
    
    HYDLA_LOGGER_CC("#** SymbolicSimulator::check_consistency in calculate_closure: **\n");

    
    constraint_list.clear();
    for(tells_t::iterator it = tell_list.begin(); it != tell_list.end(); it++){
      constraint_list.push_back((*it)->get_child());
    }
 
    for(constraints_t::const_iterator it = state->added_constraints.begin(); it != state->added_constraints.end(); it++){
      constraint_list.push_back(*it);
    }
    
    //tell����Ȃ��Đ��񕔕��̂ݑ���
    solver_->add_constraint(constraint_list);
    
    // �����ǉ����C����X�g�A���������������Ă��Ȃ����ǂ���
    switch(solver_->check_consistency()) 
    {
      case VCSR_TRUE:
        // do nothing
        break;
      case VCSR_FALSE:
        return CC_FALSE;
        break;
      case VCSR_SOLVER_ERROR:
        // TODO: ��O�Ƃ��Ȃ�����ABP�V�~�����[�^�Ɉڍs������
        assert(0);
        break;
      default:
        // TODO: ��O�Ƃ��Ȃ�����ABP�V�~�����[�^�Ɉڍs������
        assert(0);
        break;
    }

    // ask������W�߂�
    ask_collector.collect_ask(&expanded_always, 
                              &positive_asks, 
                              &negative_asks);


    HYDLA_LOGGER_DEBUG("#** calculate_closure: expanded always after collect_ask: **\n",
                       expanded_always);

    HYDLA_LOGGER_CC("#** SymbolicSimulator::check_entailment in calculate_closure: **\n");
    // ask����̃G���e�[������
    expanded = false;
    negative_asks_t::iterator it  = negative_asks.begin();
    negative_asks_t::iterator end = negative_asks.end();
    constraints_t tmp_constraints;
    while(it!=end) {
      tmp_constraints.clear();
      tmp_constraints.push_back((*it)->get_guard());
      switch(solver_->check_consistency(tmp_constraints))
      {
        case VCSR_TRUE:
        {
          tmp_constraints.clear();
          tmp_constraints.push_back(node_sptr(new Not((*it)->get_guard())));
          switch(solver_->check_consistency(tmp_constraints)){
            case VCSR_TRUE:
              if(!expanded&&!branched_ask){
                branched_ask = &(*it);
              }
              it++;
              break;
              
            case VCSR_FALSE:
              positive_asks.insert(*it);
              negative_asks.erase(it++);
              expanded = true;
              break;
            
            default:
            // TODO: ��O�Ƃ��Ȃ�����ABP�V�~�����[�^�Ɉڍs������
            assert(0);
            break;
          }
          break;
        }
        case VCSR_FALSE:
          it++;
          break;
        default:
          // TODO: ��O�Ƃ��Ȃ�����ABP�V�~�����[�^�Ɉڍs������
          assert(0);
          break;
      }
    }
  }while(expanded);
  
  if(branched_ask!=NULL){
    HYDLA_LOGGER_CC("#*** create new phase state (branch) ***");
    if(opts_.nd_mode){
      // �����𐶐��i���o����Ȃ����j
      phase_state_sptr new_state(create_new_phase_state(state));
      new_state->module_set_container = state->module_set_container;
      new_state->visited_module_sets = state->module_set_container->get_visited_module_sets();
      new_state->parent_state_result = state->parent_state_result;
      new_state->phase = state->phase;
      new_state->added_constraints = state->added_constraints;
      new_state->added_constraints.push_back(node_sptr(new Not((*branched_ask)->get_guard())));
      push_phase_state(new_state);
      HYDLA_LOGGER_CC("---push_phase_state(not_ask)---\n", **branched_ask,"\n");
    }
    // �����𐶐��i���o�������j
    phase_state_sptr new_state(create_new_phase_state(state));
    new_state->module_set_container = state->module_set_container;
    new_state->visited_module_sets = state->module_set_container->get_visited_module_sets();
    new_state->parent_state_result = state->parent_state_result;
    new_state->phase = state->phase;
    new_state->added_constraints = state->added_constraints;
    new_state->added_constraints.push_back((*branched_ask)->get_guard());
    push_phase_state(new_state);
    HYDLA_LOGGER_CC("---push_phase_state(ask)---\n", **branched_ask,"\n");
    return CC_BRANCH;
  }
  
  if(opts_.assertion){
    HYDLA_LOGGER_CC("#** SymbolicSimulator::check_assertion **\n");
    if(solver_->check_consistency(constraints_t(1, opts_.assertion) ) == VCSR_FALSE ){
      std::cout << "Assertion Failed!" << std::endl;
      is_safe_ = false;
      return CC_TRUE;
    }
  }
  return CC_TRUE;
}


bool SymbolicSimulator::point_phase(const module_set_sptr& ms, 
                                const phase_state_const_sptr& state)
{

  //�O����
  if(state->changed_asks.size() != 0) {
    HYDLA_LOGGER_DEBUG("#** point_phase: changed_ask_id: ",
                       state->changed_asks.at(0).second,
                       " **");
  }
  expanded_always_t expanded_always;
  expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  HYDLA_LOGGER_DEBUG("#** point_phase: expanded always from IP: **\n",
                     expanded_always);  
  solver_->change_mode(DiscreteMode, opts_.approx_precision);
  solver_->reset(state->variable_map, state->parameter_map);

  positive_asks_t positive_asks(state->positive_asks);
  negative_asks_t negative_asks;


  //��v�Z
  switch(calculate_closure(state,ms,expanded_always,positive_asks,negative_asks)){
    case CC_TRUE:
    break;
    case CC_FALSE:
    return false;    
    case CC_BRANCH:
    return true;
  }

  

  SymbolicVirtualConstraintSolver::create_result_t create_result;
  HYDLA_LOGGER_MS("#** SymbolicSimulator::create_map: **\n");  
  solver_->create_maps(create_result);
  for(unsigned int create_it = 0; create_it < create_result.result_maps.size()&&(opts_.nd_mode||create_it==0); create_it++)
  {
    // Interval Phase�ֈڍs�i����Ԃ̐����j
    HYDLA_LOGGER_DEBUG("#*** create new phase state ***");
    phase_state_sptr new_state(create_new_phase_state());
    new_state->step         = state->step+1;
    new_state->phase        = IntervalPhase;
    new_state->current_time = state->current_time;
    expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
    HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                       new_state->expanded_always_id);
    new_state->module_set_container = msc_no_init_;
 
    take_all_variables(state->variable_map, create_result.result_maps[create_it].variable_map);
    
    new_state->variable_map = create_result.result_maps[create_it].variable_map;
    new_state->parameter_map = create_result.result_maps[create_it].parameter_map;
    
    
    state_result_sptr_t state_result(new StateResult(new_state->variable_map,
                                                     new_state->parameter_map,
                                                     new_state->current_time,
                                                     PointPhase,
                                                     state->parent_state_result,
                                                     is_safe_?StateResult::NONE:StateResult::ASSERTION));
    state_result->parent->children.push_back(state_result);
    new_state->parent_state_result = state_result;

    //��Ԃ��X�^�b�N�ɉ�������
    HYDLA_LOGGER_DEBUG("---push_phase_state---\n", new_state->current_time,"\n",new_state->variable_map,"\n", new_state->parameter_map);
    push_phase_state(new_state);
    
    if(opts_.dump_in_progress)
      output_state_result(*state_result, true, false);

    HYDLA_LOGGER_MS_SUMMARY("%%%%%%%%%%%%% point phase result  %%%%%%%%%%%%%\n",
                       "time:", new_state->current_time, "\n",
                       new_state->variable_map,
                       new_state->parameter_map);
  }
  
  HYDLA_LOGGER_MS_SUMMARY("#*** end point phase ***");

  return true;
}

bool SymbolicSimulator::interval_phase(const module_set_sptr& ms, 
                                   const phase_state_const_sptr& state)
{
  //�O����
  expanded_always_t expanded_always;
  expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  HYDLA_LOGGER_DEBUG("#** interval_phase: expanded always from PP: **\n",
                     expanded_always);

  solver_->change_mode(ContinuousMode, opts_.approx_precision);
  solver_->reset(state->variable_map, state->parameter_map);
  negative_asks_t negative_asks;
  positive_asks_t positive_asks(state->positive_asks);

  //��v�Z
  switch(calculate_closure(state,ms,expanded_always, positive_asks ,negative_asks)){
    case CC_TRUE:
    break;
    case CC_FALSE:
    return false;    
    case CC_BRANCH:
    return true;
  }
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

  // ask�̓��o��Ԃ��ω�����܂Őϕ��������Ȃ�
  SymbolicVirtualConstraintSolver::IntegrateResult integrate_result;
  HYDLA_LOGGER_MS("#** SymbolicSimulator::integrate: **\n");  
  constraints_t disc_cause;
  
  //���ݓ��o����Ă���K�[�h������Not���������̂𗣎U�ω������Ƃ��Ēǉ�
  for(positive_asks_t::const_iterator it = positive_asks.begin(); it != positive_asks.end(); it++){
    disc_cause.push_back(node_sptr(new Not((*it)->get_guard() ) ) );
  }
  //���ݓ��o����Ă��Ȃ��K�[�h�����𗣎U�ω������Ƃ��Ēǉ�
  for(negative_asks_t::const_iterator it = negative_asks.begin(); it != negative_asks.end(); it++){
    disc_cause.push_back((*it)->get_guard());
  }
  /*
  // ���W���[���̊e�����AND�łȂ���ꍇ
  for(not_adopted_tells_list_t::const_iterator it = not_adopted_tells_list.begin(); it != not_adopted_tells_list.end(); it++){
    node_sptr tmp_and_node;
    tells_t::const_iterator na_it = it -> begin();
    tells_t::const_iterator na_end = it -> end();
    if(na_it != na_end ){
      tmp_and_node = (*na_it)->get_child();
      na_it++;
      for(; na_it != na_end; na_it++){
        tmp_and_node.reset(new LogicalAnd(tmp_and_node, (*na_it)->get_child() ) ); 
      }
    }
    if(tmp_and_node)disc_cause.push_back(tmp_and_node);
  }*/
  
  //���ݍ̗p����Ă��Ȃ�����𗣎U�ω������Ƃ��Ēǉ�
  for(not_adopted_tells_list_t::const_iterator it = not_adopted_tells_list.begin(); it != not_adopted_tells_list.end(); it++){
    tells_t::const_iterator na_it = it -> begin();
    tells_t::const_iterator na_end = it -> end();
    for(; na_it != na_end; na_it++){
      disc_cause.push_back((*na_it)->get_child());
    }
  }
  //assertion�̔ے��ǉ�
  if(opts_.assertion){
    disc_cause.push_back(node_sptr(new Not(opts_.assertion)));
  }
  
  solver_->integrate(
    integrate_result,
    disc_cause,
    state->current_time,
    time_t(node_sptr(new hydla::parse_tree::Number(opts_.max_time))));

  //to next pointphase
  for(int it=0;it<(int)integrate_result.states.size()&&(opts_.nd_mode||it==0);it++){
    take_all_variables(state->variable_map, integrate_result.states[it].variable_map);
    state_result_sptr_t state_result(new StateResult(integrate_result.states[it].variable_map,
                                                       integrate_result.states[it].parameter_map,
                                                       integrate_result.states[it].time,
                                                       IntervalPhase,
                                                       state->parent_state_result,
                                                       is_safe_?(integrate_result.states[it].is_max_time?StateResult::TIME_LIMIT:StateResult::NONE):StateResult::ASSERTION));
    state_result->parent->children.push_back(state_result);
    if(!integrate_result.states[it].is_max_time) {
      phase_state_sptr new_state(create_new_phase_state());
      new_state->phase        = PointPhase;
      new_state->step         = state->step;
      expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
      HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                       new_state->expanded_always_id);
      new_state->module_set_container = msc_no_init_;
      new_state->current_time = integrate_result.states[it].time;
      new_state->parameter_map = integrate_result.states[it].parameter_map;
      //���̃t�F�[�Y�ɂ�����ϐ��̒l�𓱏o����
      HYDLA_LOGGER_DEBUG("--- calc next phase variable map ---");
      solver_->apply_time_to_vm(integrate_result.states[it].variable_map, new_state->variable_map, integrate_result.states[it].time-state->current_time);

      new_state->parent_state_result = state_result;
      HYDLA_LOGGER_MS("---push_phase_state(interval)---", it, "\n", new_state->current_time, "\n", new_state->variable_map, "\n", new_state->parameter_map, "\n"); 
      push_phase_state(new_state);
      if(opts_.dump_in_progress){
        output_state_result(*state_result, true, false);
      }
    }
    
      
    HYDLA_LOGGER_MS_SUMMARY("%%%%%%%%%%%%% interval phase result  %%%%%%%%%%%%%\n",
                       "time:", solver_->get_real_val(integrate_result.states[it].time, 5), "\n",
                       integrate_result.states[it].variable_map,
                       integrate_result.states[it].parameter_map);

  }
  return true;
}

variable_map_t SymbolicSimulator::shift_variable_map_time(const variable_map_t& vm, const time_t &time){
    variable_map_t shifted_vm;
    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
      if(it->second.is_undefined())
        shifted_vm.set_variable(it->first, it->second);
      else
        shifted_vm.set_variable(it->first, solver_->shift_expr_time(it->second, time));
    }
    return shifted_vm;
}



std::string SymbolicSimulator::range_to_string(const value_range_t& val){
  std::string tmp;
  if(val.is_undefined()||val.is_unique())
    return val.get_first_value().get_string();
  value_range_t::or_const_iterator or_it = val.or_begin(), or_end  = val.or_end();
  while(1){
    value_range_t::and_const_iterator and_it = or_it->begin(), and_end  = or_it->end();
    while(1){
      tmp.append(and_it->get_symbol());
      tmp.append(and_it->get_value().get_string());
      if(++and_it==and_end)break;
      tmp.append("&");
    }
    if(++or_it==or_end)break;
    tmp.append("|");
  }
  return tmp;
}


void SymbolicSimulator::output_parameter_map(const parameter_map_t& pm)
{
  parameter_map_t::const_iterator it  = pm.begin();
  parameter_map_t::const_iterator end = pm.end();
  if(it != end){
    std::cout << "\n#---------parameter condition---------\n";
  }
  for(; it!=end; ++it) {
    std::cout << "p" << it->first << "\t: " << range_to_string(it->second) << "\n";
  }
}

void SymbolicSimulator::output_variable_map(const variable_map_t& vm, const time_t& time, const bool& numeric)
{
  variable_map_t::const_iterator it  = vm.begin();
  variable_map_t::const_iterator end = vm.end();
  if(numeric){
    std::cout << std::endl;
    std::cout << solver_->get_real_val(time, opts_.output_precision) << "\t";
    for(; it!=end; ++it) {
      std::cout << solver_->get_real_val(it->second, opts_.output_precision) << "\t";
    }
  }else{
    for(; it!=end; ++it) {
      std::cout << it->first << "\t: " << it->second << "\n";
    }
  }
}

void SymbolicSimulator::take_all_variables(const variable_map_t& from, variable_map_t& to)
{
  variable_map_t::const_iterator it  = from.begin();
  variable_map_t::const_iterator end = from.end();
  value_t undef_value;
  for(; it!=end; ++it) {
    // get���āC���݂��Ȃ��ꍇ��UNDEF�̂��̂��V�����ǉ������̂ł���ł���
    to.get_variable(it->first);
  }
}


void SymbolicSimulator::output_result_tree()
{
  if(result_root_->children.size() == 0){
    std::cout << "No Result." << std::endl;
    return;
  }
  int i = 1;
  while(1){
    state_result_sptr_t now_node = result_root_->children.back();
    if(opts_.nd_mode)
      std::cout << "#---------Case " << i++ << "---------" << std::endl;
    int phase_num = 0;
    while(1){
      std::cout << "\n#Phase No." << ++phase_num << std::endl;
      output_state_result(*now_node, false, opts_.output_format == fmtNumeric);
      if(now_node->children.size() == 0){//�t�ɓ��B
        output_parameter_map(now_node->parameter_map);
        std::cout << std::endl;
        std::cout << "#";
        switch(now_node->cause_of_termination){
          case StateResult::INCONSISTENCY:
            std::cout << "execution stacked\n";
          break;
          case StateResult::TIME_LIMIT:
            std::cout << "time ended\n" ;
          break;
          
          case StateResult::ERROR:
            std::cout << "some error occured\n" ;
          break;
          
          case StateResult::ASSERTION:
            std::cout << "assertion failed\n" ;
          break;
          
          case StateResult::NONE:
            std::cout << "unknown termination occured\n" ;
          break;
        }
        std::cout << std::endl;
        while(now_node->children.size() == 0){
          if(now_node->parent != NULL){
            now_node = now_node->parent;
          }else{
            return;//�e�����Ȃ��Ƃ������Ƃ͍��܂ŗ����Ƃ������ƂȂ̂ŏI���D
          }
          now_node->children.pop_back();
        }
        break;
      }
      else{
        now_node = now_node->children.back();
      }
    }
    std::cout << std::endl;
  }
}

void SymbolicSimulator::output_state_result(const StateResult& result, const bool& need_parameter,const bool& numeric){
  if(!numeric){
    variable_map_t vm;
    if(result.phase_type==IntervalPhase){
      std::cout << "---------IP---------" << std::endl;
      vm = shift_variable_map_time(result.variable_map, result.parent->time);
      std::cout << "time\t: " << result.parent->time << " -> " << result.time << "\n";
    }else{
      std::cout << "---------PP---------" << std::endl;
      vm = result.variable_map;
      std::cout << "time\t: " << result.time << "\n";
    }
    output_variable_map(vm, result.time, false);
  }else{
    if(result.phase_type==IntervalPhase){
      std::cout << "#---------IP---------" << std::endl;
      output_variable_labels(result.variable_map);
      variable_map_t output_vm;
      time_t elapsed_time("0");
      time_t limit_time = result.time-result.parent->time;
      solver_->simplify(limit_time);
      do{
        solver_->apply_time_to_vm(result.variable_map, output_vm, elapsed_time);
        output_variable_map(output_vm, (elapsed_time+result.parent->time), true);
        elapsed_time += time_t(opts_.output_interval);
        solver_->simplify(elapsed_time);
      }while(solver_->less_than(elapsed_time, limit_time));
      solver_->apply_time_to_vm(result.variable_map, output_vm, limit_time);
      output_variable_map(output_vm, result.time, true);

      std::cout << std::endl;
    }else{
      std::cout << "#---------PP---------" << std::endl;
      output_variable_labels(result.variable_map);
      output_variable_map(result.variable_map, result.time, true);
    }
    std::cout << std::endl;
  }
  
  if(need_parameter){
      output_parameter_map(result.parameter_map);
  }
}

void SymbolicSimulator::output_variable_labels(const variable_map_t variable_map){
    // �ϐ��̃��x��
    // TODO: ����`�̒l�Ƃ��̂����ł����\������?
  std::cout << "# time\t";
  BOOST_FOREACH(const variable_map_t::value_type& i, variable_map) {
   std::cout << i.first << "\t";
  }
  std::cout << std::endl;
}

} //namespace symbolic_simulator
} //namespace hydla
