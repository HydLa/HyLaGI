#pragma once

#include <iostream>
#include <string>
#include <sstream>
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
    None
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

  bool valid_level(LogLevel level)
  {
    return log_level_ <= level;
  }
  
  /// LoggerのSingletonなインスタンス
  static Logger& instance();

  /**
   * ログレベルdebugとしてログの出力をおこなう
   */
  template<typename... As>
  static void debug_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    std::ostringstream stream;
    using List = int[];
    List{ ((stream << args), 0) ... };

    i.debug_ << stream.str() << std::endl;
  }

  /**
   * ログレベルwarnとしてログの出力をおこなう
   */
  template<typename... As>
  static void warn_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    std::ostringstream stream;
    using List = int[];
    List{ ((stream << args), 0) ... };

    i.warn_ << stream.str() << std::endl;
  }

  /**
   * ログレベルerrorとしてログの出力をおこなう
   */
  template<typename... As>
  static void error_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    std::ostringstream stream;
    using List = int[];
    List{ ((stream << args), 0) ... };

    i.error_ << stream.str() << std::endl;
  }
  
  /**
   * ログレベルfatalとしてログの出力をおこなう
   */
  template<typename... As>
  static void fatal_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    std::ostringstream stream;
    using List = int[];
    List{ ((stream << args), 0) ... };

    i.fatal_ << stream.str() << std::endl;
  }

private:
  Logger();
  Logger(const Logger&);
  Logger& operator=(const Logger&); 

  LogLevel log_level_;
  bool is_html_mode = false;

  boost::iostreams::filtering_ostream debug_;
  boost::iostreams::filtering_ostream warn_;
  boost::iostreams::filtering_ostream error_;
  boost::iostreams::filtering_ostream fatal_;
};  

#define HYDLA_LOGGER_DEBUG_INTERNAL(...)  hydla::logger::Logger::debug_write(__VA_ARGS__)

/**
 * ログレベルdebugでのログの出力
 * append informations for location by default
 */
#define HYDLA_LOGGER_DEBUG(...)                                  \
  HYDLA_LOGGER_DEBUG_INTERNAL("@", __FILE__, " ", __LINE__, " function: ", __FUNCTION__, "   ",  __VA_ARGS__)

/**
 * ログレベルwarnでのログの出力
 */
#define HYDLA_LOGGER_WARN_INTERNAL(...)  hydla::logger::Logger::warn_write(__VA_ARGS__)

#define HYDLA_LOGGER_WARN(...)  HYDLA_LOGGER_WARN_INTERNAL("WARNING: ", __VA_ARGS__)

/**
 * ログレベルerrorでのログの出力
 */
#define HYDLA_LOGGER_ERROR(...)  hydla::logger::Logger::error_write(__VA_ARGS__)

/**
 * ログレベルfatalでのログの出力
 */
#define HYDLA_LOGGER_FATAL(...)  hydla::logger::Logger::fatal_write(__VA_ARGS__)

/**
 * log macro for variables (printed like "(name of variable): (value of var)")
 */
#define HYDLA_LOGGER_DEBUG_VAR(VAR)  HYDLA_LOGGER_DEBUG(#VAR": ", VAR)

} // namespace logger
} // namespace hydla
