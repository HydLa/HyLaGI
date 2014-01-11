#include "Value.h"

using namespace hydla::simulator;

bool hydla::simulator::operator<(const Value& lhs, const Value& rhs){
  return lhs.get_string() < rhs.get_string();
}

std::ostream& hydla::simulator::operator<<(std::ostream& s, const Value & v){
  return v.dump(s);
}

Value& Value::operator+=(const Value& rhs){
  set_node(node_sptr(new hydla::parse_tree::Plus(get_node(), rhs.get_node())));
  return *this;
}

Value& Value::operator-=(const Value& rhs){
  set_node(node_sptr(new hydla::parse_tree::Subtract(get_node(), rhs.get_node())));
  return *this;
}




bool Value::undefined() const
{
  return (node_ == NULL);
}

Value::Value(){}

Value::Value(const std::string &str){
  node_.reset((new hydla::parse_tree::Number(str)));
}
Value::Value(const Value::node_sptr & node){
  node_ = node;
}


std::string Value::get_string() const
{
  if(undefined()) return "";
  return hydla::parse_tree::get_infix_string(node_);
}


Value::node_sptr Value::get_node() const
{
  if(node_==NULL)assert(0);
  return node_;
}

void Value::set_node(const node_sptr &n)
{
  node_ = n;
}
