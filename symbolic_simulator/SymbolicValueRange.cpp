#include "SymbolicValueRange.h"

#include <cassert>

#include <iostream>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{


const std::string SymbolicValueRange::relation_symbol_[RELATION_NUMBER] = {"", "!=", "<=", "<", ">=", ">"};

SymbolicValueRange::SymbolicValueRange(){
}

SymbolicValueRange::Element::Element(const SymbolicValue& val, const Relation& rel){
  value = val;
  relation = rel;
}

SymbolicValueRange::Relation SymbolicValueRange::Element::get_relation() const{
  return relation;
}

std::string SymbolicValueRange::Element::get_symbol() const{
  return SymbolicValueRange::relation_symbol_[relation];
}

SymbolicValue SymbolicValueRange::Element::get_value() const{
  return value;
}

bool SymbolicValueRange::is_undefined() const
{
  return value_range_.empty();
}

//定量的に一意に定まるかどうか．
//複数の不等号から値が一意に決定する場合は考慮しない
bool SymbolicValueRange::is_unique() const
{
  if(value_range_.empty()) return false;
  return value_range_.front().size()==1&&value_range_.front().front().relation == EQUAL;
}


std::ostream& SymbolicValueRange::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << get_string();
  return s;
}

std::string SymbolicValueRange::get_string() const
{
  //return str_;
  if(value_range_.empty()||value_range_.front().empty())return "";
  std::string tmp_str;
  std::vector< std::vector<Element> >::const_iterator out_it = value_range_.begin();
  tmp_str.append(visit_all(*out_it++, "&&"));
  for(;out_it != value_range_.end();out_it++){
    tmp_str.append("||");
    tmp_str.append(visit_all(*out_it, "&&"));
  }
  return tmp_str;
}

SymbolicValue SymbolicValueRange::get_first_value() const{
  if(value_range_.empty()||value_range_.front().empty())return SymbolicValue();
  return value_range_.front().front().value;
}

SymbolicValueRange::Relation SymbolicValueRange::get_first_relation() const{
  if(value_range_.empty()||value_range_.front().empty())return EQUAL;
  return value_range_.front().front().relation;
}

std::string SymbolicValueRange::get_first_symbol() const{
  if(value_range_.empty()||value_range_.front().empty())return "?";
  return relation_symbol_[value_range_.front().front().relation];
}


std::string SymbolicValueRange::visit_all(const std::vector<Element> &vec, const std::string &delimiter) const{
  if(vec.empty()) return "";
  std::string tmp;
  std::vector<Element>::const_iterator it = vec.begin();
  tmp.append(relation_symbol_[it->relation]);
  tmp.append((it++)->value.get_string());
  for(;it != vec.end();it++){
    tmp.append(delimiter);
    tmp.append(relation_symbol_[it->relation]);
    tmp.append(it->value.get_string());
  }
  return tmp;
}

void SymbolicValueRange::set(const std::vector<std::vector<SymbolicValueRange::Element> > &vec)
{
 value_range_ = vec;
}

void SymbolicValueRange::clear()
{
 value_range_.clear();
}

void SymbolicValueRange::set(const Element &ele)
{
 clear();
 add(ele);
}


void SymbolicValueRange::add(const SymbolicValueRange::Element &ele)
{
  if(value_range_.size()==0){
    and_vector tmp;
    value_range_.push_back(tmp);
  }
  value_range_.back().push_back(ele);
}


void SymbolicValueRange::add_vector(const and_vector &clause)
{
 value_range_.push_back(clause);
}

bool operator<(const SymbolicValueRange& lhs, 
               const SymbolicValueRange& rhs)
{
  return lhs.get_string() < rhs.get_string();
}


std::ostream& operator<<(std::ostream& s, 
                         const SymbolicValueRange & v)
{
  return v.dump(s);
}


} // namespace symbolic_simulator
} // namespace hydla