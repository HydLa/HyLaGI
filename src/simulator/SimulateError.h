#pragma once

#include <string>
#include <sstream>
#include <stdexcept>

#define HYDLA_TO_STR(n) HYDLA_TO_STR_(n) // needed to expand __LINE__ macro
#define HYDLA_TO_STR_(n) #n
#define HYDLA_SIMULATE_ERROR(MSG)                                  \
  hydla::simulator::SimulateError("@" __FILE__ " " HYDLA_TO_STR(__LINE__) " " MSG)

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

