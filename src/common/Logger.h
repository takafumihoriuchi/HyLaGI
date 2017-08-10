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
    Standard,
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
    i.debug_ << i.format(args...) << std::endl;
  }

  /**
   * ログレベルwarnとしてログの出力をおこなう
   */
  template<typename... As>
  static void warn_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    i.warn_ << i.format(args...) << std::endl;
  }

  /**
   * ログレベルerrorとしてログの出力をおこなう
   */
  template<typename... As>
  static void error_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    i.error_ << i.format(args...) << std::endl;
  }
  
  /**
   * ログレベルfatalとしてログの出力をおこなう
   */
  template<typename... As>
  static void fatal_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    i.fatal_ << i.format(args...) << std::endl;
  }

  /**
  * ログレベルstandardとしてログの出力をおこなう
  */
  template<typename... As>
  static void standard_write(const As&... args) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    i.standard_ << i.format(args...) << std::endl;
  }

  static void debug_write_row(const std::string& str) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    i.debug_ << str << std::endl;
  }

  static bool is_html_mode() {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    return i.html_mode;
  }

  static void set_html_mode(bool enabled) {
    hydla::logger::Logger& i = hydla::logger::Logger::instance();
    i.html_mode = enabled;
  }

private:

  template<typename... As>
  std::string format(const As&... args) const {
    std::stringstream stream;
    using List = int[];
    List{ ((stream << args), 0) ... };

    if (!html_mode) {
      return stream.str();
    }

    const std::string old_str(std::move(stream.str()));
    std::string new_str;
    new_str.reserve(old_str.length());

    for (const char* data = old_str.data(); *data != '\0'; ++data) {
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
  Logger(const Logger&);
  Logger& operator=(const Logger&); 

  LogLevel log_level_;
  bool html_mode = false;

  boost::iostreams::filtering_ostream debug_;
  boost::iostreams::filtering_ostream warn_;
  boost::iostreams::filtering_ostream error_;
  boost::iostreams::filtering_ostream fatal_;
  boost::iostreams::filtering_ostream standard_;
};

class Detail
{
public:
  Detail(const std::string& summary) {
    if (Logger::is_html_mode()) {
      Logger::debug_write_row(std::string("<details><summary>") + summary + "</summary>\n<div style=\"padding-left:1em\">\n");
    }
  }

  ~Detail() {
    if (Logger::is_html_mode()) {
      Logger::debug_write_row("</div>\n</details>\n");
    }
  }
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
 * ログレベルstandardでのログの出力
 */
#define HYDLA_LOGGER_STANDARD(...)  hydla::logger::Logger::standard_write(__VA_ARGS__)

/**
 * log macro for variables (printed like "(name of variable): (value of var)")
 */
#define HYDLA_LOGGER_DEBUG_VAR(VAR)  HYDLA_LOGGER_DEBUG(#VAR": ", VAR)

} // namespace logger
} // namespace hydla
