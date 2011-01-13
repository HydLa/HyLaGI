#include "SymbolicValue.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{


const std::string SymbolicValue::relation_symbol_[RELATION_NUMBER] = {"", "!=", "<=", "<", ">=", ">"};

SymbolicValue::SymbolicValue(){
}

bool SymbolicValue::is_undefined() const
{
  return value_.empty();
  //return str_.empty();
}


std::ostream& SymbolicValue::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << get_string();
  return s;
}

std::string SymbolicValue::get_string() const
{
  //return str_;
  if(value_.empty()||value_.front().empty())return "";
  std::string tmp_str;
  std::vector< std::vector<Element> >::const_iterator out_it = value_.begin();
  tmp_str.append(visit_all(*out_it++, "&&"));
  for(;out_it != value_.end();out_it++){
    tmp_str.append("||");
    tmp_str.append(visit_all(*out_it, "&&"));
  }
  return tmp_str;

}

symbolic_element_value_t SymbolicValue::get_first_value() const{
  if(value_.empty()||value_.front().empty())return "UNDEF";
  return value_.front().front().value;
}

SymbolicValue::Relation SymbolicValue::get_first_relation() const{
  if(value_.empty()||value_.front().empty())return EQUAL;
  return value_.front().front().relation;
}

std::string SymbolicValue::get_first_symbol() const{
  if(value_.empty()||value_.front().empty())return "";
  return relation_symbol_[value_.front().front().relation];
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

void SymbolicValue::set(const std::vector<std::vector<SymbolicValue::Element> > &vec)
{
 value_ = vec;
}


void SymbolicValue::go_next_or()
{
 std::vector< Element> tmp;
 value_.push_back(tmp);
 current_or_ = &value_.back();
}

void SymbolicValue::add(const SymbolicValue::Element &ele)
{
 current_or_->push_back(ele);
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

