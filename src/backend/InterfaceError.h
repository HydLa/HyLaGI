#pragma once

namespace hydla{
namespace backend{

class InterfaceError : public std::runtime_error{
public:
  InterfaceError(const std::string& msg) : 
  std::runtime_error(init(msg))
  {}

private:
    std::string init(const std::string& msg)
  {
    std::stringstream s;
    s << "error of backend interface: " << msg << std::endl;
    return s.str();
  }
};

}
}
