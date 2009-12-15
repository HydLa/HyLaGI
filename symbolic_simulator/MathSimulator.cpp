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

struct rawchar_formatter
{
  string operator()(smatch const &what) const
  {
    char c[2] = {0, 0};
    c[0] = (char)strtol(what.str(1).c_str(), NULL, 8);
    return c;
  }
};

MathSimulator::MathSimulator()
{
}

MathSimulator::~MathSimulator()
{
}

void MathSimulator::init(Opts& opts)
{
  if(!ml_.init(opts.mathlink.c_str())) {
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
  ml_.MLPutSymbol(opts.debug_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // プロファイルモード
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optUseProfile"); 
  ml_.MLPutSymbol(opts.profile_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // 並列モード
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optParallel"); 
  ml_.MLPutSymbol(opts.parallel_mode ? "True" : "False");
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();

  // 出力形式
  ml_.MLPutFunction("Set", 2);
  ml_.MLPutSymbol("optOutputFormat"); 
  switch(opts.output_format) {
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

bool MathSimulator::simulate(
  boost::shared_ptr<hydla::ch::ModuleSetContainer> msc,
  boost::shared_ptr<hydla::parse_tree::ParseTree> pt,
  Opts& opts)
{
  debug_mode_ = opts.debug_mode;

  init(opts);

  //module_set_container_ = msc;

  // 中間言語送信
  /*
  InterlanguageSender interlanguage_sender(parser.parse_tree(), ml_);
  interlanguage_sender.create_interlanguage(opts.max_time);

  if(opts.debug_mode) {
    std::cout << "#*** Interlanguage ***\n"; 
    std::cout << interlanguage_sender.get_interlanguage() << std::endl;
  }

  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(interlanguage_sender.get_interlanguage().c_str());
  */


  simulator_t::simulate(msc, pt);

  ml_.MLPutFunction("Exit", 0);
  ml_.MLEndPacket();

  sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
  std::ostream_iterator< char > out_iter( std::cout );
  rawchar_formatter rfmt;

  int pkt;
  while((pkt = ml_.MLNextPacket()) != ILLEGALPKT) {
    switch(pkt) 
    {
      case RETURNPKT:
      {
        int rpt = ml_.MLGetType();
        switch(rpt) 
        {
          case MLTKSTR: 
          {
            const char *str;
            ml_.MLGetString(&str);
            std::cout << "#string:" << str << std::endl;
            ml_.MLReleaseString(str);
            break;
          }

          case MLTKSYM: 
          {
            const char *sym;
            ml_.MLGetSymbol(&sym);
            //std::cout << "#symbol:" << sym << std::endl;
            ml_.MLReleaseSymbol(sym);
            break;
          }

          case MLTKINT:
          case MLTKOLDINT: 
          {	  
            int q;
            ml_.MLGetInteger(&q);
            std::cout << "#integer:" << q << std::endl;
            break;
          }
          case MLTKERR:
            std::cout << "#error type" << std::endl;
            break;

          case MLTKFUNC:
            break;

          default:
            std::cout << "#unknown type:" << rpt << std::endl;
        }
        break;
      }

      case TEXTPKT: 
      {
        const char *s;
        ml_.MLGetString(&s);
        string str(s);
        regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	
        std::cout << std::flush;
        ml_.MLReleaseString(s);
        break;
      }

      case MESSAGEPKT: 
      {
        const char *sym, *tag;
        ml_.MLGetSymbol(&sym);
        ml_.MLGetString(&tag);
        std::cout << "#message:" << sym << ":" << tag << std::endl;
        ml_.MLReleaseSymbol(sym);
        ml_.MLReleaseString(tag);
        break;
      }

      case SYNTAXPKT: 
      {
        int q;
        ml_.MLGetInteger(&q);
        std::cout << "#syntax:" << q << std::endl;
        break; 
      }

      case INPUTNAMEPKT: 
      {
        const char *name;
        ml_.MLGetString(&name);
        //std::cout << "#inputname:" << name << std::endl;
        ml_.MLReleaseString(name);
        break;
      }

      default:
        std::cout << "#unknown packet:" << pkt << std::endl;
    }
    ml_.MLNewPacket();
  }

  return true;
}

namespace {
class NodeDump {
public:
  template<typename T>
  void operator()(T& it) 
  {
    std::cout << *it << "\n";
  }
};
}

bool MathSimulator::point_phase(hydla::ch::module_set_sptr& ms, 
                                phase_state_sptr& state)
{
  if(debug_mode_) {
    std::cout << "#***** bagin point phase *****\n";
    std::cout << "#** module set **\n";
    std::cout << ms->get_name() << std::endl;
    ms->dump_tree(std::cout) << std::endl;
  }

  TellCollector tell_collector(ms);
  AskCollector  ask_collector;
  ConstraintStoreBuilderPoint csbp;       //TODO: kenshiroが作成
  csbp.build_constraint_store();
  ConsistencyChecker consistency_checker(ml_);
  EntailmentChecker entailment_checker(ml_);

  TellCollector::tells_t tell_list;
  positive_asks_t   positive_asks;
  negative_asks_t   negative_asks;
  
  bool expanded   = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_all_tells(&tell_list,
                                     &state->expanded_always, 
                                     &positive_asks);
    if(debug_mode_) {
      std::cout << "#** collected tells **\n";  
      std::for_each(tell_list.begin(), tell_list.end(), NodeDump());
    }

    // 制約が充足しているかどうかの確認
    if(!consistency_checker.is_consistent(tell_list, csbp.getcs())){
      if(debug_mode_) std::cout << "#*** inconsistent\n";
      return false;
    }
    if(debug_mode_) std::cout << "#*** consistent\n";

    // ask制約を集める
    ask_collector.collect_ask(ms.get(), &state->expanded_always, 
                              &positive_asks, &negative_asks);
    if(debug_mode_) {
      std::cout << "#** positive asks **\n";  
      std::for_each(positive_asks.begin(), positive_asks.end(), NodeDump());

      std::cout << "#** negative asks **\n";  
      std::for_each(negative_asks.begin(), negative_asks.end(), NodeDump());
    }

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
    
//   if(!csbp.build_constraint_store(&new_tells, &state->constraint_store)) {
//     return false;
//   }

//  state->phase = IntervalPhase;
  //state_queue_.push(*state);

  return true;
}

bool MathSimulator::interval_phase(hydla::ch::module_set_sptr& ms, 
                                   phase_state_sptr& state)
{

  return true;
}

} //namespace symbolic_simulator
} //namespace hydla

