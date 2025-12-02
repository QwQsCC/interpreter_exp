// 文件：ErrorLog.cpp
// 内容：错误日志模块的实现
// 基于spdlog实现

#include "ErrorLog.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <cstdarg>
#include <cstdio>


namespace interpreter_exp {
namespace errlog {

// ============================================================================
// ErrorLog 实现
// ============================================================================

ErrorLog &ErrorLog::getInstance() {
  static ErrorLog instance;
  return instance;
}

ErrorLog::ErrorLog() {
  // 默认初始化：创建控制台日志器
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");

    logger_ = std::make_shared<spdlog::logger>("interpreter", console_sink);
    logger_->set_level(spdlog::level::debug);

    spdlog::register_logger(logger_);
    initialized_ = true;
  } catch (const spdlog::spdlog_ex &ex) {
    std::fprintf(stderr, "Log initialization failed: %s\n", ex.what());
  }
}

ErrorLog::~ErrorLog() { shutdown(); }

void ErrorLog::initialize(const std::string &logFile,
                          const std::string &errorFile, bool enableConsole) {
  try {
    std::vector<spdlog::sink_ptr> sinks;

    // 控制台输出
    if (enableConsole) {
      auto console_sink =
          std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      console_sink->set_level(spdlog::level::debug);
      console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");
      sinks.push_back(console_sink);
    }

    // 日志文件
    if (!logFile.empty()) {
      auto file_sink =
          std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile, true);
      file_sink->set_level(spdlog::level::trace);
      file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
      sinks.push_back(file_sink);
    }

    // 创建主日志器
    logger_ = std::make_shared<spdlog::logger>("interpreter", sinks.begin(),
                                               sinks.end());
    logger_->set_level(spdlog::level::trace);
    spdlog::register_logger(logger_);

    // 错误专用日志器
    if (!errorFile.empty()) {
      auto error_sink =
          std::make_shared<spdlog::sinks::basic_file_sink_mt>(errorFile, true);
      error_sink->set_level(spdlog::level::warn);
      error_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

      errorLogger_ = std::make_shared<spdlog::logger>("errors", error_sink);
      errorLogger_->set_level(spdlog::level::warn);
      spdlog::register_logger(errorLogger_);
    }

    initialized_ = true;

  } catch (const spdlog::spdlog_ex &ex) {
    std::fprintf(stderr, "Log initialization failed: %s\n", ex.what());
  }
}

void ErrorLog::shutdown() {
  if (initialized_) {
    spdlog::drop_all();
    logger_.reset();
    errorLogger_.reset();
    initialized_ = false;
  }
}

void ErrorLog::errorAt(const SourceLocation &loc, const std::string &message) {
  ++errorCount_;
  std::string fullMsg =
      fmt::format("Error at [{}:{}]: {}", loc.line, loc.column, message);
  if (logger_) {
    logger_->error(fullMsg);
  }
  if (errorLogger_) {
    errorLogger_->error(fullMsg);
  }

  // 记录错误
  errors_.emplace_back(message, loc, LogLevel::Error, "General");
}

void ErrorLog::errorAt(size_t line, size_t col, const std::string &message) {
  SourceLocation loc;
  loc.line = line;
  loc.column = col;
  errorAt(loc, message);
}

void ErrorLog::errorAt(size_t line, size_t col, const std::string &desc,
                       const std::string &text) {
  SourceLocation loc;
  loc.line = line;
  loc.column = col;
  errorAt(loc, desc + " " + text);
}

void ErrorLog::warnAt(const SourceLocation &loc, const std::string &message) {
  ++warningCount_;
  std::string fullMsg =
      fmt::format("Warning at [{}:{}]: {}", loc.line, loc.column, message);
  if (logger_) {
    logger_->warn(fullMsg);
  }

  // 记录警告
  warnings_.emplace_back(message, loc, LogLevel::Warn, "General");
}

void ErrorLog::resetCounts() {
  errorCount_ = 0;
  warningCount_ = 0;
}

void ErrorLog::setLevel(LogLevel level) {
  if (!logger_)
    return;

  spdlog::level::level_enum spdLevel;
  switch (level) {
  case LogLevel::Trace:
    spdLevel = spdlog::level::trace;
    break;
  case LogLevel::Debug:
    spdLevel = spdlog::level::debug;
    break;
  case LogLevel::Info:
    spdLevel = spdlog::level::info;
    break;
  case LogLevel::Warn:
    spdLevel = spdlog::level::warn;
    break;
  case LogLevel::Error:
    spdLevel = spdlog::level::err;
    break;
  case LogLevel::Critical:
    spdLevel = spdlog::level::critical;
    break;
  default:
    spdLevel = spdlog::level::info;
  }

  logger_->set_level(spdLevel);
}

void ErrorLog::clearRecords() {
  errors_.clear();
  warnings_.clear();
}

void ErrorLog::recordError(const ErrorInfo &err) {
  errors_.push_back(err);
  ++errorCount_;
}

void ErrorLog::recordWarning(const ErrorInfo &warn) {
  warnings_.push_back(warn);
  ++warningCount_;
}

// ============================================================================
// ErrLog 兼容接口实现
// ============================================================================

void ErrLog::reOpen(bool open_it) {
  auto &log = ErrorLog::getInstance();
  if (open_it) {
    log.resetCounts();
    log.clearRecords();
    // 可以在这里重新初始化文件
  }
}

int ErrLog::error_count() {
  return static_cast<int>(ErrorLog::getInstance().getErrorCount());
}

void ErrLog::error_msg(int line, const char *desc, const char *text) {
  ErrorLog::getInstance().errorAt(static_cast<size_t>(line), 0, desc, text);
}

void ErrLog::error_msg(int line, int col, const char *desc, const char *text) {
  ErrorLog::getInstance().errorAt(static_cast<size_t>(line),
                                  static_cast<size_t>(col), desc, text);
}

} // namespace errlog
} // namespace interpreter_exp
