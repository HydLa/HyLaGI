#include "Value.h"
#include "DefaultParameter.h"
#include <sstream>
#include "Utility.h"

using namespace hydla::simulator;
using namespace hydla::utility;
using namespace hydla::parse_tree;

bool hydla::simulator::operator<(const Value& lhs, const Value& rhs){
  return lhs.get_string() < rhs.get_string();
}

std::ostream& hydla::simulator::operator<<(std::ostream& s, const Value & v){
  return v.dump(s);
}

Value& Value::operator+=(const Value& rhs){
  set_node(node_sptr(new Plus(get_node(), rhs.get_node())));
  return *this;
}

Value Value::operator+(const Value& rhs){
  Value ret(*this);
  return ret += rhs;
}

Value& Value::operator-=(const Value& rhs){
  set_node(node_sptr(new Subtract(get_node(), rhs.get_node())));
  return *this;
}

Value Value::operator-(const Value& rhs){
  Value ret(*this);
  return ret -= rhs;
}

Value& Value::operator*=(const Value& rhs){
  set_node(node_sptr(new Times(get_node(), rhs.get_node())));
  return *this;
}

Value Value::operator*(const Value& rhs){
  Value ret(*this);
  return ret *= rhs;
}

Value& Value::operator/=(const Value& rhs){
  set_node(node_sptr(new Divide(get_node(), rhs.get_node())));
  return *this;
}

Value Value::operator/(const Value& rhs){
  Value ret(*this);
  return ret /= rhs;
}


bool Value::undefined() const
{
  return (node_ == NULL);
}

Value::Value(){}

Value::Value(const std::string &str){
  node_.reset(new Number(str));
}

Value::Value(const DefaultParameter &param)
{
  node_.reset(new Parameter(
               param.get_name(),
               param.get_derivative_count(), 
               param.get_phase_id()));
}


Value::Value(int num){
  std::stringstream sstr;
  sstr << num;
  node_.reset(new Number(sstr.str()));
}

Value::Value(double num)
{
  node_.reset(new Float(num));
}

Value::Value(const Value::node_sptr & node){
  node_ = node;
}


std::string Value::get_string() const
{
  if(undefined()) return "";
  return get_infix_string(node_);
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