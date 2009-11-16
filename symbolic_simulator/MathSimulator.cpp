#include "MathSimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>

#include "math_source.h"
#include "HydLaParser.h"
#include "InterlanguageSender.h"
#include "CollectTellVisitor.h"

#include "TellCollector.h"

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
  ml_.MLPutFunction("ToExpression", 2);
  ml_.MLPutString(math_source()); 
  ml_.MLPutSymbol("InputForm"); 
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
}

bool MathSimulator::simulate(
  HydLaParser& parser,
  boost::shared_ptr<hydla::ch::ModuleSetContainer> msc,
  Opts& opts)
{
  debug_mode_ = opts.debug_mode;

  init(opts);

  //module_set_container_ = msc;

  // 中間言語送信
  InterlanguageSender interlanguage_sender(parser.parse_tree(), ml_);
  interlanguage_sender.create_interlanguage(opts.max_time);

  if(opts.debug_mode) {
    std::cout << "#*** Interlanguage ***\n"; 
    std::cout << interlanguage_sender.get_interlanguage() << std::endl;
  }

  //ml_.MLPutFunction("ToExpression", 1);
  //ml_.MLPutString(interlanguage_sender.get_interlanguage().c_str());

  simulator_t::simulate(msc);

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

bool MathSimulator::point_phase(hydla::ch::module_set_sptr& ms, State* state)
{
  std::cout << ms->get_name() << std::endl;
  std::cout << ms->get_tree_dump() << std::endl;

  TellCollector tell_collector;
  //AskCollector  ask_collector;
  //ConstraintStoreBuilderPoint csbp(ml_); //TODO: kenshiroが作成

  std::vector<boost::shared_ptr<hydla::parse_tree::Tell> > new_tells;
  std::set<boost::shared_ptr<hydla::parse_tree::Tell> >    collected_tells;
  std::set<boost::shared_ptr<hydla::parse_tree::Ask> >     entailed_asks;
  
  bool expanded = true;
  while(expanded) {
    // tell制約を集める
    tell_collector.collect_tell(ms, &new_tells, &collected_tells, &entailed_asks);
    if(debug_mode_) {
      std::cout << "#** new tells **\n";
      std::for_each(new_tells.begin(), new_tells.end(), NodeDump());

      std::cout << "#** collected tells **\n";  
      std::for_each(collected_tells.begin(), collected_tells.end(), NodeDump());
              
      std::cout << "#** entailed asks **\n";  
      std::for_each(entailed_asks.begin(), entailed_asks.end(), NodeDump());
    }

    // 制約ストア構築
    //TODO: kenshiroが作成
    /*
    if(!csbp.build_constraint_store(&new_tells, &state->constraint_store)) {
      return false;
    }
    */
    if(debug_mode_) {
      std::cout << "#** constraint store **\n";  
      state->constraint_store.dump(std::cout);
    }
    

    // ask制約を集める


    // ask制約のエンテール処理
    expanded = false;
  }

  state->phase = IntervalPhase;
  //state_queue_.push(*state);

  return true;
}

bool MathSimulator::interval_phase(hydla::ch::module_set_sptr& ms, State* state)
{

  return true;
}

} //namespace symbolic_simulator
} //namespace hydla

