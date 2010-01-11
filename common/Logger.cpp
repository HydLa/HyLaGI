#include "Logger.h"

#include <iostream>

namespace hydla {
namespace logger {

Logger::Logger()
{
  debug_.push(std::cout);

  //warn_;
  //error_;
  //fatal_;
}

Logger::~Logger()
{}

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

} // namespace logger
} // namespace hydla
