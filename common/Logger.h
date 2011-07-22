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
  static bool parsing_area_;
  static bool calculate_closure_area_;
  static bool module_set_area_;
  static bool vcs_area_;
  static bool external_area_; //MathematicaとかREDUCEとかの外部ソフトからの出力
  static bool output_area_;
  static bool rest_area_;  //どこで出したら良いか分からないようなの

  enum LogLevel {
    Debug,
    ParsingArea,
    CalculateClosureArea,
    ModuleSetArea,
    VCSArea,
    ExternalArea,
    OutputArea,
    RestArea,
    Area,   //局所的出力モード
    Summary,//大局的出力モード
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
    if(log_level_ == Area){
      switch(level){
        case ParsingArea:
          return parsing_area_;
        case CalculateClosureArea:
          return calculate_closure_area_;
        case ModuleSetArea:
          return module_set_area_;
        case VCSArea:
          return vcs_area_;
        case ExternalArea:
          return external_area_;
        case OutputArea:
          return output_area_;
        case RestArea:
          return rest_area_;
        default:
          return level != Summary && log_level_ <= level;
      }
    } else {
      return log_level_ <= level;
    }
  }

  static Logger& instance();

  /**
   * ログレベルareaとしてログの出力をおこなう
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(area_write, Area, area_)

  /**
   * ログレベルdebugとしてログの出力をおこなう
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(debug_write, Debug, debug_)

  /**
   * ログレベルsummary_debugとしてログの出力をおこなう
   */
  HYDLA_LOGGER_DEF_LOG_WRITE(summary_write, Summary, summary_)

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

  boost::iostreams::filtering_ostream area_; //局所的出力モード
  boost::iostreams::filtering_ostream debug_;
  boost::iostreams::filtering_ostream summary_; //大局的出力モード
  boost::iostreams::filtering_ostream warn_;
  boost::iostreams::filtering_ostream error_;
  boost::iostreams::filtering_ostream fatal_;
};  


#define HYDLA_LOGGER_LOG_WRITE_MACRO(LEVEL, FUNC, ARGS) {             \
    hydla::logger::Logger& logger=hydla::logger::Logger::instance();  \
    if(logger.is_valid_level(hydla::logger::Logger::LEVEL)) {         \
      logger. FUNC ARGS;                                              \
    }                                                                 \
  }

#define HYDLA_LOGGER_LOG_WRITE_MACRO_SUMMARY(LEVEL, FUNC, ARGS) {     \
    hydla::logger::Logger& logger=hydla::logger::Logger::instance();  \
    if(logger.is_valid_level(hydla::logger::Logger::Summary)||        \
    logger.is_valid_level(hydla::logger::Logger::LEVEL)) {            \
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
  HYDLA_LOGGER_LOG_WRITE_MACRO(ParsingArea, area_write, (__VA_ARGS__))


/**
 * 閉包計算時のdebugログの出力
 */
#define HYDLA_LOGGER_CC(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(CalculateClosureArea, area_write, (__VA_ARGS__))
  
/**
 * モジュール集合選択時のdebugログの出力
 */
#define HYDLA_LOGGER_MS(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ModuleSetArea, area_write, (__VA_ARGS__))
  
/**
 * モジュール集合選択時のdebugログの出力with Summary
 */
#define HYDLA_LOGGER_MS_SUMMARY(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO_SUMMARY(ModuleSetArea, area_write, (__VA_ARGS__))


/**
 * VCSを継承したクラスのdebugログの出力with Summary
 */
#define HYDLA_LOGGER_VCS_SUMMARY(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO_SUMMARY(VCSArea, area_write, (__VA_ARGS__))

/**
 * VCSを継承したクラスのdebugログの出力
 */
#define HYDLA_LOGGER_VCS(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(VCSArea, area_write, (__VA_ARGS__))

/**
 * 外部ソフトウェアによるdebugログの出力
 */
#define HYDLA_LOGGER_EXTERN(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(ExternalArea, area_write, (__VA_ARGS__))


/**
 * HydLa出力処理関連のdebugログの出力
 */
#define HYDLA_LOGGER_OUTPUT(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(OutputArea, area_write, (__VA_ARGS__))


/**
 * その他のdebugログの出力with Summary
 */
#define HYDLA_LOGGER_REST_SUMMARY(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO_SUMMARY(RestArea, area_write, (__VA_ARGS__))

/**
 * その他のdebugログの出力
 */
#define HYDLA_LOGGER_REST(...)                                   \
  HYDLA_LOGGER_LOG_WRITE_MACRO(RestArea, area_write, (__VA_ARGS__))

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


} // namespace logger
} // namespace hydla

#endif // _INCLUDED_HYDLA_LOGGER_H_
