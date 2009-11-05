#include "CollectTellVisitor.h"

#include <iostream>

using namespace hydla::parse_tree;

namespace hydla {
namespace symbolic_simulator {

bool CollectTellVisitor::is_consistent(MathLink& ml)
{
  ml.MLPutFunction("isConsistent", 2);
  ml.MLPutFunction("List", 3);
  ml.MLPutFunction("Equal", 2);
  ml.MLPutSymbol("x");
  ml.MLPutSymbol("y");
  ml.MLPutFunction("Equal", 2);
  ml.MLPutSymbol("x");
  ml.MLPutInteger(1);
  ml.MLPutFunction("Equal", 2);
  ml.MLPutSymbol("y");
  ml.MLPutInteger(2);
  ml.MLPutFunction("List", 2);
  ml.MLPutSymbol("x");
  ml.MLPutSymbol("y");
  ml.MLEndPacket();

  int p;
  while ((p = ml.MLNextPacket()) && p != RETURNPKT) ml.MLNewPacket();

  int num;
  ml.MLGetInteger(&num);
  std::cout << "#num:" << num << std::endl;
  
  // Mathematicaから1（Trueを表す）が返ればtrueを、0（Falseを表す）が返ればfalseを返す
  if(num==1) return true;
  return false;
}

} //namespace symbolic_simulator
} // namespace hydla
