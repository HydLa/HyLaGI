#ifndef _VALUE_VISITOR_H_
#define _VALUE_VISITOR_H_


namespace hydla {

namespace symbolic_simulator{
class SymbolicValue;
}
namespace simulator {


class ValueVisitor{
  public:
  virtual void visit(hydla::symbolic_simulator::SymbolicValue&) = 0;
};

} // namespace simulator
} // namespace hydla

#endif // _VALUE_VISITOR_H_
