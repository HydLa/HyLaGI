#include "SymbolicValue.h"

#include <cassert>

#include "Logger.h"

namespace hydla {
namespace symbolic_simulator{

using namespace std;


bool SymbolicValue::is_undefined() const
{
  //return value_.empty();
  return str_.empty();
}


ostream& SymbolicValue::dump(ostream& s) const
{
  if(is_undefined()) s << "UNDEF";
  else s << str_;
  return s;
}

string SymbolicValue::get_string() const
{
  return str_;/*
  string tmp_str;
  vector< vector<string> >::const_iterator out_it = value_.begin();
  vector<string>::const_iterator in_it = out_it->begin();
  for(;out_it != out_it->end();out_it++){
    tmp_str.append("||");
    tmp_str.append(visit_all(out_it, out_it->end(), "&&");
  }
  return tmp_str;*/
}

/*
template <typename Element> string visit_all(vector<Element>::const_iterator begin,vector<Element>::const_iterator end, string delimiter){
  string tmp;
  tmp.append(*(begin++));
  for(;begin != end;begin++){
    tmp.append(delimiter);
    tmp.append(*begin);
  }
}
*/

void SymbolicValue::set(string string)
{
  str_ = string;
}

bool operator<(const SymbolicValue& lhs, 
               const SymbolicValue& rhs)
{
  return lhs.get_string() < rhs.get_string();
}


ostream& operator<<(ostream& s, 
                         const SymbolicValue & v)
{
  return v.dump(s);
}


} // namespace symbolic_simulator
} // namespace hydla

