#include "MathSimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include "mathlink_helper.h"
#include "math_source.h"
#include "HydLaParser.h"
#include "InterlanguageSender.h"
#include "CollectTellVisitor.h"

using namespace std;
using namespace boost;
using namespace boost::xpressive;

namespace hydla{
namespace symbolic_simulator {

MathSimulator::MathSimulator()
{
}

MathSimulator::~MathSimulator()
{
}

struct rawchar_formatter
{
  string operator()(smatch const &what) const
  {
    char c[2] = {0, 0};
    c[0] = (char)strtol(what.str(1).c_str(), NULL, 8);
    return c;
  }
};


bool MathSimulator::simulate(const char mathlink[], 
  HydLaParser& parser,
  bool debug_mode,
  std::string max_time,
  bool profile_mode,
  bool parallel_mode,
  OutputFormat output_format)
{
  MathLink ml;
  if(!ml.init(mathlink)) {
    std::cerr << "can not link" << std::endl;
    return false;
  }

  int p;
/*
  // 出力する画面の横幅の設定
  ml.MLPutFunction("SetOptions", 2);
  ml.MLPutSymbol("$Output"); 
  ml.MLPutFunction("Rule", 2);
  ml.MLPutSymbol("PageWidth"); 
  ml.MLPutSymbol("Infinity"); 
  ml.MLEndPacket();
  while ((p = ml.MLNextPacket()) && p != RETURNPKT) ml.MLNewPacket();

  // デバッグプリント
  ml.MLPutFunction("Set", 2);
  ml.MLPutSymbol("optUseDebugPrint"); 
  ml.MLPutSymbol(debug_mode ? "True" : "False");
  ml.MLEndPacket();
  while ((p = ml.MLNextPacket()) && p != RETURNPKT) ml.MLNewPacket();

  // プロファイルモード
  ml.MLPutFunction("Set", 2);
  ml.MLPutSymbol("optUseProfile"); 
  ml.MLPutSymbol(profile_mode ? "True" : "False");
  ml.MLEndPacket();
  while ((p = ml.MLNextPacket()) && p != RETURNPKT) ml.MLNewPacket();

  // 並列モード
  ml.MLPutFunction("Set", 2);
  ml.MLPutSymbol("optParallel"); 
  ml.MLPutSymbol(parallel_mode ? "True" : "False");
  ml.MLEndPacket();
  while ((p = ml.MLNextPacket()) && p != RETURNPKT) ml.MLNewPacket();

  // 出力形式
  ml.MLPutFunction("Set", 2);
  ml.MLPutSymbol("optOutputFormat"); 
  switch(output_format) {
  case fmtTFunction:
    ml.MLPutSymbol("fmtTFunction");
    break;

  case fmtNumeric:
  default:
    ml.MLPutSymbol("fmtNumeric");
    break;
  }
  ml.MLEndPacket();
  while ((p = ml.MLNextPacket()) && p != RETURNPKT) ml.MLNewPacket();

  //   ml.MLPutFunction("Get", 1);
  //   ml.MLPutString("symbolic_simulator/HydLa.m"); 
*/
  ml.MLPutFunction("ToExpression", 2);
  ml.MLPutString(math_source()); 
  ml.MLPutSymbol("InputForm"); 
  ml.MLEndPacket();
  while ((p = ml.MLNextPacket()) && p != RETURNPKT) ml.MLNewPacket();
  ml.MLNewPacket();

  //std::cout << math_source() << std::endl;

  InterlanguageSender interlanguage_sender(parser.parse_tree(), ml);
  interlanguage_sender.create_interlanguage(max_time);

  if(debug_mode) {
    std::cout << "#*** Interlanguage ***\n"; 
    std::cout << interlanguage_sender.get_interlanguage() << std::endl;

//     ml.MLPutFunction("Print", 1);
//     ml.MLPutFunction("InputForm", 1);
//     ml.MLPutFunction("Hold", 1);

//     ml.MLPutFunction("HydLaMain", 3);

//     InterlanguageSender interlanguage_sender(ml);
//     parser.dispatch(&interlanguage_sender);

//     ml.MLPutFunction("ToExpression", 1);
//     ml.MLPutString("{ht, v}");

//     ml.MLPutInteger(1);

  }

  // ml.MLPutFunction("ToExpression", 1);
  // ml.MLPutString(interlanguage_sender.get_interlanguage().c_str());
  
  CollectTellVisitor ctv(parser.parse_tree(), ml);
  ctv.is_consistent();


  ml.MLPutFunction("Exit", 0);
  ml.MLEndPacket();

  sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
  std::ostream_iterator< char > out_iter( std::cout );
  rawchar_formatter rfmt;

  int pkt;
  while((pkt = ml.MLNextPacket()) != ILLEGALPKT) {
    switch(pkt) 
    {
    case RETURNPKT:
      {
        int rpt = ml.MLGetType();
        switch(rpt) 
        {
        case MLTKSTR: 
          {
            const char *str;
            ml.MLGetString(&str);
            std::cout << "#string:" << str << std::endl;
            ml.MLReleaseString(str);
            break;
          }

        case MLTKSYM: 
          {
            const char *sym;
            ml.MLGetSymbol(&sym);
            //std::cout << "#symbol:" << sym << std::endl;
            ml.MLReleaseSymbol(sym);
            break;
          }

        case MLTKINT:
        case MLTKOLDINT: 
          {	  
            int q;
            ml.MLGetInteger(&q);
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
        ml.MLGetString(&s);
        string str(s);
        regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	
        std::cout << std::flush;
        ml.MLReleaseString(s);
        break;
      }

    case MESSAGEPKT: 
      {
        const char *sym, *tag;
        ml.MLGetSymbol(&sym);
        ml.MLGetString(&tag);
        std::cout << "#message:" << sym << ":" << tag << std::endl;
        ml.MLReleaseSymbol(sym);
        ml.MLReleaseString(tag);
        break;
      }

    case SYNTAXPKT: 
      {
        int q;
        ml.MLGetInteger(&q);
        std::cout << "#syntax:" << q << std::endl;
        break; 
      }

    case INPUTNAMEPKT: 
      {
        const char *name;
        ml.MLGetString(&name);
        //std::cout << "#inputname:" << name << std::endl;
        ml.MLReleaseString(name);
        break;
      }

    default:
      std::cout << "#unknown packet:" << pkt << std::endl;
    }
    ml.MLNewPacket();
  }

  return true;
}

} //namespace symbolic_simulator
} //namespace hydla

