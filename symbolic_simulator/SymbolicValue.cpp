#include "SymbolicValue.h"
#include "../parser/TreeInfixPrinter.h"

#include <cassert>
#include <sstream>

#include "Logger.h"

namespace hydla {
namespace simulator{
namespace symbolic {

bool SymbolicValue::undefined() const
{
  return (node_ == NULL);
}

SymbolicValue::SymbolicValue(){}

SymbolicValue::SymbolicValue(const std::string &str){
  node_.reset((new hydla::parse_tree::Number(str)));
}
SymbolicValue::SymbolicValue(const SymbolicValue::node_sptr & node){
  node_ = node;
}


std::string SymbolicValue::get_string() const
{
  if(undefined()) return "";
  hydla::parse_tree::TreeInfixPrinter printer;
  std::ostringstream os;
  printer.print_infix(node_,os);
  return os.str();
}


SymbolicValue::node_sptr SymbolicValue::get_node() const
{
  if(node_==NULL)assert(0);
  return node_;
}


void SymbolicValue::set_node(node_sptr n)
{
  node_ = n;
}

void SymbolicValue::set(const node_sptr &nod)
{
 node_=nod;
}


} // namespace symbolic
} // namespace simulator
} // namespace hydla