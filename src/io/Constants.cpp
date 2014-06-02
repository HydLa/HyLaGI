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
    break;
  case simulator::STEP_LIMIT:
    return "STEP_LIMIT";
    break;
  case simulator::SOME_ERROR:
    return "SOME_ERROR";
    break;
  case simulator::INCONSISTENCY:
    return "INCONSISTENCY";
    break;
  case simulator::ASSERTION:
    return "ASSERTION";
    break;
  case simulator::OTHER_ASSERTION:
    return "OTHER_ASSERTION";
    break;
  case simulator::TIME_OUT_REACHED:
    return "TIME_OUT_REACHED";
    break;
  case simulator::NOT_UNIQUE_IN_INTERVAL:
    return "NOT_UNIQUE_IN_INTERVAL";
    break;
  case simulator::NOT_SELECTED:
    return "NOT_SELECTED";
    break;
  case simulator::NONE:
    return "NONE";
    break;
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
  else return NONE;
}


}
}

