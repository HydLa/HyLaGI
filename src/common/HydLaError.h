#pragma once

#include <string>
#include <sstream>
#include <stdexcept>

#define HYDLA_TO_STR(n) HYDLA_TO_STR_(n) // needed to expand __LINE__ macro
#define HYDLA_TO_STR_(n) #n
#define HYDLA_ERROR(MSG)                                  \
  hydla::HydLaError("@" __FILE__ " " HYDLA_TO_STR(__LINE__) " ", MSG)

#define HYDLA_ASSERT(COND)                                \
  if(!(COND)) throw  hydla::HydLaError("@" __FILE__ " " HYDLA_TO_STR(__LINE__) " assertion failure: ", HYDLA_TO_STR(COND))


namespace hydla {

class HydLaError : public std::runtime_error {
public:
  HydLaError(const std::string& msg) : 
    std::runtime_error(msg)
  {}
  
  HydLaError(const std::string& prefix, const std::string& msg) : 
    std::runtime_error(prefix + msg)
  {}
};
    
} //namespace hydla 

