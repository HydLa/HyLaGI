#include "MathSimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
//#include <boost/thread.hpp>
#include "mathlink_helper.h"

using namespace std;
using namespace boost;
using namespace boost::xpressive;


namespace hydla{
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
			     const char interlanguage[],
			     bool debug_mode,
			     bool profile_mode)
{
  MathLink ml;
  if(!ml.init(mathlink)) {
    std::cerr << "can not link" << std::endl;
    return false;
  }
  
  ml.MLPutFunction("Set", 2);
  ml.MLPutSymbol("optUseDebugPrint"); 
  ml.MLPutSymbol(debug_mode ? "True" : "False");

  ml.MLPutFunction("Set", 2);
  ml.MLPutSymbol("optUseProfile"); 
  ml.MLPutSymbol(profile_mode ? "True" : "False");
 
  ml.MLPutFunction("Get", 1);
  ml.MLPutString("HydLa.m"); 

  ml.MLPutFunction("ToExpression", 1);
  ml.MLPutString(interlanguage); 

  ml.MLEndPacket();

  ml.MLPutFunction("Exit", 0);
  ml.MLEndPacket();

  
  sregex rawchar_reg = sregex::compile("\\\\(0\\d\\d)");
  std::ostream_iterator< char > out_iter( std::cout );
  rawchar_formatter rfmt;

  int pkt;
  while((pkt = ml.MLNextPacket()) != ILLEGALPKT) {
    switch(pkt) {
    case RETURNPKT:
      switch(ml.MLGetType()) {
      case MLTKSTR: {
	const char *str;
	ml.MLGetString(&str);
	std::cout << "#string:" << str << std::endl;
	ml.MLReleaseString(str);
	break;
      }

      case MLTKSYM: {
	const char *sym;
	ml.MLGetSymbol(&sym);
	std::cout << "#symbol:" << sym << std::endl;
	ml.MLReleaseSymbol(sym);
	break;
      }
	
      case MLTKINT:
      case MLTKOLDINT: {	  
	int q;
	ml.MLGetInteger(&q);
	std::cout << "#integer:" << q << std::endl;
	break;
      }
      case MLTKERR:
	std::cout << "#error type" << std::endl;
	break;
	  
      default:
	std::cout << "#unknown type" << std::endl;
      }
      break;

    case TEXTPKT: {
	const char *s;
	ml.MLGetString(&s);
	string str(s);
 	regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);	
	//	std::cout << str << std::endl;
	ml.MLReleaseString(s);
      }
      break;

      //    default:
    }
    ml.MLNewPacket();
  }
  
  return true;
}



} //namespace hydla

