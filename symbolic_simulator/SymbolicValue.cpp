#include "SymbolicValue.h"
#include "TreeInfixPrinter.h"

#include <cassert>
#include <sstream>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{

bool SymbolicValue::is_undefined() const
{
  return (node_==NULL);
}

SymbolicValue::SymbolicValue():is_unique_(true){}

SymbolicValue::SymbolicValue(const std::string &str){
  node_.reset((new hydla::parse_tree::Number(str)));
  is_unique_=true;
}
SymbolicValue::SymbolicValue(const SymbolicValue::node_sptr & node){
  node_ = node;
  is_unique_=true;
}

//定量的に一意に定まるかどうか．
//複数の不等号から値が一意に決定する場合は考慮しない？
bool SymbolicValue::is_unique() const
{
  return is_unique_;
}


std::ostream& SymbolicValue::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << get_string();
  return s;
}

std::string SymbolicValue::get_string() const
{
  if(is_undefined()) return "";
  TreeInfixPrinter printer;
  std::ostringstream os;
  printer.print_infix(node_,os);
  return os.str();
}


SymbolicValue::node_sptr SymbolicValue::get_node() const
{
  if(node_==NULL)assert(0);
  return node_;
}



void SymbolicValue::set(const node_sptr &nod)
{
 node_=nod;
}


SymbolicValue& SymbolicValue::operator+=(const SymbolicValue& rhs)
{
  node_ = node_sptr(new hydla::parse_tree::Plus(node_, rhs.get_node()));
  return *this;
}


SymbolicValue& SymbolicValue::operator-=(const SymbolicValue& rhs)
{
  node_ = node_sptr(new hydla::parse_tree::Subtract(node_, rhs.get_node()));
  return *this;
}



bool operator<(const SymbolicValue& lhs, 
               const SymbolicValue& rhs)
{
  return lhs.get_string() < rhs.get_string();
}


std::ostream& operator<<(std::ostream& s, 
                         const SymbolicValue & v)
{
  return v.dump(s);
}


} // namespace symbolic_simulator
} // namespace hydla