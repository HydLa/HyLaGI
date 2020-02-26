#pragma once

#include <stdexcept>

namespace hydla {
namespace timeout {

class TimeOutError : public std::runtime_error {
public:
  TimeOutError(const std::string &msg)
      : std::runtime_error("time out while solving\n" + msg) {}
};

} // namespace timeout
} // namespace hydla
