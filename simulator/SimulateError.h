#ifndef _INCLUDED_HYDLA_SIMULATE_ERROR_H_
#define _INCLUDED_HYDLA_SIMULATE_ERROR_H_


#include <string>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace simulator {

class SimulateError : public std::runtime_error {
public:
  SimulateError(const std::string& msg) : 
    std::runtime_error("error occured while simulating: " + msg)
  {}
};
    
} //namespace simulator
} //namespace hydla 

#endif