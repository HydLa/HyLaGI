#include "SymbolicValue.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{


const std::string SymbolicValue::relation_symbol_[RELATION_NUMBER] = {"", "!=", "<=", "<", ">=", ">"};

SymbolicValue::SymbolicValue(){
 current_or_ = NULL;
}

SymbolicValue::Element::Element(const symbolic_element_value_t& val, const Relation& rel){
  value = val;
  relation = rel;
}


std::string SymbolicValue::Element::get_symbol() const{
  return SymbolicValue::relation_symbol_[relation];
}

std::string SymbolicValue::Element::get_value() const{
  if(value.empty())return "UNDEF";
  return value;
}




bool SymbolicValue::is_undefined() const
{
  return value_range_.empty();
}


//定量的に一意に定まるかどうか．
//複数の不等号から値が一意に決定する場合は考慮しない
bool SymbolicValue::is_unique() const
{
  if(value_range_.empty()) return false;
  return value_range_.front().size()==1&&value_range_.front().front().relation == EQUAL;
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

symbolic_element_value_t SymbolicValue::get_first_value() const{
  if(value_range_.empty()||value_range_.front().empty())return "UNDEF";
  return value_range_.front().front().value;
}

SymbolicValue::Relation SymbolicValue::get_first_relation() const{
  if(value_range_.empty()||value_range_.front().empty())return EQUAL;
  return value_range_.front().front().relation;
}

std::string SymbolicValue::get_first_symbol() const{
  if(value_range_.empty()||value_range_.front().empty())return "";
  return relation_symbol_[value_range_.front().front().relation];
}


std::string SymbolicValue::visit_all(const std::vector<Element> &vec, const std::string &delimiter) const{
  if(vec.empty()) return "";
  std::string tmp;
  std::vector<Element>::const_iterator it = vec.begin();
  tmp.append(relation_symbol_[it->relation]);
  tmp.append((it++)->value);
  for(;it != vec.end();it++){
    tmp.append(delimiter);
    tmp.append(relation_symbol_[it->relation]);
    tmp.append(it->value);
  }
  return tmp;
}

void SymbolicValue::set(const std::vector<std::vector<SymbolicValue::Element> > &vec)
{
 value_range_ = vec;
}

void SymbolicValue::clear()
{
 value_range_.clear();
 current_or_ = NULL;
}


void SymbolicValue::set(const std::string &str)
{
 clear();
 go_next_or();
 Element ele(str,EQUAL);
 add(ele);
}

void SymbolicValue::set(const Element &ele)
{
 clear();
 go_next_or();
 add(ele);
}

void SymbolicValue::go_next_or()
{
 std::vector< Element> tmp;
 value_range_.push_back(tmp);
 current_or_ = &value_range_.back();
}

void SymbolicValue::add(const SymbolicValue::Element &ele)
{
 if(!current_or_){
  go_next_or();
 }
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