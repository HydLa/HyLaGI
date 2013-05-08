#ifndef _INCLUDED_HYDLA_LOGGER_H_
#define _INCLUDED_HYDLA_LOGGER_H_

#include <boost/preprocessor.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace hydla {
namespace logger {

/**
 * ���O�o�͊֐��ɂ����đΉ���������̐�
 */
#define HYDLA_LOGGER_LOG_WRITE_ARG_NUM 10

/**
 * ���O�o�͊֐����̒�`
 */
#define HYDLA_LOGGER_LEFT_SHIFT_ARGS_GEN(z, n, d) \
  << BOOST_PP_CAT(a, n)

/**
 * �w�肳�ꂽ�l�̐��̈����������O�o�͊֐��𐶐�
 */

//d�̗v�f => (�֐���, �t�B���^�ϐ�)
#define HYDLA_LOGGER_DEF_LOG_WRITE_GEN(z, n, d)                   \
  template< BOOST_PP_ENUM_PARAMS(n, typename A) >                 \
  void BOOST_PP_TUPLE_ELEM(2, 0, d)(                              \
    BOOST_PP_ENUM_BINARY_PARAMS(n, const A, &a) ) {               \
    BOOST_PP_TUPLE_ELEM(2, 1, d)                                  \
      BOOST_PP_REPEAT(n, HYDLA_LOGGER_LEFT_SHIFT_ARGS_GEN, _)     \
      << std::endl;                                               \
  }


/**
 * �w�肳�ꂽ���x���̃��O�o�͊֐��𐶐�
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
  /// �v���O�����ǂݍ��݂̏���
  static bool parsing_area_;
  
  /// ����K�w�̍쐬��Ǘ�
  static bool constraint_hierarchy_area_;
  
  /// ��v�Z�̏���
  static bool calculate_closure_area_;
  
  /// PP��IP�̑S�̓I�ȏ����̗���
  static bool phase_area_;
  
  /// ���W���[���W���̑I��
  static bool module_set_area_; 
  
  /// VirtualConstraintSolver���ł̏����D��ɊO���\�t�g�Ƃ̒ʐM����
  static bool vcs_area_;
  
  /// �O���\�t�g�ł̏���
  static bool extern_area_; 

  /// �ǂ��ɂ��Y�����Ȃ����C�o�͂��������́D�ɗ͎g��Ȃ��悤��
  static bool rest_area_;
	
	/// HA�ϊ��̏���
	static bool ha_converter_area_;

  enum LogLevel {
    ParsingArea,
    ConstraintHierarchyArea,
    CalculateClosureArea,
    PhaseArea,
    ModuleSetArea,
    VCSArea,
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
        case VCSArea:
          return vcs_area_;
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
  
  /// Logger��Singleton�ȃC���X�^���X
  static Logger& instance();

  /**
   * ���O���x��debug�Ƃ��ă��O�̏o�͂������Ȃ�
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(debug_write, Debug, debug_)

  /**
   * ���O���x��warn�Ƃ��ă��O�̏o�͂������Ȃ�
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(warn_write,  Warn,  warn_)

  /**
   * ���O���x��error�Ƃ��ă��O�̏o�͂������Ȃ�
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(error_write, Error, error_)
  
  /**
   * ���O���x��fatal�Ƃ��ă��O�̏o�͂������Ȃ�
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
 * ���O���x��debug�ł̃��O�̏o��
 */
#define HYDLA_LOGGER_DEBUG(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Debug, debug_write, (__VA_ARGS__))


/**
 * �\����͎���debug���O�̏o��
 */
#define HYDLA_LOGGER_PARSING(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ParsingArea, debug_write, (__VA_ARGS__))

/**
 * ����K�w�֌W��debug���O�̏o��
 */
#define HYDLA_LOGGER_HIERARCHY(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ConstraintHierarchyArea, debug_write, (__VA_ARGS__))

/**
 * ��v�Z����debug���O�̏o��
 */
#define HYDLA_LOGGER_CLOSURE(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(CalculateClosureArea, debug_write, (__VA_ARGS__))
  
/**
 * �t�F�[�Y���Ƃ�debug���O�̏o��
 */
#define HYDLA_LOGGER_PHASE(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(PhaseArea, debug_write, (__VA_ARGS__))
  
/**
 * ���W���[���W���I������debug���O�̏o��
 */
#define HYDLA_LOGGER_MS(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ModuleSetArea, debug_write, (__VA_ARGS__))
  
/**
 * VCS���p�������N���X��debug���O�̏o��
 */
#define HYDLA_LOGGER_VCS(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(VCSArea, debug_write, (__VA_ARGS__))


/**
 * �O���\�t�g�E�F�A�ɂ��debug���O�̏o��
 */
#define HYDLA_LOGGER_EXTERN(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ExternArea, debug_write, (__VA_ARGS__))


/**
 * ���̑���debug���O�̏o��
 */
#define HYDLA_LOGGER_REST(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(RestArea, debug_write, (__VA_ARGS__))
	
/**
 * HA�ϊ�������debug���O�o��
 */
#define HYDLA_LOGGER_HA(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(HAConverterArea, debug_write, (__VA_ARGS__))

/**
 * ���O���x��warn�ł̃��O�̏o��
 */
#define HYDLA_LOGGER_WARN(...)                                  \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Warn, warn_write, (__VA_ARGS__))

/**
 * ���O���x��error�ł̃��O�̏o��
 */
#define HYDLA_LOGGER_ERROR(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Error, error_write, (__VA_ARGS__))

/**
 * ���O���x��fatal�ł̃��O�̏o��
 */
#define HYDLA_LOGGER_FATAL(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(Fatal, fatal_write, (__VA_ARGS__))


#define HYDLA_LOGGER_FUNC_BEGIN(LEVEL)                                   \
  HYDLA_LOGGER_##LEVEL("*** Begin ", __FUNCTION__, " ", __FILE__ " ", __LINE__, " ***")
  
#define HYDLA_LOGGER_FUNC_END(LEVEL)                                   \
  HYDLA_LOGGER_##LEVEL("*** End ", __FUNCTION__, " ", __FILE__ " ", __LINE__, " ***")

/**
 * print location for this code
 */
#define HYDLA_LOGGER_LOCATION(LEVEL)                                   \
  HYDLA_LOGGER_##LEVEL("%% @", __FUNCTION__, " ", __FILE__ " ", __LINE__)


} // namespace logger
} // namespace hydla

#endif // _INCLUDED_HYDLA_LOGGER_H_
