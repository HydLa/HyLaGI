#include "MathSimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"

#include "math_source.h"

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

MathSimulator::MathSimulator(const Opts& opts) :
  opts_(opts)
//  vcs_(MathematicaVCS::DiscreteMode, &ml_)
{
}

MathSimulator::~MathSimulator()
{
}

void MathSimulator::do_initialize(const parse_tree_sptr& parse_tree)
{
  init_module_set_container(parse_tree);

  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->current_time = symbolic_time_t();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;

  // 変数のラベル
  // TODO: 未定義の値とかのせいでずれる可能性あり?
  std::cout << "# time\t";
  BOOST_FOREACH(variable_map_t::value_type& i, variable_map_) {
    std::cout << i.first << "\t";
  }
  std::cout << std::endl;

  push_phase_state(state);

  init_mathlink();
}

void MathSimulator::init_module_set_container(const parse_tree_sptr& parse_tree)
{  
  HYDLA_LOGGER_DEBUG("#*** create module set list ***");

  ModuleSetContainerCreator<ModuleSetGraph> mcc;
  {
    parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
    AskTypeAnalyzer().analyze(pt_original.get());
    AskDisjunctionFormatter().format(pt_original.get());
    AskDisjunctionSplitter().split(pt_original.get());
    msc_original_ = mcc.create(pt_original);
  }

  {
    parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init.get());
    AskTypeAnalyzer().analyze(pt_no_init.get());
    AskDisjunctionFormatter().format(pt_no_init.get());
    AskDisjunctionSplitter().split(pt_no_init.get());
    msc_no_init_ = mcc.create(pt_no_init);
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


void MathSimulator::init_mathlink()
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
  ml_.MLPutString(math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
}

bool MathSimulator::point_phase(const module_set_sptr& ms, 
                                const phase_state_const_sptr& state)
{
  TellCollector tell_collector(ms);

  AskCollector  ask_collector(ms, AskCollector::ENABLE_COLLECT_NON_TYPED_ASK | 
                              AskCollector::ENABLE_COLLECT_DISCRETE_ASK |
                              AskCollector::ENABLE_COLLECT_CONTINUOUS_ASK);

  tells_t         tell_list;
  positive_asks_t   positive_asks;
  negative_asks_t   negative_asks;


  expanded_always_t expanded_always;
  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);

  
  MathematicaVCS vcs(MathematicaVCS::DiscreteMode, &ml_);
  vcs.set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&MathSimulator::output, this, _1, _2));
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
  //expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
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

  HYDLA_LOGGER_DEBUG("%%%%%%%%%%%%% point phase result  %%%%%%%%%%%%%\n",
                     "time:", new_state->current_time, "\n",
                     new_state->variable_map);

  HYDLA_LOGGER_DEBUG("#*** end point phase ***");


  return true;
}

bool MathSimulator::interval_phase(const module_set_sptr& ms, 
                                   const phase_state_const_sptr& state)
{

  TellCollector tell_collector(ms);

  AskCollector  ask_collector(ms);

  tells_t         tell_list;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;

  expanded_always_t expanded_always;
  //expanded_always_id2sptr(state->expanded_always_id, expanded_always);

  MathematicaVCS vcs(MathematicaVCS::ContinuousMode, &ml_);
  vcs.set_output_func(symbolic_time_t(opts_.output_interval), 
                      boost::bind(&MathSimulator::output, this, _1, _2));
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
  
  // askの導出状態が変化するまで積分をおこなう
  virtual_constraint_solver_t::IntegrateResult integrate_result;
  vcs.integrate(
    integrate_result,
    positive_asks,
    negative_asks,
    state->current_time,
    symbolic_time_t(opts_.max_time));
  
  //to next pointphase
  assert(integrate_result.states.size() == 1);

  if(!integrate_result.states[0].is_max_time) {
    phase_state_sptr new_state(create_new_phase_state());
    new_state->phase        = PointPhase;
    new_state->variable_map = integrate_result.states[0].variable_map;
    new_state->module_set_container = msc_no_init_;
    new_state->current_time = integrate_result.states[0].time;

    push_phase_state(new_state);
  }


  HYDLA_LOGGER_DEBUG("%%%%%%%%%%%%% interval phase result  %%%%%%%%%%%%%\n",
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

void MathSimulator::output(const symbolic_time_t& time, 
                           const variable_map_t& vm)
{
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

