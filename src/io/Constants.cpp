#include "Constants.h"

using namespace std;
using namespace hydla::simulator;

namespace hydla
{
namespace io
{

string get_string_for_cause(simulator::CauseForTermination cause)
{
  switch(cause){
  case simulator::TIME_LIMIT:
    return "TIME_LIMIT";
  case simulator::STEP_LIMIT:
    return "STEP_LIMIT";
  case simulator::SOME_ERROR:
    return "SOME_ERROR";
  case simulator::INCONSISTENCY:
    return "INCONSISTENCY";
  case simulator::ASSERTION:
    return "ASSERTION";
  case simulator::OTHER_ASSERTION:
    return "OTHER_ASSERTION";
  case simulator::TIME_OUT_REACHED:
    return "TIME_OUT_REACHED";
  case simulator::NOT_UNIQUE_IN_INTERVAL:
    return "NOT_UNIQUE_IN_INTERVAL";
  case simulator::NOT_SELECTED:
    return "NOT_SELECTED";
  case simulator::NONE:
    return "NONE";
  case simulator::INTERRUPTED:
    return "INTERRUPTED";
  default:
    return "ERROR";
  }
}


simulator::CauseForTermination get_cause_for_string(const string& str)
{
  if(str == "TIME_LIMIT")return TIME_LIMIT;
  else if(str == "STEP_LIMIT")return STEP_LIMIT;
  else if(str == "SOME_ERROR")return SOME_ERROR;
  else if(str == "INCONSISTENCY")return INCONSISTENCY;
  else if(str == "ASSERTION")return ASSERTION;
  else if(str == "OTHER_ASSERTION")return OTHER_ASSERTION;
  else if(str == "TIME_OUT_REACHED")return TIME_OUT_REACHED;
  else if(str == "NOT_UNIQUE_IN_INTERVAL")return NOT_UNIQUE_IN_INTERVAL;
  else if(str == "NOT_SELECTED")return NOT_SELECTED;
  else if(str == "INTERRUPTED")return INTERRUPTED;
  else return NONE;
}


}
}

