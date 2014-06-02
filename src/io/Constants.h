#pragma once

#include <map>
#include "PhaseResult.h"

namespace hydla
{
namespace io
{

std::string get_string_for_cause(simulator::CauseForTermination);
simulator::CauseForTermination get_cause_for_string(const std::string &str);

}
}
