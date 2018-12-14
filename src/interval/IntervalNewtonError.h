#pragma once

#include <stdexcept>

namespace hydla {
namespace interval {

class IntervalNewtonError : public std::runtime_error {
public:
  IntervalNewtonError(const std::string& msg, const std::string& branchParamName, int branchPhaseID) : 
    std::runtime_error("error while interval newton \n" + msg),
    branchParamName(branchParamName),
    branchPhaseID(branchPhaseID)
  {}
  std::string branchParamName;
  int branchPhaseID;
};

} //namespace interval
} //namespace hydla 
