#include "Value.h"
#include "SymbolicValue.h"

using namespace hydla::simulator;
using hydla::simulator::symbolic::SymbolicValue;

bool hydla::simulator::operator<(const Value& lhs, const Value& rhs){
  return lhs.get_string() < rhs.get_string();
}

std::ostream& hydla::simulator::operator<<(std::ostream& s, const Value & v){
  return v.dump(s);
}

Value& Value::operator+=(const Value& rhs){
  assert(typeid(*this) == typeid(SymbolicValue) && typeid(rhs) == typeid(SymbolicValue));
  SymbolicValue& me = static_cast<SymbolicValue &>(*this), him = static_cast<const SymbolicValue &>(rhs);
  me.set_node(SymbolicValue::node_sptr(new hydla::parse_tree::Plus(me.get_node(), him.get_node())));
  return *this;
}

Value& Value::operator-=(const Value& rhs){
  assert(typeid(*this) == typeid(SymbolicValue) && typeid(rhs) == typeid(SymbolicValue));
  SymbolicValue& me = static_cast<SymbolicValue &>(*this), him = static_cast<const SymbolicValue &>(rhs);
  me.set_node(SymbolicValue::node_sptr(new hydla::parse_tree::Subtract(me.get_node(), him.get_node())));
  return *this;
}
