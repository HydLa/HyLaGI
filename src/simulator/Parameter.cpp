#include "Parameter.h"
#include "PhaseResult.h"
#include "Value.h"
#include <stdexcept>
#include <stdlib.h>

using namespace std;

namespace hydla{
namespace simulator{

Parameter::Parameter(const Variable &variable, const PhaseResult &phase)
  :variable_name_(variable.get_name()), differential_count_(variable.get_differential_count()), phase_id_(phase.id)
{}

Parameter::Parameter(const std::string &name, int diff_cnt, int id)
  :variable_name_(name), differential_count_(diff_cnt), phase_id_(id)
{}

std::string Parameter::to_string() const
{
  std::stringstream strstr;
  std::string ret("p[" + variable_name_);
  strstr << ", " << differential_count_ << ", " << phase_id_ << "]";
  ret += strstr.str();
  return ret;
}
  
std::ostream& Parameter::dump(std::ostream& s) const
{
  s << to_string();
  return s;
}
  
    
bool operator<(const Parameter& lhs, 
                        const Parameter& rhs)
{
  return lhs.to_string() < rhs.to_string();
}
  
Value Parameter::as_value()const
{
  return Value(symbolic_expression::node_sptr(new symbolic_expression::Parameter(get_name(), get_differential_count(), get_phase_id())));
}

Parameter::Parameter(const std::string &str)
{
  string::size_type prev_index, index;
  prev_index = str.find_first_of("[");
  index = str.find_first_of(",");
  if(prev_index == string::npos || index == string::npos || prev_index >= index)throw runtime_error("invalid parameter name " + str); 
  variable_name_ = str.substr(prev_index + 1, index - (prev_index + 1));
  prev_index = index;
  index = str.find_first_of(",", index + 1);
  if(prev_index == string::npos || index == string::npos || prev_index >= index)throw runtime_error("invalid parameter name " + str); 
  differential_count_ = atoi(str.substr(prev_index + 1, index - (prev_index + 1)).c_str());
  prev_index = index;
  index = str.find_first_of("]", index + 1);
  if(prev_index == string::npos || index == string::npos || prev_index >= index)throw runtime_error("invalid parameter name " + str); 
  phase_id_ = atoi(str.substr(prev_index + 1, index - (prev_index + 1)).c_str());
}


bool ParameterComparator::operator()(const Parameter x,const Parameter y) const { return x < y; }

std::ostream& operator<<(std::ostream& s, 
                               const Parameter& p)
{
  return p.dump(s);
}

} //simulator
} //hydla
