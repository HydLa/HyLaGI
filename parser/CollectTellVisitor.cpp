#include "CollectTellVisitor.h"
#include "TreeVisitor.h"
#include "mathlink_helper.h"

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

bool CollectTellVisitor::is_consinstent(MathLink& ml)
{
//    ml.MLPutFunction("Equal", 2);
    ml.MLPutSymbol("x");
    //  ml.MLPutSymbol("x");

    int p;
    while ((p = ml.MLNextPacket()) && p != RETURNPKT)
      ml.MLNewPacket();

    std::cout << "type:" << ml.MLGetType() << std::endl;

    const char *sym;
    ml.MLGetSymbol(&sym);
    std::cout << "#symbol:" << sym << std::endl;
    ml.MLReleaseSymbol(sym);
    
    return false;
}

} //namespace symbolic_simulator
} // namespace hydla
