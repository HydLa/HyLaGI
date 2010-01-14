#include "MathSimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "math_source.h"
//#include "InterlanguageSender.h"
#include "ConsistencyChecker.h"
#include "EntailmentChecker.h"
#include "ConstraintStoreBuilderPoint.h"

#include "TellCollector.h"
#include "AskCollector.h"

#include "ModuleSetList.h"
#include "ModuleSetGraph.h"
#include "ModuleSetContainerCreator.h"
#include "InitNodeRemover.h"
#include "AskDisjunctionSplitter.h"
#include "AskDisjunctionFormatter.h"
#include "DiscreteAskRemover.h"

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
{
}

MathSimulator::~MathSimulator()
{
}

void MathSimulator::do_initialize(const parse_tree_sptr& parse_tree)
{
  ModuleSetContainerCreator<ModuleSetList> mcc;

  {
    parse_tree_sptr pt_original(boost::make_shared<ParseTree>(*parse_tree));
    AskDisjunctionFormatter().format(pt_original.get());
    AskDisjunctionSplitter().split(pt_original.get());
    msc_original_ = mcc.create(pt_original);
  }

  {
    parse_tree_sptr pt_no_init(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init.get());
    AskDisjunctionFormatter().format(pt_no_init.get());
    AskDisjunctionSplitter().split(pt_no_init.get());
    msc_no_init_ = mcc.create(pt_no_init);
  }

  {
    parse_tree_sptr pt_no_init_discreteask(boost::make_shared<ParseTree>(*parse_tree));
    InitNodeRemover().apply(pt_no_init_discreteask.get());
    DiscreteAskRemover().apply(pt_no_init_discreteask.get());
    AskDisjunctionFormatter().format(pt_no_init_discreteask.get());
    AskDisjunctionSplitter().split(pt_no_init_discreteask.get());
    msc_no_init_discreteask_ = mcc.create(pt_no_init_discreteask);
  }

  phase_state_sptr state(create_new_phase_state());
  state->phase        = PointPhase;
  state->time         = SymbolicTime();
  state->variable_map = variable_map_;
  state->module_set_container = msc_original_;

  push_phase_state(state);

  init_mathlink();

}

void MathSimulator::init_mathlink()
{
  //TODO: 例外を投げるようにする
  if(!ml_.init(opts_.mathlink.c_str())) {
    std::cerr << "can not link" << std::endl;
    exit(-1);
  }

  // 出力する画面の横幅の設定
  //ml_.MLPutFunction("SetOptions", 2);
  //ml_.MLPutSymbol("$Output"); 
  //ml_.MLPutFunction("Rule", 2);
  //ml_.MLPutSymbol("PageWidth"); 
  //ml_.MLPutSymbol("Infinity"); 
  //ml_.MLEndPacket();
  //ml_.skip_pkt_until(RETURNPKT);
  //ml_.MLNewPacket();

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
  AskCollector  ask_collector(ms);
  ConstraintStoreBuilderPoint csbp(ml_);
  variable_map_t vm;
  csbp.build_constraint_store(vm);
  ConsistencyChecker consistency_checker(ml_);
  EntailmentChecker  entailment_checker(ml_);

  tells_t         tell_list;
  positive_asks_t   positive_asks;
  negative_asks_t   negative_asks;


  expanded_always_t expanded_always;
  expanded_always_id2sptr(state->expanded_always_id, expanded_always);
  
  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_all_tells(&tell_list,
                                     &expanded_always, 
                                     &positive_asks);

    // 制約が充足しているかどうかの確認
    if(!consistency_checker.is_consistent(tell_list, csbp.get_constraint_store())){
      return false;
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
        if(entailment_checker.check_entailment(*it, csbp.get_constraint_store())) {
          expanded = true;
          positive_asks.insert(*it);
          negative_asks.erase(it++);
        }
        else {
          it++;
        }
      }
    }
  }

  // Interval Phaseへ移行
  phase_state_sptr new_state(create_new_phase_state());
  new_state->phase        = IntervalPhase;
  expanded_always_sptr2id(expanded_always, new_state->expanded_always_id);
  csbp.build_variable_map(new_state->variable_map);
  new_state->module_set_container = msc_no_init_discreteask_;

  push_phase_state(new_state);

  std::cout << new_state->variable_map;

  return true;
}

bool MathSimulator::interval_phase(const module_set_sptr& ms, 
                                   const phase_state_const_sptr& state)
{

  return true;
}

} //namespace symbolic_simulator
} //namespace hydla

