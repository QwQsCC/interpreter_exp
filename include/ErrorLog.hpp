// 文件：ErrorLog.hpp
// 内容：错误日志模块的接口声明
// 基于spdlog实现，提供错误报告、警告、日志打印等功能

#pragma once

#include "Token.hpp"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <cstdarg>
#include <memory>
#include <string>
#include <vector>


namespace interpreter_exp {
namespace errlog {

// 日志级别
enum class LogLevel { Trace, Debug, Info, Warn, Error, Critical };

// 错误信息结构
struct ErrorInfo {
  std::string message;
  SourceLocation location;
  LogLevel level;
  std::string category; // 错误类别，如"Lexer", "Parser", "Semantic"

  ErrorInfo(const std::string &msg, SourceLocation loc,
            LogLevel lvl = LogLevel::Error, const std::string &cat = "General")
      : message(msg), location(loc), level(lvl), category(cat) {}

  std::string toString() const {
    return "[" + category + "] " + location.toString() + ": " + message;
  }
};

// 错误日志类
class ErrorLog {
public:
  // 获取单例实例
  static ErrorLog &getInstance();

  // 禁止拷贝和赋值
  ErrorLog(const ErrorLog &) = delete;
  ErrorLog &operator=(const ErrorLog &) = delete;

  // 初始化日志系统
  void initialize(const std::string &logFile = "",
                  const std::string &errorFile = "", bool enableConsole = true);

  // 关闭日志系统
  void shutdown();

  // 日志打印方法
  template <typename... Args>
  void trace(const std::string &fmtStr, Args &&...args) {
    if (logger_) {
      logger_->trace(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  void debug(const std::string &fmtStr, Args &&...args) {
    if (logger_) {
      logger_->debug(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  void info(const std::string &fmtStr, Args &&...args) {
    if (logger_) {
      logger_->info(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  void warn(const std::string &fmtStr, Args &&...args) {
    ++warningCount_;
    if (logger_) {
      logger_->warn(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  void error(const std::string &fmtStr, Args &&...args) {
    ++errorCount_;
    if (logger_) {
      logger_->error(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
    if (errorLogger_) {
      errorLogger_->error(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
  }

  template <typename... Args>
  void critical(const std::string &fmtStr, Args &&...args) {
    ++errorCount_;
    if (logger_) {
      logger_->critical(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
    if (errorLogger_) {
      errorLogger_->critical(fmt::runtime(fmtStr), std::forward<Args>(args)...);
    }
  }

  // 带位置信息的错误报告
  void errorAt(const SourceLocation &loc, const std::string &message);
  void errorAt(size_t line, size_t col, const std::string &message);
  void errorAt(size_t line, size_t col, const std::string &desc,
               const std::string &text);

  void warnAt(const SourceLocation &loc, const std::string &message);

  // 错误统计
  size_t getErrorCount() const { return errorCount_; }
  size_t getWarningCount() const { return warningCount_; }
  bool hasErrors() const { return errorCount_ > 0; }
  bool hasWarnings() const { return warningCount_ > 0; }

  // 重置错误计数
  void resetCounts();

  // 设置日志级别
  void setLevel(LogLevel level);

  // 获取所有记录的错误
  const std::vector<ErrorInfo> &getErrors() const { return errors_; }
  const std::vector<ErrorInfo> &getWarnings() const { return warnings_; }

  // 清除记录的错误和警告
  void clearRecords();

  // 记录错误信息（用于后续检索）
  void recordError(const ErrorInfo &err);
  void recordWarning(const ErrorInfo &warn);

private:
  ErrorLog();
  ~ErrorLog();

  std::shared_ptr<spdlog::logger> logger_;      // 主日志器
  std::shared_ptr<spdlog::logger> errorLogger_; // 错误专用日志器

  size_t errorCount_ = 0;
  size_t warningCount_ = 0;

  std::vector<ErrorInfo> errors_;
  std::vector<ErrorInfo> warnings_;

  bool initialized_ = false;
};

// 便捷的全局函数
inline ErrorLog &getErrorLog() { return ErrorLog::getInstance(); }

// 兼容旧接口的静态方法封装
class ErrLog {
public:
  static void reOpen(bool open_it);
  static int error_count();
  static void error_msg(int line, const char *desc, const char *text);
  static void error_msg(int line, int col, const char *desc, const char *text);

  // 支持spdlog格式化的模板方法
  template <typename... Args>
  static void logPrint(const std::string &fmt, Args &&...args) {
    ErrorLog::getInstance().info(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void error(const std::string &fmt, Args &&...args) {
    ErrorLog::getInstance().error(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void error_msg(const std::string &fmt, Args &&...args) {
    ErrorLog::getInstance().error(fmt, std::forward<Args>(args)...);
  }

  template <typename... Args>
  static void warn(const std::string &fmt, Args &&...args) {
    ErrorLog::getInstance().warn(fmt, std::forward<Args>(args)...);
  }
};

} // namespace errlog
} // namespace interpreter_exp
