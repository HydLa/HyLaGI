#include "MathSimulator.h"

#include <iostream>
#include <boost/xpressive/xpressive.hpp>
#include <boost/thread.hpp>
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

bool MathSimulator::simulate(const char mathlink[], const char interlanguage[])
{
  MathLink ml;
  if(!ml.init(mathlink)) {
    std::cerr << "can not link" << std::endl;
    return false;
  }
  
  ml.MLPutFunction("EvaluatePacket", 2);  
  
  ml.MLPutFunction("Get", 1);
  ml.MLPutString("HydLa.m"); 

  ml.MLPutFunction("ToExpression", 1);
  ml.MLPutString(interlanguage); 

  ml.MLEndPacket();

  ml.MLPutFunction("Exit", 0);

  sregex rawchar_reg = sregex::compile("\\\\(\\d\\d\\d)");
  std::ostream_iterator< char > out_iter( std::cout );
  rawchar_formatter rfmt;

  	
  int pkt;
  while((pkt = ml.MLNextPacket()) != ILLEGALPKT) {
    switch(pkt) {
    case RETURNPKT:
      {
// 	const char *str;
// 	ml.MLGetString(&str);
// 	std::cout << str << std::endl;
// 	ml.MLReleaseString(str);
	int q;
	ml.MLGetInteger(&q);
	//	printf( "sol = %d\n", q);
      }
      break;

    case TEXTPKT:
      {
	const char *s;
	ml.MLGetString(&s);
	string str(s);
	regex_replace( out_iter, str.begin(), str.end(), rawchar_reg, rfmt);
	ml.MLReleaseString(s);
      }
      break;

      //    default:
    }
    ml.MLNewPacket();
    /* 
   ml.MLFlush();
    int i=0;
    while(ml.MLReady() == 0) {
      std::cout << "wait_" << i++ << std::endl;
      //boost::xtime xt;
      //boost::xtime_get(&xt, boost::TIME_UTC);
      //xt.nsec += 100;
      //boost::thread::sleep(xt);
    }
    */
  }
  
  return true;
}

} //namespace hydla

