#include "MathSimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "math_source.h"
//#include "InterlanguageSender.h"
#include "ConsistencyChecker.h"
#include "EntailmentChecker.h"
#include "ConstraintStoreBuilderPoint.h"

#include "TellCollector.h"
#include "AskCollector.h"

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::ch;
using namespace hydla::simulator;

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
  simulator_t(opts.debug_mode),
  opts_(opts)
{
}

MathSimulator::~MathSimulator()
{
}

void MathSimulator::do_initialize()
{
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
  TellCollector tell_collector(ms, is_debug_mode());
  AskCollector  ask_collector(ms, is_debug_mode());
  ConstraintStoreBuilderPoint csbp;
  csbp.build_constraint_store();
  ConsistencyChecker consistency_checker(ml_);
  EntailmentChecker  entailment_checker(ml_);

  tells_t         tell_list;
  positive_asks_t positive_asks;
  negative_asks_t negative_asks;
  
  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_all_tells(&tell_list,
                                     &state->expanded_always, 
                                     &positive_asks);

    // 制約が充足しているかどうかの確認
    if(!consistency_checker.is_consistent(tell_list, csbp.getcs())){
      return false;
    }

    // ask制約を集める
    ask_collector.collect_ask(&state->expanded_always, 
                              &positive_asks, 
                              &negative_asks);

    // ask制約のエンテール処理
    expanded = false;
    {
      negative_asks_t::iterator it  = negative_asks.begin();
      negative_asks_t::iterator end = negative_asks.end();
      while(it!=end) {
        if(entailment_checker.check_entailment(*it, csbp.getcs())) {
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
  phase_state_sptr new_state(create_new_phase_state(state));
  new_state->phase        = phase_state_t::IntervalPhase;
  new_state->initial_time = false;
  //state->variable_map
  push_phase_state(new_state);

  return true;
}

bool MathSimulator::interval_phase(const module_set_sptr& ms, 
                                   const phase_state_const_sptr& state)
{

  return true;
}

} //namespace symbolic_simulator
} //namespace hydla

