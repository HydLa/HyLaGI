#ifndef _INCLUDED_HYDLA_LOGGER_H_
#define _INCLUDED_HYDLA_LOGGER_H_

#include <boost/iostreams/filtering_stream.hpp>

namespace hydla {
namespace logger {

class Logger
{
public:
  enum LogLevel {
    Debug,
    Warn,
    Error,
    Fatal,
    None,
  };

  ~Logger();

  LogLevel set_log_level(LogLevel l) 
  {
    LogLevel old = log_level_;
    log_level_ = l;
    return old;
  }

  LogLevel get_log_level() const
  {
    return log_level_;
  }

  bool is_valid_level(LogLevel level)
  {
    return log_level_ <= level;
  }

  static Logger& instance();

  template<typename A1>
  void debug_write(const A1& a1) {
    if(is_valid_level(Logger::Debug)) debug_ << a1;
  }

  template<typename A1, typename A2>
  void debug_write(const A1& a1, const A2& a2) {
    if(is_valid_level(Logger::Debug)) debug_ << a1 << a2;
  }

  template<typename A1, typename A2, typename A3>
  void debug_write(const A1& a1, const A2& a2, const A3& a3) {
    if(is_valid_level(Logger::Debug)) debug_ << a1 << a2 << a3;
  }

private:
  Logger();
  Logger(const Logger&);
  Logger& operator=(const Logger&); 

  LogLevel log_level_;

  boost::iostreams::filtering_ostream debug_;
  boost::iostreams::filtering_ostream warn_;
  boost::iostreams::filtering_ostream error_;
  boost::iostreams::filtering_ostream fatal_;
};  

#define HYDLA_LOGGER_DEBUG(...) {\
  hydla::logger::Logger& logger=hydla::logger::Logger::instance(); \
  if(logger.is_valid_level(hydla::logger::Logger::Debug)){logger.debug_write(__VA_ARGS__);} \
}

} // namespace logger
} // namespace hydla

#endif // _INCLUDED_HYDLA_LOGGER_H_