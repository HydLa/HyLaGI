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

  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->current_time = symbolic_time_t();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;
  
  if(opts_.output.size()>0){
    output_dest= new std::ofstream(opts_.output.c_str());
    if(!(*output_dest)){
      std::cout << "Output-File can't be opend.";
      assert(0);
    }
  }else{
    output_dest=NULL;
  }
  // 変数のラベル
  // TODO: 未定義の値とかのせいでずれる可能性あり?
  std::cout << "# time\t";
  BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
    std::cout << i.first << "\t";
  }
  std::cout << std::endl;
  if(output_dest){
    (*output_dest) << "# time\t";
    BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
     (*output_dest) << i.first << "\t";
    }
   (*output_dest) << std::endl;
  }

  push_phase_state(state);

  step=0;
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

      // 最適化された形のパースツリーを得る
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
    ModuleSetContainerInitializer::init<ModuleSetGraph>(
      parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
  else {
    ModuleSetContainerInitializer::init<ModuleSetList>(
      parse_tree, msc_original_, msc_no_init_, parse_tree_);
  }
}


void SymbolicSimulator::init_mathlink()
{
  HYDLA_LOGGER_DEBUG("#*** init mathlink ***");

  //TODO: 例外を投げるようにする
  if(!ml_.init(opts_.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }

  // 出力する画面の横幅の設定
  ml_.MLPutFunction("SetOptions", 2);
  ml_.MLPutSymbol("$Output"); 
  ml_.MLPutFunction("Rule", 2);
  ml_.MLPutSymbol("PageWidth"); 
  ml_.MLPutSymbol("Infinity"); 
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // デバッグプリント
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseDebugPrint"); 
  ml_.MLPutSymbol(opts_.debug_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // プロファイルモード
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseProfile"); 
  ml_.MLPutSymbol(opts_.profile_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // 並列モード
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optParallel"); 
  ml_.MLPutSymbol(opts_.parallel_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // 出力形式
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

  // HydLa.mの内容送信
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
    //インタラクティブモード
    if(opts_.interactive_mode){
        state->module_set_container->dispatch(
        boost::bind(&SymbolicSimulator::judge_phase_state, 
                   this, _1, state));
      int selected_num=0;
      int vector_size=module_set_vector.size();
      if(--step<=0||vector_size>1){
        cout << endl;
        for(int i=0;i<vector_size;i++){
         ostringstream ost,ost2;
         cout << i << endl << "|Module\t\t:" << module_set_vector[i]->get_name() << endl;
         cout << "|Constraint\t: ";
         module_set_vector[i]->dump_infix(cout,"\n|                 ");
         cout << endl;
         positive_asks_t::iterator it=positive_asks_vector[i].begin();
         positive_asks_t::iterator end=positive_asks_vector[i].end();
         if(it!=end){
          cout << "|Positive Ask\t: ";
          (*it)->dump_infix(cout);
          it++;
          for(;it!=end;it++){
           cout << ".\n|                 ";
           (*it)->dump_infix(cout);
          }
         }
         else{
           cout << "|No Positive Ask" << endl;
         }
         cout << endl << endl;
        }
        cout << endl;

        if(state->phase==PointPhase){
         cout << "In PP(";
        }
        else{
         cout << "In IP(";
        }
        cout << "time:" << state->current_time.get_real_val(ml_, opts_.output_precision) << ")" << endl;
        
        string buf;

        if(vector_size>1){
          do{
           cout << "Select Module Set (" << "0~" << vector_size-1 << ")" << endl;
           cin >> buf;
           try{
             selected_num=lexical_cast<int>(buf);
           }catch(bad_lexical_cast e){
             cerr << "exeption: " << e.what() << endl;
             continue;
           }
           cout << endl;
          }while(selected_num<0||selected_num>=vector_size);
        }
        if(step<=0){
          do{
           cout << "How many steps?" << endl;
           cin >> buf;
           try{
             step=lexical_cast<int>(buf);
           }catch(bad_lexical_cast e){
             cerr << "exeption: " << e.what() << endl;
             continue;
           }
           cout << endl;
          }while(step==0);
        }
        if(step<0){
          while(++step<0&&track_back_stack.size()>1){
            track_back_stack.pop();
          }
          if(track_back_stack.size()>0){
            push_phase_state(track_back_stack.top());
            track_back_stack.pop();
          }else{
            push_phase_state(state);
          }
          step=0;
          module_set_vector.clear();
          positive_asks_vector.clear();
          negative_asks_vector.clear();
          expanded_always_vector.clear();
          tell_vector.clear();
          continue;
        }
        std::cout << "# time\t";
        BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
         std::cout << i.first << "\t";
        }
      std::cout << std::endl;
      }
      switch(state->phase) 
      {
        case PointPhase:
        do_point_phase(module_set_vector[selected_num],state, tell_vector[selected_num], expanded_always_vector[selected_num]);
        //point_phase(module_set_vector[selected_num],state);
        break;

        case IntervalPhase: 
        do_interval_phase(module_set_vector[selected_num],state, tell_vector[selected_num],positive_asks_vector[selected_num],negative_asks_vector[selected_num], expanded_always_vector[selected_num]);
        //interval_phase(module_set_vector[selected_num],state);
        break;
      
        default:
        assert(0);
      }
      module_set_vector.clear();
      positive_asks_vector.clear();
      negative_asks_vector.clear();
      expanded_always_vector.clear();
      tell_vector.clear();
      track_back_stack.push(state);
    }
    else{
      state->module_set_container->dispatch(
      boost::bind(&SymbolicSimulator::simulate_phase_state, 
                 this, _1, state));
    }
  }
}




bool SymbolicSimulator::judge_phase_state(const module_set_sptr& ms, 
                                  const phase_state_const_sptr& state)
{
  MathematicaVCS *vcs;

  TellCollector tell_collector(ms);

  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
                              AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
                              AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);

  tells_t         tell_list;
  positive_asks_t   positive_asks;
  negative_asks_t   negative_asks;
  
  
  expanded_always_t expanded_always;

  switch(state->phase) 
  {
  case PointPhase:
    {        
      HYDLA_LOGGER_DEBUG(
        "#***** begin point phase *****",
        "\n#** module set **\n",
        ms->get_name(),
        "\n",
        *ms,
        "\n#*** variable map ***\n",
        variable_map_);
      vcs=new MathematicaVCS(MathematicaVCS::DiscreteMode, &ml_, opts_.approx_precision);
      vcs->set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&SymbolicSimulator::output, this, _1, _2));
      
      if(state->changed_asks.size() != 0) {
        HYDLA_LOGGER_DEBUG("#** point_phase: changed_ask_id: ",
                          state->changed_asks.at(0).second,
                          " **");
      }
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);
      HYDLA_LOGGER_DEBUG("#** point_phase: expanded always from IP: **\n",
                     expanded_always);
      break;
    }

  case IntervalPhase: 
    {        
      HYDLA_LOGGER_DEBUG(
        "#***** begin interval phase *****",
        "\n#** module set **\n",
        ms->get_name(),
        "\n",
        *ms,
        "\n#*** variable map ***\n",
        variable_map_);
      vcs=new MathematicaVCS(MathematicaVCS::ContinuousMode, &ml_, opts_.approx_precision);
      vcs->set_output_func(symbolic_time_t(opts_.output_interval), 
                            boost::bind(&SymbolicSimulator::output, this, _1, _2));
      expanded_always_id2sptr(state->expanded_always_id, expanded_always);
      HYDLA_LOGGER_DEBUG("#** interval_phase: expanded always from PP: **\n",
                   expanded_always);
      break;
    }

  default:
    assert(0);
  }



  
  vcs->reset(state->variable_map);

  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    // 制約を追加し，制約ストアが矛盾をおこしていないかどうか
    switch(vcs->add_constraint(tell_list)) 
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
        // TODO: 例外とかなげたり、BPシミュレータに移行したり
        assert(0);
        break;
    }

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always, 
                              &positive_asks, 
                              &negative_asks);

    // ask制約のエンテール処理
    expanded = false;
    negative_asks_t::iterator it  = negative_asks.begin();
    negative_asks_t::iterator end = negative_asks.end();
    while(it!=end) {
      switch(vcs->check_entailment(*it))
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
          // TODO: 例外とかなげたり、BPシミュレータに移行したり
          assert(0);
          break;
      }
    }
  }
  module_set_vector.push_back(ms);
  positive_asks_vector.push_back(positive_asks);
  negative_asks_vector.push_back(negative_asks);
  tell_vector.push_back(tell_list);
  expanded_always_vector.push_back(expanded_always);
  return true;
}


bool SymbolicSimulator::do_interval_phase(const module_set_sptr& ms, 
                                   const phase_state_const_sptr& state,const tells_t& tell_list,const positive_asks_t& positive_asks, const negative_asks_t& negative_asks, const expanded_always_t& expanded_always)
{
  MathematicaVCS vcs(MathematicaVCS::ContinuousMode, &ml_, opts_.approx_precision);
  vcs.set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&SymbolicSimulator::output, this, _1, _2));
  vcs.reset(state->variable_map);
  vcs.add_constraint(tell_list);
  
  // MaxModuleの導出
  module_set_sptr max_module_set = (*msc_no_init_).get_max_module_set();
  HYDLA_LOGGER_DEBUG("#** interval_phase: ms: **\n",
                     *ms,
                     "\n#** interval_phase: max_module_set: ##\n",
                     *max_module_set);


  // 採用していないモジュールのリスト導出
  hydla::ch::ModuleSet::module_list_t diff_module_list(max_module_set->size() - ms->size());

  std::set_difference(
    max_module_set->begin(),
    max_module_set->end(),
    ms->begin(),
    ms->end(),
    diff_module_list.begin());


  // それぞれのモジュールをsingletonなモジュール集合とする
  std::vector<module_set_sptr> diff_module_set_list;

  hydla::ch::ModuleSet::module_list_const_iterator diff_it = diff_module_list.begin();
  hydla::ch::ModuleSet::module_list_const_iterator diff_end = diff_module_list.end();
  for(; diff_it!=diff_end; ++diff_it){
    module_set_sptr diff_ms(new ModuleSet((*diff_it).first, (*diff_it).second));
    diff_module_set_list.push_back(diff_ms);
  }

  assert(diff_module_list.size() == diff_module_set_list.size());


  // diff_module_set_list内の各モジュール集合内にある条件なし制約をそれぞれ得る
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

  
  // askの導出状態が変化するまで積分をおこなう
  virtual_constraint_solver_t::IntegrateResult integrate_result;
  vcs.integrate(
    integrate_result,
    positive_asks,
    negative_asks,
    state->current_time,
    symbolic_time_t(opts_.max_time),
    not_adopted_tells_list);
  
  //to next pointphase
  assert(integrate_result.states.size() == 1);

  if(!integrate_result.states[0].is_max_time) {
    phase_state_sptr new_state(create_new_phase_state());
    new_state->phase        = PointPhase;
    expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
    HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                       new_state->expanded_always_id);
    new_state->variable_map = integrate_result.states[0].variable_map;
    new_state->module_set_container = msc_no_init_;
    new_state->current_time = integrate_result.states[0].time;

    push_phase_state(new_state);
  }


  HYDLA_LOGGER_DEBUG("%%%%%%%%%%%%% interval phase result  %%%%%%%%%%%%%\n",
                     "time:", integrate_result.states[0].time.get_real_val(ml_, 5), "\n",
                     integrate_result.states[0].variable_map);
  return true;
}

bool SymbolicSimulator::do_point_phase(const module_set_sptr& ms, 
                                const phase_state_const_sptr& state,tells_t tell_list, const expanded_always_t& expanded_always)
{
  MathematicaVCS vcs(MathematicaVCS::DiscreteMode, &ml_, opts_.approx_precision);
  vcs.reset(state->variable_map);
  vcs.add_constraint(tell_list);
  
  // Interval Phaseへ移行
  HYDLA_LOGGER_DEBUG("#*** create new phase state ***");
  phase_state_sptr new_state(create_new_phase_state());
  new_state->phase        = IntervalPhase;
  new_state->current_time = state->current_time;
  expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                     new_state->expanded_always_id);
  new_state->module_set_container = msc_no_init_;


  {
    // 暫定的なフレーム公理の処理
    // 未定義の値や変数表に存在しない場合は以前の値をコピー
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
  output(new_state->current_time, new_state->variable_map);
  push_phase_state(new_state);

  HYDLA_LOGGER_SUMMARY("%%%%%%%%%%%%% point phase result  %%%%%%%%%%%%%\n",
                     "time:", new_state->current_time, "\n",
                     new_state->variable_map);

  HYDLA_LOGGER_SUMMARY("#*** end point phase ***");


  return true;
}


bool SymbolicSimulator::point_phase(const module_set_sptr& ms, 
                                const phase_state_const_sptr& state)
{
  TellCollector tell_collector(ms);

  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
                              AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
                              AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);

  tells_t         tell_list;
  positive_asks_t   positive_asks;
  negative_asks_t   negative_asks;

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
  vcs.set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&SymbolicSimulator::output, this, _1, _2));
  vcs.reset(state->variable_map);

  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_new_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    // 制約を追加し，制約ストアが矛盾をおこしていないかどうか
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
        // TODO: 例外とかなげたり、BPシミュレータに移行したり
        assert(0);
        break;
    }

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always, 
                              &positive_asks, 
                              &negative_asks);

    // ask制約のエンテール処理
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
          // TODO: 例外とかなげたり、BPシミュレータに移行したり
          assert(0);
          break;
      }
    }
  }

  
  // Interval Phaseへ移行
  HYDLA_LOGGER_DEBUG("#*** create new phase state ***");
  phase_state_sptr new_state(create_new_phase_state());
  new_state->phase        = IntervalPhase;
  new_state->current_time = state->current_time;
  expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                     new_state->expanded_always_id);
  new_state->module_set_container = msc_no_init_;
  

  {
    // 暫定的なフレーム公理の処理
    // 未定義の値や変数表に存在しない場合は以前の値をコピー
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

  output(new_state->current_time, new_state->variable_map);

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

  TellCollector tell_collector(ms);

  AskCollector  ask_collector(ms);

  tells_t         tell_list;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  expanded_always_t expanded_always;
  expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  HYDLA_LOGGER_DEBUG("#** interval_phase: expanded always from PP: **\n",
                     expanded_always);

  MathematicaVCS vcs(MathematicaVCS::ContinuousMode, &ml_, opts_.approx_precision);
  vcs.set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&SymbolicSimulator::output, this, _1, _2));
  vcs.reset(state->variable_map);

  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_all_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    // 制約を追加し，制約ストアが矛盾をおこしていないかどうか
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
        // TODO: 例外とかなげたり、BPシミュレータに移行したり
        assert(0);
        break;
    }

    // ask制約を集める
    ask_collector.collect_ask(&expanded_always, 
                              &positive_asks, 
                              &negative_asks);

    // ask制約のエンテール処理
    expanded = false;
    {
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
        if(boost::dynamic_pointer_cast<DiscreteAsk>(*it)) {
          it++;
        }
        else {
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
              // TODO: 例外とかなげたり、BPシミュレータに移行したり
              assert(0);
              break;
          }
        }
      }
    }
  }

  // MaxModuleの導出
  module_set_sptr max_module_set = (*msc_no_init_).get_max_module_set();
  HYDLA_LOGGER_DEBUG("#** interval_phase: ms: **\n",
                     *ms,
                     "\n#** interval_phase: max_module_set: ##\n",
                     *max_module_set);


  // 採用していないモジュールのリスト導出
  hydla::ch::ModuleSet::module_list_t diff_module_list(max_module_set->size() - ms->size());

  std::set_difference(
    max_module_set->begin(),
    max_module_set->end(),
    ms->begin(),
    ms->end(),
    diff_module_list.begin());


  // それぞれのモジュールをsingletonなモジュール集合とする
  std::vector<module_set_sptr> diff_module_set_list;

  hydla::ch::ModuleSet::module_list_const_iterator diff_it = diff_module_list.begin();
  hydla::ch::ModuleSet::module_list_const_iterator diff_end = diff_module_list.end();
  for(; diff_it!=diff_end; ++diff_it){
    module_set_sptr diff_ms(new ModuleSet((*diff_it).first, (*diff_it).second));
    diff_module_set_list.push_back(diff_ms);
  }

  assert(diff_module_list.size() == diff_module_set_list.size());


  // diff_module_set_list内の各モジュール集合内にある条件なし制約をそれぞれ得る
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

  
  // askの導出状態が変化するまで積分をおこなう
  virtual_constraint_solver_t::IntegrateResult integrate_result;
  vcs.integrate(
    integrate_result,
    positive_asks,
    negative_asks,
    state->current_time,
    symbolic_time_t(opts_.max_time),
    not_adopted_tells_list);
  
  //to next pointphase
  assert(integrate_result.states.size() == 1);

  if(!integrate_result.states[0].is_max_time) {
    phase_state_sptr new_state(create_new_phase_state());
    new_state->phase        = PointPhase;
    expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
    HYDLA_LOGGER_DEBUG("--- expanded always ID ---\n",
                       new_state->expanded_always_id);
    new_state->variable_map = integrate_result.states[0].variable_map;
    new_state->module_set_container = msc_no_init_;
    new_state->current_time = integrate_result.states[0].time;

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

void SymbolicSimulator::output(const symbolic_time_t& time, 
                           const variable_map_t& vm)
{
  if(output_dest){
    (*output_dest) << time.get_real_val(ml_, opts_.output_precision) << "\t";

    variable_map_t::const_iterator it  = vm.begin();
    variable_map_t::const_iterator end = vm.end();
    for(; it!=end; ++it) {
       (*output_dest) << it->second.get_real_val(ml_, opts_.output_precision) << "\t";
    }
    (*output_dest) << std::endl;
  }
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

} //namespace symbolic_simulator
} //namespace hydla

