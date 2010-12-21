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

#include "vcs_math_source.h"

#include "TellCollector.h"
#include "AskCollector.h"

#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "DiscreteAskRemover.h"
#include "AskTypeAnalyzer.h"

using namespace hydla::vcs;
using namespace hydla::vcs::mathematica;

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::simulator;
using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

//struct rawchar_formatter
//{
//  string operator()(smatch const &what) const
//  {
//    char c[2] = {0, 0};
//    c[0] = (char)strtol(what.str(1).c_str(), NULL, 8);
//    return c;
//  }
//};
//sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
//std::ostream_iterator< char > out_iter( std::cout );
//rawchar_formatter rfmt;
//regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	

SymbolicSimulator::SymbolicSimulator(const Opts& opts) :
  opts_(opts)
//  vcs_(MathematicaVCS::DiscreteMode, &ml_)
{
}

SymbolicSimulator::~SymbolicSimulator()
{
}

void SymbolicSimulator::do_initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  //������Ԃ�����ăX�^�b�N�ɓ����
  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->current_time = symbolic_time_t();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;
  push_phase_state(state);

  

  // �ϐ��̃��x��
  // TODO: ����`�̒l�Ƃ��̂����ł����\������?
  std::cout << "# time\t";
  BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
    std::cout << i.first << "\t";
  }
  std::cout << std::endl;


  //mathematica�֘A
  init_mathlink();
  
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

    //{
    //  parse_tree_sptr pt_no_init_discreteask(boost::make_shared<ParseTree>(*parse_tree));
    //  InitNodeRemover().apply(pt_no_init_discreteask.get());
    //  DiscreteAskRemover().apply(pt_no_init_discreteask.get());
    //  AskDisjunctionFormatter().format(pt_no_init_discreteask.get());
    //  AskDisjunctionSplitter().split(pt_no_init_discreteask.get());
    //  msc_no_init_discreteask_ = mcc.create(pt_no_init_discreteask);
    //}
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


void SymbolicSimulator::init_mathlink()
{
  HYDLA_LOGGER_DEBUG("#*** init mathlink ***");

  //TODO: ��O�𓊂���悤�ɂ���
  if(!ml_.init(opts_.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }

  // �o�͂����ʂ̉����̐ݒ�
  ml_.MLPutFunction("SetOptions", 2);
  ml_.MLPutSymbol("$Output"); 
  ml_.MLPutFunction("Rule", 2);
  ml_.MLPutSymbol("PageWidth"); 
  ml_.MLPutSymbol("Infinity"); 
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �f�o�b�O�v�����g
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseDebugPrint"); 
  ml_.MLPutSymbol(opts_.debug_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �v���t�@�C�����[�h
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseProfile"); 
  ml_.MLPutSymbol(opts_.profile_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // ���񃂁[�h
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optParallel"); 
  ml_.MLPutSymbol(opts_.parallel_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // �o�͌`��
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optOutputFormat"); 
  switch(opts_.output_format) {
    case fmtTFunction:
      ml_.MLPutSymbol("fmtTFunction");
      break;

    case fmtNumeric:
    default:
      ml_.MLPutSymbol("fmtNumeric");
      break;
  }
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // HydLa.m�̓��e���M
  //   ml_.MLPutFunction("Get", 1);
  //   ml_.MLPutString("symbolic_simulator/HydLa.m");
  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(vcs_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
}


void SymbolicSimulator::simulate()
{

  while(!state_stack_.empty()) {
    phase_state_sptr state(pop_phase_state());
    branch_=0;
    state->module_set_container->dispatch(
    boost::bind(&SymbolicSimulator::simulate_phase_state, 
               this, _1, state));
    if(opts_.nd_mode&&opts_.output_style==styleList){
     if(!branch_){
       if(!output_vector_.empty()){
         std::cout << std::endl;
         std::cout << "# time\t";
         BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
           std::cout << i.first << "\t";
         }
         std::cout << std::endl;
         std::vector<string>::iterator it = output_vector_.begin();
         std::vector<string>::iterator end = output_vector_.end();
         for(;it!=end;it++){
            std::cout << *it;
            std::cout << std::endl;
         }
       }
       std::cout << output_buffer_.str();

       while(!branch_stack_.empty()&&--branch_stack_.top()<=0){
         branch_stack_.pop();
         if(!output_vector_.empty()){
           output_vector_.pop_back();
         }
       }
       output_buffer_.str("");
     }else if(branch_>1){
       branch_stack_.push(branch_);
       output_vector_.push_back(output_buffer_.str());
       output_buffer_.str("");
     }
    }
  }
}


bool SymbolicSimulator::calculate_closure(const module_set_sptr& ms, MathematicaVCS &vcs, expanded_always_t &expanded_always,
                                          positive_asks_t &positive_asks, negative_asks_t &negative_asks){

  //�O����
  TellCollector tell_collector(ms);
  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
                              AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
                              AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);
  tells_t         tell_list;


  bool expanded   = true;
  while(expanded) {
    // tell������W�߂�
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    HYDLA_LOGGER_DEBUG("#** calculate_closure: expanded always after collect_new_tells: **\n",
                       expanded_always);  

    // �����ǉ����C����X�g�A���������������Ă��Ȃ����ǂ���
    switch(vcs.add_constraint(tell_list)) 
    {
      case VCSR_TRUE:
        // do nothing
        break;
      case VCSR_FALSE:
        return false;
        break;
      case VCSR_UNKNOWN:
        assert(0);
        break;
      case VCSR_SOLVER_ERROR:
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

    // ask����̃G���e�[������
    expanded = false;
    negative_asks_t::iterator it  = negative_asks.begin();
    negative_asks_t::iterator end = negative_asks.end();
    while(it!=end) {
      switch(vcs.check_entailment(*it))
      {
        case VCSR_TRUE:
          expanded = true;
          positive_asks.insert(*it);
          negative_asks.erase(it++);
          break;
        case VCSR_FALSE:
          it++;
          break;
        case VCSR_UNKNOWN:
          assert(0);
          break;
        case VCSR_SOLVER_ERROR:
          // TODO: ��O�Ƃ��Ȃ�����ABP�V�~�����[�^�Ɉڍs������
          assert(0);
          break;
      }
    }
  }
  return true;
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
  MathematicaVCS vcs(MathematicaVCS::DiscreteMode, &ml_, opts_.approx_precision);
  /*vcs.set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&SymbolicSimulator::output, this, _1, _2));*/
  vcs.reset(state->variable_map);

  positive_asks_t positive_asks;
  negative_asks_t negative_asks;


  //��v�Z
  if(!calculate_closure(ms, vcs,expanded_always,positive_asks,negative_asks)){
    return false;
  }


  // Interval Phase�ֈڍs�i����Ԃ̐����j
  HYDLA_LOGGER_DEBUG("#*** create new phase state ***");
  phase_state_sptr new_state(create_new_phase_state());
  new_state->phase        = IntervalPhase;
  new_state->current_time = state->current_time;
  expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                     new_state->expanded_always_id);
  new_state->module_set_container = msc_no_init_;

  {
    // �b��I�ȃt���[�������̏���
    // ����`�̒l��ϐ��\�ɑ��݂��Ȃ��ꍇ�͈ȑO�̒l���R�s�[
    vcs.create_variable_map(new_state->variable_map);
    variable_map_t::const_iterator it  = state->variable_map.begin();
    variable_map_t::const_iterator end = state->variable_map.end();
    for(; it!=end; ++it) {
      if(new_state->variable_map.get_variable(it->first).is_undefined())
      {
        new_state->variable_map.set_variable(it->first, it->second);
      }
    }
  }


  //�o��
  output(new_state->current_time, new_state->variable_map);

  //��Ԃ��X�^�b�N�ɉ�������
  push_phase_state(new_state);

  HYDLA_LOGGER_SUMMARY("%%%%%%%%%%%%% point phase result  %%%%%%%%%%%%%\n",
                     "time:", new_state->current_time, "\n",
                     new_state->variable_map);
  HYDLA_LOGGER_SUMMARY("#*** end point phase ***");

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

  MathematicaVCS vcs(MathematicaVCS::ContinuousMode, &ml_, opts_.approx_precision);
  /*vcs.set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&SymbolicSimulator::output, this, _1, _2));*/
  vcs.reset(state->variable_map);
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  //��v�Z
  if(!calculate_closure(ms, vcs,expanded_always,positive_asks,negative_asks)){
    return false;
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
  virtual_constraint_solver_t::IntegrateResult integrate_result;
  vcs.integrate(
    integrate_result,
    positive_asks,
    negative_asks,
    state->current_time,
    symbolic_time_t(opts_.max_time),
    not_adopted_tells_list);

  //�o��
  output_interval(vcs,
                  state->current_time,
                  integrate_result.states[0].time-state->current_time,
                  integrate_result.states[0].variable_map);



  //to next pointphase
  assert(integrate_result.states.size() == 1);

  if(!integrate_result.states[0].is_max_time) {
    phase_state_sptr new_state(create_new_phase_state());
    new_state->phase        = PointPhase;
    expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
    HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                       new_state->expanded_always_id);
    new_state->module_set_container = msc_no_init_;
    new_state->current_time = integrate_result.states[0].time;
    
    //���̃t�F�[�Y�ɂ�����ϐ��̒l�𓱏o����
    HYDLA_LOGGER_DEBUG("--- calc next phase variable map ---");  
    vcs.apply_time_to_vm(integrate_result.states[0].variable_map, new_state->variable_map, integrate_result.states[0].time-state->current_time);

    push_phase_state(new_state);
  }



  HYDLA_LOGGER_SUMMARY("%%%%%%%%%%%%% interval phase result  %%%%%%%%%%%%%\n",
                     "time:", integrate_result.states[0].time.get_real_val(ml_, 5), "\n",
                     integrate_result.states[0].variable_map);

/*
  std::cout << "%%%%%%%%%%%%% interval phase result %%%%%%%%%%%%% \n"
  << "time:" << integrate_result.states[0].time.get_real_val(ml_, 5) << "\n";

  variable_map_t::const_iterator it  = integrate_result.states[0].variable_map.begin();
  variable_map_t::const_iterator end = integrate_result.states[0].variable_map.end();
  for(; it!=end; ++it) {
  std::cout << it->first << "\t: "
  << it->second.get_real_val(ml_, 5) << "\n";
  }
*/

  return true;
}

void SymbolicSimulator::output_interval(MathematicaVCS &vcs, const symbolic_time_t& current_time, const symbolic_time_t& limit_time,
                                        const variable_map_t& variable_map){
  variable_map_t output_vm;
  symbolic_time_t elapsed_time;
  
  do{
    vcs.apply_time_to_vm(variable_map, output_vm, elapsed_time);
    output((elapsed_time+current_time),output_vm);
    elapsed_time += symbolic_time_t(opts_.output_interval);
  }while(elapsed_time.lessThan(ml_, limit_time));
  elapsed_time = limit_time;
  vcs.apply_time_to_vm(variable_map, output_vm, elapsed_time);
  output((elapsed_time+current_time),output_vm);

  std::cout << std::endl << std::endl;
}


void SymbolicSimulator::output(const symbolic_time_t& time, 
                           const variable_map_t& vm)
{
  
  
  if(opts_.output_style==styleList){
    output_buffer_ << time.get_real_val(ml_, opts_.output_precision) << "\t";

    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
       output_buffer_ << it->second.get_real_val(ml_, opts_.output_precision) << "\t";
    }
    output_buffer_ << std::endl;
  }else{
  //   std::cout << "$time\t: " << time.get_real_val(ml_, 5) << "\n";
    std::cout << time.get_real_val(ml_, opts_.output_precision) << "\t";

    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
  //     std::cout << it->first << "\t: "
  //               << it->second.get_real_val(ml_, 5) << "\n";
       std::cout << it->second.get_real_val(ml_, opts_.output_precision) << "\t";
    }
    std::cout << std::endl;
  }
}

} //namespace symbolic_simulator
} //namespace hydla

