#pragma once

#include <stdexcept>

namespace hydla {
namespace interval {

class IntervalNewtonError : public std::runtime_error {
public:
  IntervalNewtonError(const std::string& msg) : 
    std::runtime_error("error while interval newton \n" + msg)
  {}
};

} //namespace interval
} //namespace hydla 
