#ifndef _INCLUDED_HYDLA_LOGGER_H_
#define _INCLUDED_HYDLA_LOGGER_H_

#include <boost/preprocessor.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace hydla {
namespace logger {

/**
 * ログ出力関数において対応する引数の数
 */
#define HYDLA_LOGGER_LOG_WRITE_ARG_NUM 10

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
  /// プログラム読み込みの処理
  static bool parsing_area_;
  
  /// 制約階層の作成や管理
  static bool constraint_hierarchy_area_;
  
  /// 閉包計算の処理
  static bool calculate_closure_area_;
  
  /// PPやIPの全体的な処理の流れ
  static bool phase_area_;
  
  /// モジュール集合の選択
  static bool module_set_area_; 
  
  /// Backendモジュール内での処理
  static bool backend_area_;
  
  /// 外部ソフトでの処理
  static bool extern_area_; 

  /// どこにも該当しないが，出力したいもの．極力使わないように
  static bool rest_area_;
	
	/// HA変換の処理
	static bool ha_converter_area_;

  enum LogLevel {
    ParsingArea,
    ConstraintHierarchyArea,
    CalculateClosureArea,
    PhaseArea,
    ModuleSetArea,
    BackendArea,
    ExternArea,
    RestArea,
  	HAConverterArea,
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

  bool valid_level(LogLevel level)
  {
    if(log_level_ == Debug){
      switch(level){
        case ParsingArea:
          return parsing_area_;
        case ConstraintHierarchyArea:
          return constraint_hierarchy_area_;
        case CalculateClosureArea:
          return calculate_closure_area_;
        case ModuleSetArea:
          return module_set_area_;
        case PhaseArea:
          return phase_area_;
        case BackendArea:
          return backend_area_;
        case ExternArea:
          return extern_area_;
        case RestArea:
          return rest_area_;
      case HAConverterArea:
      		return ha_converter_area_;
        default:
          return log_level_ <= level;
      }
    } else {
      return log_level_ <= level;
    }
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


/**
 * ログレベルdebugでのログの出力
 */
#define HYDLA_LOGGER_DEBUG(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Debug, debug_write, (__VA_ARGS__))


/**
 * 構文解析時のdebugログの出力
 */
#define HYDLA_LOGGER_PARSING(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ParsingArea, debug_write, (__VA_ARGS__))

/**
 * 制約階層関係のdebugログの出力
 */
#define HYDLA_LOGGER_HIERARCHY(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ConstraintHierarchyArea, debug_write, (__VA_ARGS__))

/**
 * 閉包計算時のdebugログの出力
 */
#define HYDLA_LOGGER_CLOSURE(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(CalculateClosureArea, debug_write, (__VA_ARGS__))
  
/**
 * フェーズごとのdebugログの出力
 */
#define HYDLA_LOGGER_PHASE(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(PhaseArea, debug_write, (__VA_ARGS__))
  
/**
 * モジュール集合選択時のdebugログの出力
 */
#define HYDLA_LOGGER_MS(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ModuleSetArea, debug_write, (__VA_ARGS__))
  
/**
 * Backend内でのdebugログの出力
 */
#define HYDLA_LOGGER_BACKEND(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(BackendArea, debug_write, (__VA_ARGS__))


/**
 * 外部ソフトウェアによるdebugログの出力
 */
#define HYDLA_LOGGER_EXTERN(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ExternArea, debug_write, (__VA_ARGS__))


/**
 * その他のdebugログの出力
 */
#define HYDLA_LOGGER_REST(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(RestArea, debug_write, (__VA_ARGS__))
	
/**
 * HA変換処理のdebugログ出力
 */
#define HYDLA_LOGGER_HA(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(HAConverterArea, debug_write, (__VA_ARGS__))

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


#define HYDLA_LOGGER_FUNC_BEGIN(LEVEL)                                   \
  HYDLA_LOGGER_##LEVEL("*** Begin ", __FUNCTION__, " ", __FILE__ " ", __LINE__, " ***")
  
#define HYDLA_LOGGER_FUNC_END(LEVEL)                                   \
  HYDLA_LOGGER_##LEVEL("*** End ", __FUNCTION__, " ", __FILE__ " ", __LINE__, " ***")

/**
 * print location where this code is
 */
#define HYDLA_LOGGER_LOCATION(LEVEL)                                   \
  HYDLA_LOGGER_##LEVEL("%% @", __FUNCTION__, " ", __FILE__ " ", __LINE__)


} // namespace logger
} // namespace hydla

#endif // _INCLUDED_HYDLA_LOGGER_H_
