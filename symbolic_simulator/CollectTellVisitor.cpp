#include "CollectTellVisitor.h"

#include <iostream>

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

bool CollectTellVisitor::is_consinstent(MathLink& ml)
{
  ml.MLPutFunction("Equal", 2);
  ml.MLPutSymbol("x");
  ml.MLPutSymbol("x");
  ml.MLEndPacket();

    int p;
    while ((p = ml.MLNextPacket()) && p != RETURNPKT)
      ml.MLNewPacket();

    const char *sym;
    ml.MLGetSymbol(&sym);
    std::cout << "#symbol:" << sym << std::endl;
    ml.MLReleaseSymbol(sym);
    
    return false;
}

} //namespace symbolic_simulator
} // namespace hydla
