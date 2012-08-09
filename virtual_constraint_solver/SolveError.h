#ifndef _INCLUDED_HYDLA_SOLVE_ERROR_H_
#define _INCLUDED_HYDLA_SOLVE_ERROR_H_


#include <string>
#include <sstream>
#include <stdexcept>

namespace hydla {
namespace vcs {

class SolveError : public std::runtime_error {
public:
  SolveError(const std::string& msg) : 
    std::runtime_error("error occurred while solving\n" + msg)
  {}
};
    
} //namespace vcs
} //namespace hydla 

#endif