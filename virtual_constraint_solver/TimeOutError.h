#ifndef _INCLUDED_HYDLA_TIME_OUT_ERROR_H_
#define _INCLUDED_HYDLA_TIME_OUT_ERROR_H_


#include <string>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace vcs {

class TimeOutError : public std::runtime_error {
public:
  TimeOutError(const std::string& msg) : 
    std::runtime_error("time out while solving\n" + msg)
  {}
};

} //namespace vcs
} //namespace hydla 

#endif