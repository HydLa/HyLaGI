#pragma once

#include <iostream>
#include <boost/iostreams/filtering_stream.hpp>

namespace hydla {
namespace logger {

/**
 * ログ出力関数において対応する引数の数
 */
#define HYDLA_LOGGER_LOG_WRITE_ARG_NUM 20

/**
 * ログ出力関数内の定義
 */
#define HYDLA_LOGGER_LEFT_SHIFT_ARGS_GEN(z, n, d) \
  << BOOST_PP_CAT(a, n)

/**
 * 指定された値の数の引数をもつログ出力関数を生成
 */

//dの要素 => (関数名, フィルタ変数)
#define HYDLA_LOGGER_DEF_LOG_WRITE_GEN(z, n, d)                   \
  template< BOOST_PP_ENUM_PARAMS(n, typename A) >                 \
  void BOOST_PP_TUPLE_ELEM(2, 0, d)(                              \
    BOOST_PP_ENUM_BINARY_PARAMS(n, const A, &a) ) {               \
    BOOST_PP_TUPLE_ELEM(2, 1, d)                                  \
      BOOST_PP_REPEAT(n, HYDLA_LOGGER_LEFT_SHIFT_ARGS_GEN, _)     \
      << std::endl;                                               \
  }


/**
 * 指定されたレベルのログ出力関数を生成
 */
#define HYDLA_LOGGER_DEF_LOG_WRITE(FUNC, LEVEL, FILTER) \
  BOOST_PP_REPEAT_FROM_TO(                              \
    1,                                                  \
    BOOST_PP_INC(HYDLA_LOGGER_LOG_WRITE_ARG_NUM),       \
    HYDLA_LOGGER_DEF_LOG_WRITE_GEN,                     \
    (FUNC, FILTER))



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
  HYDLA_LOGGER_DEF_LOG_WRITE(debug_write, Debug, debug_)

  /**
   * ログレベルwarnとしてログの出力をおこなう
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(warn_write,  Warn,  warn_)

  /**
   * ログレベルerrorとしてログの出力をおこなう
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(error_write, Error, error_)
  
  /**
   * ログレベルfatalとしてログの出力をおこなう
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(fatal_write, Fatal, fatal_)

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


#define HYDLA_LOGGER_LOG_WRITE_MACRO(LEVEL, FUNC, ARGS) {             \
    hydla::logger::Logger& logger=hydla::logger::Logger::instance();  \
    if(logger.valid_level(hydla::logger::Logger::LEVEL)) {            \
      logger. FUNC ARGS;                                              \
    }                                                                 \
  }


#define HYDLA_LOGGER_DEBUG_INTERNAL(...)                                    \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Debug, debug_write, (__VA_ARGS__))

/**
 * ログレベルdebugでのログの出力
 * append informations for location by default
 */
#define HYDLA_LOGGER_DEBUG(...)                                  \
  HYDLA_LOGGER_DEBUG_INTERNAL("@", __FILE__, " ", __LINE__, " function: ", __FUNCTION__, "   ",  __VA_ARGS__)


/**
 * ログレベルwarnでのログの出力
 */
#define HYDLA_LOGGER_WARN(...)                                  \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Warn, warn_write, (__VA_ARGS__))

/**
 * ログレベルerrorでのログの出力
 */
#define HYDLA_LOGGER_ERROR(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Error, error_write, (__VA_ARGS__))

/**
 * ログレベルfatalでのログの出力
 */
#define HYDLA_LOGGER_FATAL(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Fatal, fatal_write, (__VA_ARGS__))

/**
 * log macro for variables (printed like "(name of variable): (value of var)")
 */
#define HYDLA_LOGGER_DEBUG_VAR(VAR)                                            \
  HYDLA_LOGGER_DEBUG(#VAR": ", VAR)


} // namespace logger
} // namespace hydla
