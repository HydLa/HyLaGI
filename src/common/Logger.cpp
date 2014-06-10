#include "Logger.h"

namespace hydla {
namespace logger {

Logger::Logger() :
  log_level_(Warn)
{
  debug_.push(std::cerr);
  warn_.push(std::cerr);
  error_.push(std::cerr);
  fatal_.push(std::cerr);
}

Logger::~Logger()
{}

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

} // namespace logger
} // namespace hydla
