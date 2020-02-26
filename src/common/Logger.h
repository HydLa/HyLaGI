#pragma once

#include "Timer.h"
#include <boost/iostreams/filtering_stream.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace hydla {
namespace logger {

/*! @class Logger
    @brief
    ログ出力を行うクラスです。

    実行時にhtmlオプションを付けることにより、ログをHTML形式で出力することができます。

    HTMLモードでは、ログに階層構造に関する情報が付加されます。

    以下に、HTMLモードの出力でそれぞれのタブが中に持っている情報について説明をします。

    |タブ|中身の説明|
    |:--|:--------|
    |parse|パース|
    |call_timer.get_elapsed_us()|時間計測|
    |dfs|1フェーズ全体の計算|
    |make_results_from_todo|フェーズのシミュレーション|
    |calculate_closure|解軌道などの計算|
    |make_next_todo|離散変化時刻などの計算|
    |find_min_time|ガードの最小変化時刻の計算|
    |find_min_time_step_by_step|ガードの最小変化時刻の計算|
    |output_result|全体結果の出力|
*/
class Logger {
public:
  enum LogLevel { Debug, Warn, Error, Fatal, Standard, None };

  ~Logger();

  LogLevel set_log_level(LogLevel l) {
    LogLevel old = log_level_;
    log_level_ = l;
    return old;
  }

  LogLevel get_log_level() const { return log_level_; }

  bool valid_level(LogLevel level) const { return log_level_ <= level; }

  /// LoggerのSingletonなインスタンス
  static Logger &instance();

  /**
   * ログレベルdebugとしてログの出力をおこなう
   */
  template <typename... As> static void debug_write(const As &... args) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    if (i.valid_level(LogLevel::Debug)) {
      i.debug_ << i.format(args...) << std::endl;
    }
  }

  /**
   * ログレベルwarnとしてログの出力をおこなう
   */
  template <typename... As> static void warn_write(const As &... args) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    if (i.valid_level(LogLevel::Warn)) {
      i.warn_ << i.format(args...) << std::endl;
    }
  }

  /**
   * ログレベルerrorとしてログの出力をおこなう
   */
  template <typename... As> static void error_write(const As &... args) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    if (i.valid_level(LogLevel::Error)) {
      i.error_ << i.format(args...) << std::endl;
    }
  }

  /**
   * ログレベルfatalとしてログの出力をおこなう
   */
  template <typename... As> static void fatal_write(const As &... args) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    if (i.valid_level(LogLevel::Fatal)) {
      i.fatal_ << i.format(args...) << std::endl;
    }
  }

  /**
   * ログレベルstandardとしてログの出力をおこなう
   */
  template <typename... As> static void standard_write(const As &... args) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    if (i.valid_level(LogLevel::Standard)) {
      i.standard_ << i.format(args...) << std::endl;
    }
  }

  static void debug_write_row(const std::string &str) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    if (i.valid_level(LogLevel::Debug)) {
      i.debug_ << str << std::endl;
    }
  }

  static void debug_write_timer(const hydla::timer::Timer &timer) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    if (i.html_mode) {
      i.debug_
          << R"(timer elapsed: <span class="timer" style="background-color:#f09b3b">)"
          << timer.get_elapsed_us() << "</span>[us]<br>" << std::endl;
    } else {
      if (i.valid_level(LogLevel::Debug)) {
        i.debug_ << "timer elapsed: " << timer.get_elapsed_us() << "[us]"
                 << std::endl;
      }
    }
  }

  static bool is_html_mode() {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    return i.html_mode;
  }

  static void set_html_mode(bool enabled) {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();
    i.html_mode = enabled;
  }

  static void initialize() {
    hydla::logger::Logger &i = hydla::logger::Logger::instance();

    if (!i.is_html_mode()) {
      return;
    }

    std::string defaultStyle(R"(
html {
  height: 100%;
}
body {
  margin: 0;
  height: 100%;
}
.ps__rail-y {
  opacity: 1.0!important;
  width: 15px!important;
  background-size:100% 100% !important;
}
.ps__thumb-y {
  width: 10px!important;
}
details {
  background: #FFF;
  margin: 7px 0;
  padding: 0 0 1px 0;
  border: solid #00a8ff;
  border-width: 0px 0px 1px 2px;
}
summary {
  background: #00a8ff;
  color: #FFF;
  font-weight: bold;
  padding: 3px;
  margin: 0 0px 5px -2px;
}
.bottomButton {
  position: fixed;
  z-index: 99;
  font-size: 18px;
  border: none;
  outline: none;
  background-color: #333;
  color: #f2f2f2;
  cursor: pointer;
  padding: 15px;
}
.bottomButton:hover {
  background: #ddd;
  color: black;
}
#expandButton {
  bottom: 20px;
  right: 30px;
}
#closeButton {
  bottom: 72px;
  right: 30px;
})");

    i.debug_ << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\" />"
             << std::endl;
    i.debug_ << std::string("<style>\n") + defaultStyle + "</style>"
             << std::endl;
    i.debug_ << "</head>\n<body>" << std::endl;
    i.debug_
        << R"*(<button onclick="openAll()" class="bottomButton" id="expandButton">Expand all</button>)*"
        << std::endl;
    i.debug_
        << R"*(<button onclick="closeAll()" class="bottomButton" id="closeButton">Close all</button>)*"
        << std::endl;
    i.debug_ << R"(<div class="article">)" << std::endl;
  }

private:
  template <typename... As> std::string format(const As &... args) const {
    std::stringstream stream;

    using swallow = std::initializer_list<int>;
    (void)swallow{(void(stream << args), 0)...};

    if (!html_mode) {
      return stream.str();
    }

    const std::string old_str(stream.str());
    std::string new_str;
    new_str.reserve(old_str.length());

    for (const char *data = old_str.data(); *data != '\0'; ++data) {
      switch (*data) {
      case '\n':
        new_str.append("<br>\n");
        break;
      case '\"':
        new_str.append("&quot;");
        break;
      case '&':
        new_str.append("&amp;");
        break;
      case '\'':
        new_str.append("&apos;");
        break;
      case '<':
        new_str.append("&lt;");
        break;
      case '>':
        new_str.append("&gt;");
        break;
      default:
        new_str.push_back(*data);
        break;
      }
    }

    new_str.append("<br>");

    return new_str;
  }

  Logger();
  Logger(const Logger &);
  Logger &operator=(const Logger &);

  LogLevel log_level_;
  bool html_mode = false;

  boost::iostreams::filtering_ostream debug_;
  boost::iostreams::filtering_ostream warn_;
  boost::iostreams::filtering_ostream error_;
  boost::iostreams::filtering_ostream fatal_;
  boost::iostreams::filtering_ostream standard_;
};

class Detail {
public:
  Detail(const std::string &summary) {
    if (Logger::is_html_mode()) {
      Logger::debug_write_row(
          std::string(R"*(<details ontoggle="reloadMakers()"><summary>)*") +
          summary + "</summary>\n<div style=\"padding-left:1em\">\n");
    }
  }

  ~Detail() {
    if (Logger::is_html_mode()) {
      Logger::debug_write_row("</div>\n</details>\n");
    }
  }
};

#define HYDLA_LOGGER_DEBUG_INTERNAL(...)                                       \
  { hydla::logger::Logger::debug_write(__VA_ARGS__); }

/**
 * ログレベルdebugでのログの出力
 * append informations for location by default
 */
#define HYDLA_LOGGER_DEBUG(...)                                                \
  HYDLA_LOGGER_DEBUG_INTERNAL("@", __FILE__, " ", __LINE__,                    \
                              " function: ", __FUNCTION__, "   ", __VA_ARGS__)

/**
 * ログレベルwarnでのログの出力
 */
#define HYDLA_LOGGER_WARN_INTERNAL(...)                                        \
  { hydla::logger::Logger::warn_write(__VA_ARGS__); }

#define HYDLA_LOGGER_WARN(...)                                                 \
  HYDLA_LOGGER_WARN_INTERNAL("WARNING: ", __VA_ARGS__)

/**
 * ログレベルerrorでのログの出力
 */
#define HYDLA_LOGGER_ERROR(...)                                                \
  { hydla::logger::Logger::error_write(__VA_ARGS__); }

/**
 * ログレベルfatalでのログの出力
 */
#define HYDLA_LOGGER_FATAL(...)                                                \
  { hydla::logger::Logger::fatal_write(__VA_ARGS__); }

/**
 * ログレベルstandardでのログの出力
 */
#define HYDLA_LOGGER_STANDARD(...)                                             \
  { hydla::logger::Logger::standard_write(__VA_ARGS__); }

/**
 * log macro for variables (printed like "(name of variable): (value of var)")
 */
#define HYDLA_LOGGER_DEBUG_VAR(VAR) HYDLA_LOGGER_DEBUG(#VAR ": ", VAR)

} // namespace logger
} // namespace hydla
