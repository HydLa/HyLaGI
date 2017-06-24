#pragma once

#include "PhaseResult.h"

namespace hydla
{
namespace io
{

std::string get_string_for_cause(simulator::SimulationState);
simulator::SimulationState get_cause_for_string(const std::string &str);

} // namespace io
} // namespace hydla
