#include "Logger.h"

#include <iostream>

namespace hydla {
namespace logger {

int	Logger::enflag=-1;//entailflag
int Logger::conflag=-1;//consistentflag
int Logger::ptflag=-1;//parse_treeflag

Logger::Logger() :
  log_level_(Warn)
{
  area_.push(std::cout);
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
