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
  standard_.push(std::cout);
}

Logger::~Logger()
{
  hydla::logger::Logger& i = hydla::logger::Logger::instance();
  if (!i.is_html_mode())
  {
    return;
  }

  std::cerr << std::endl;
  std::cout << std::endl;

  i.debug_ << "</body>\n</html>" << std::endl;
}

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

} // namespace logger
} // namespace hydla
