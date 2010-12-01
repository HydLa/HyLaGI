#include "Logger.h"

#include <iostream>

namespace hydla {
namespace logger {

Logger::Logger() :
  log_level_(Warn)
{
  debug_.push(std::cout);
  summary_.push(std::cout);//大局的デバッグモード
  warn_.push(std::cout);
  error_.push(std::cout);
  fatal_.push(std::cout);
}

Logger::~Logger()
{}

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

} // namespace logger
} // namespace hydla
