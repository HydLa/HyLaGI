#ifndef _VALUE_VISITOR_H_
#define _VALUE_VISITOR_H_


namespace hydla {
namespace simulator {
namespace symbolic
{
class SymbolicValue;
}

class ValueVisitor{
  public:
  virtual void visit_value(hydla::simulator::symbolic::SymbolicValue&) = 0;
};

} // namespace simulator
} // namespace hydla

#endif // _VALUE_VISITOR_H_
