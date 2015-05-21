#include "Constants.h"
#include "Logger.h"

using namespace std;
using namespace hydla::simulator;

namespace hydla
{
namespace io
{

string get_string_for_cause(simulator::SimulationState cause)
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
  case simulator::TIME_OUT_REACHED:
    return "TIME_OUT_REACHED";
  case simulator::NOT_UNIQUE_IN_INTERVAL:
    return "NOT_UNIQUE_IN_INTERVAL";
  case simulator::NOT_SIMULATED:
    return "NOT_SIMULATED";
  case simulator::INTERRUPTED:
    return "INTERRUPTED";
  case simulator::SIMULATED:
    return "SIMULATED";
  defalut:
    assert(0);
  }
}


simulator::SimulationState get_cause_for_string(const string& str)
{
  if(str == "TIME_LIMIT")return TIME_LIMIT;
  else if(str == "STEP_LIMIT")return STEP_LIMIT;
  else if(str == "SOME_ERROR")return SOME_ERROR;
  else if(str == "INCONSISTENCY")return INCONSISTENCY;
  else if(str == "ASSERTION")return ASSERTION;
  else if(str == "TIME_OUT_REACHED")return TIME_OUT_REACHED;
  else if(str == "NOT_UNIQUE_IN_INTERVAL")return NOT_UNIQUE_IN_INTERVAL;
  else if(str == "NOT_SIMULATED")return NOT_SIMULATED;
  else if(str == "INTERRUPTED")return INTERRUPTED;
  else if(str == "SIMULATED")return SIMULATED;
  assert(0);
}


}
}

