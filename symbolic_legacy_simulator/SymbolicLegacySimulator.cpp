#include "SymbolicLegacySimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "Logger.h"
#include "IntermediateCodeGenerator.h"
#include "sls_math_source.h"

using namespace std;
using namespace boost;
using namespace boost::xpressive;

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_legacy_simulator {

namespace {

struct rawchar_formatter
{
 string operator()(smatch const &what) const
 {
   char c[2] = {0, 0};
   c[0] = (char)strtol(what.str(1).c_str(), NULL, 8);
   return c;
 }
};

}

//sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
//std::ostream_iterator< char > out_iter( std::cout );
//rawchar_formatter rfmt;
//regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	

SymbolicLegacySimulator::SymbolicLegacySimulator(const Opts& opts) :
  opts_(opts)
{
}

SymbolicLegacySimulator::~SymbolicLegacySimulator()
{
}

void SymbolicLegacySimulator::init_mathlink()
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

  // sls_math_sourceの内容送信
  ml_.MLPutFunction("ToExpression", 1);
  ml_.MLPutString(sls_math_source());  
  ml_.MLEndPacket();
  ml_.skip_pkt_until(RETURNPKT);
  ml_.MLNewPacket();
}

void SymbolicLegacySimulator::initialize(const parse_tree_sptr& parse_tree)
{
  pt_ = parse_tree;
  init_mathlink();
  
}

void SymbolicLegacySimulator::simulate()
{
  ml_.MLPutFunction("ToExpression", 1);
  IntermediateCodeGenerator icg;
  ml_.put_string(icg.create(pt_, opts_.max_time));
  ml_.MLPutFunction("Exit", 0);
  ml_.MLEndPacket();

  sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
  std::ostream_iterator<char> out_iter(std::cout);
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
//             const char *str;
//             ml_.MLGetString(&str);
//             std::cout << "#string:" << str << std::endl;
//             ml_.MLReleaseString(str);
            break;
          }

        case MLTKSYM: 
          {
//             const char *sym;
//             ml_.MLGetSymbol(&sym);
//             std::cout << "#symbol:" << sym << std::endl;
//             ml_.MLReleaseSymbol(sym);
            break;
          }

        case MLTKINT:
        case MLTKOLDINT: 
          {	  
//             int q;
//             ml_.MLGetInteger(&q);
//             std::cout << "#integer:" << q << std::endl;
            break;
          }
        case MLTKERR:
          std::cerr << "#error type" << std::endl;
          break;

        case MLTKFUNC:
          break;

        default:
          std::cerr << "#unknown type:" << rpt << std::endl;
        }
        break;
      }

    case TEXTPKT: 
      {
        std::string str(ml_.get_string());
        regex_replace(out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	
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
//         int q;
//         ml_.MLGetInteger(&q);
//         std::cout << "#syntax:" << q << std::endl;
        break; 
      }

    case INPUTNAMEPKT: 
      {
//         const char *name;
//         ml_.MLGetString(&name);
//         std::cout << "#inputname:" << name << std::endl;
//         ml_.MLReleaseString(name);
        break;
      }

    default:
      std::cerr << "#unknown packet:" << pkt << std::endl;
    }
    ml_.MLNewPacket();
  }
}

} // namespace symbolic_legacy_simulator
} // namespace hydla

