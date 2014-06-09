#pragma once

#include <string>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace simulator {

class SimulateError : public std::runtime_error {
public:
  SimulateError(const std::string& msg) : 
    std::runtime_error("error occurred while simulating: " + msg)
  {}
};

} //namespace simulator
} //namespace hydla 
