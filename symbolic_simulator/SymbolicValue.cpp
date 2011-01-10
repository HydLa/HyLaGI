#include "SymbolicValue.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{


const std::string SymbolicValue::relation_symbol_[RELATION_NUMBER] = {"=", "!=", "<=", "<", ">=", ">"};

bool SymbolicValue::is_undefined() const
{
  //return value_.empty();
  return str_.empty();
}


std::ostream& SymbolicValue::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << str_;
  return s;
}

std::string SymbolicValue::get_string() const
{
  return str_;
  std::string tmp_str;
  std::vector< std::vector<Element> >::const_iterator out_it = value_.begin();
  tmp_str.append(visit_all(*out_it, "&&"));
  for(;out_it != value_.end();out_it++){
    tmp_str.append("||");
    tmp_str.append(visit_all(*out_it, "&&"));
  }
  return tmp_str;

}


std::string SymbolicValue::visit_all(const std::vector<Element> &vec, const std::string &delimiter) const{
  if(vec.empty()) return "";
  std::string tmp;
  std::vector<Element>::const_iterator it = vec.begin();
  tmp.append((it++)->value);
  for(;it != vec.end();it++){
    tmp.append(delimiter);
    tmp.append(it->value);
  }
  return tmp;
}


void SymbolicValue::set(std::string string)
{
  str_ = string;
}

void SymbolicValue::add(const std::vector<Element> &vec)
{
  //value_.push_back(vec);
}

void SymbolicValue::set(const std::vector<std::vector<SymbolicValue::Element> > &vec)
{
 value_ = vec;
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

