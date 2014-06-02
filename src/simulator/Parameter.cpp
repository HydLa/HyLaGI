#include "Parameter.h"
#include <stdexcept>
#include <stdlib.h>

using namespace std;

namespace hydla{
namespace simulator{

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

std::ostream& operator<<(std::ostream& s, 
                               const Parameter& p)
{
  return p.dump(s);
}

} //simulator
} //hydla
