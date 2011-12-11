#include "SymbolicValueRange.h"

#include <cassert>

#include <iostream>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{


SymbolicValueRange::SymbolicValueRange(){
}


bool SymbolicValueRange::is_undefined() const
{
  return (lower_.value.is_undefined() && upper_.value.is_undefined());
}

//定量的に一意に定まるかどうか．
//上限と下限が文字列として同じなら、ということで。文字列表現はいくらでも考えられるはずなので不完全
bool SymbolicValueRange::is_unique() const
{
  return (lower_.value.get_string() == upper_.value.get_string());
}


std::ostream& SymbolicValueRange::dump(std::ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << get_string();
  return s;
}

std::string SymbolicValueRange::get_string() const
{
  std::string tmp_str;
  if(!lower_.value.is_undefined() || !upper_.value.is_undefined()){
    if(lower_.include_bound){
      tmp_str += "[";
    }
    else{
      tmp_str += "(";
    }
    if(!lower_.value.is_undefined()){
      tmp_str += lower_.value.get_string();
    }else{
      tmp_str += "-inf";
    }
    
    tmp_str += ", ";
    
    if(!upper_.value.is_undefined()){
      tmp_str += upper_.value.get_string();
    }else{
      tmp_str += "inf";
    }
    
    if(upper_.include_bound){
      tmp_str += "]";
    }
    else{
      tmp_str += ")";
    }
  }
  return tmp_str;
}


const SymbolicValueRange::bound_t& SymbolicValueRange::get_lower_bound() const{
  return lower_;
}


const SymbolicValueRange::bound_t& SymbolicValueRange::get_upper_bound() const{
  return upper_;
}


void SymbolicValueRange::set_upper_bound(const SymbolicValue& val, const bool& include)
{
  upper_.value = val;
  upper_.include_bound = include;
}


void SymbolicValueRange::set_lower_bound(const SymbolicValue& val, const bool& include)
{
  lower_.value = val;
  lower_.include_bound = include;
}


std::ostream& operator<<(std::ostream& s, 
                         const SymbolicValueRange & v)
{
  return v.dump(s);
}


} // namespace symbolic_simulator
} // namespace hydla