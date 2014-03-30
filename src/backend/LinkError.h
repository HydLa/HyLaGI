#ifndef _INCLUDED_HYDLA_BACKEND_LINK_ERROR_H_
#define _INCLUDED_HYDLA_BACKEND_LINK_ERROR_H_

#include <sstream>
#include <stdexcept>

namespace hydla{
namespace backend{

class LinkError : public std::runtime_error{
public:
 LinkError(const std::string& backend_name, const std::string& msg, int code, const std::string &trace = "") : 
  std::runtime_error(init(backend_name, msg, code, trace))
  {}

private:
    std::string init(const std::string& backend_name, const std::string& msg, int code, const std::string &trace)
  {
    std::stringstream s;
    s << backend_name << " link error: " << msg << " : " << code << "\n" << trace;
    return s.str();
  }
};

}
}

#endif
