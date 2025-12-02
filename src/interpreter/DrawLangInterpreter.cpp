// 文件：DrawLangInterpreter.cpp
// 内容：Draw语言完整解释器应用实现
// 整合词法分析器、语法分析器、语义分析器和UI

#include "DrawLangInterpreter.hpp"
#include "ErrorLog.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace interpreter_exp {

using namespace lexer;
using namespace parser;
using namespace semantic;
using namespace ast;
using namespace ui;
using namespace errlog;

// ============================================================================
// DrawLangApp 实现
// ============================================================================

DrawLangApp &DrawLangApp::getInstance() {
  static DrawLangApp instance;
  return instance;
}

DrawLangApp::DrawLangApp() {
  // 初始化错误日志
  // ErrorLog::getInstance().initialize("", "", true);
}

DrawLangApp::~DrawLangApp() {
  // 清理资源
}

void DrawLangApp::setConfig(const Config &config) { config_ = config; }

void DrawLangApp::setUI(DrawLangUI *ui) {
  ui_ = ui;

  // 设置UI的解释回调
  if (ui_) {
    ui_->setInterpretCallback([this](const std::string &filePath) {
      if (!filePath.empty()) {
        interpretFile(filePath);
      }
    });
  }
}

int DrawLangApp::interpretFile(const std::string &filePath) {
  sourceFilePath_ = filePath;

  // 重置错误计数
  ErrorLog::getInstance().resetCounts();
  errorCount_ = 0;

  // 显示开始信息
  if (ui_) {
    ui_->showMessage(0, "Loading file: " + filePath);
    ui_->clearCanvas();
  }

  // 读取文件内容
  std::ifstream file(filePath);
  if (!file.is_open()) {
    std::string errorMsg = "Failed to open file: " + filePath;
    ErrLog::error(errorMsg);
    if (ui_) {
      ui_->showMessage(1, errorMsg);
      ui_->setStatus("Error: Cannot open file");
    }
    return 1;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();
  file.close();

  if (ui_) {
    ui_->showMessage(0, "File loaded, starting interpretation...");
  }

  // 创建词法分析器
  auto lexer = createDrawLangLexerFromString(source, config_.dfaType, filePath);

  return doInterpret(lexer.get());
}

int DrawLangApp::interpretString(const std::string &source,
                                 const std::string &sourceName) {
  sourceFilePath_ = sourceName;

  // 重置错误计数
  ErrorLog::getInstance().resetCounts();
  errorCount_ = 0;

  if (ui_) {
    ui_->showMessage(0, "Interpreting from string: " + sourceName);
    ui_->clearCanvas();
  }

  // 创建词法分析器
  auto lexer =
      createDrawLangLexerFromString(source, config_.dfaType, sourceName);

  return doInterpret(lexer.get());
}

int DrawLangApp::reinterpret() {
  if (sourceFilePath_.empty()) {
    if (ui_) {
      ui_->showMessage(1, "No file to reinterpret.");
    }
    return 1;
  }

  return interpretFile(sourceFilePath_);
}

int DrawLangApp::doInterpret(DrawLangLexer *lexer) {
  isRunning_ = true;

  if (ui_) {
    ui_->setStatus("Parsing...");
  }

  try {
    // 创建语法分析器
    DrawLangParser parser(lexer);

    // 配置
    DrawParserConfig parserConfig;
    parserConfig.traceParsing = config_.traceExecution;
    parserConfig.recoverFromErrors = true;
    parser.setConfig(parserConfig);

    // 创建语义分析器
    DrawLangSemanticAnalyzer semantic(&parser);

    // 配置
    SemanticConfig semConfig;
    semConfig.enableDebugOutput = config_.enableDebugOutput;
    semConfig.enableDemoMode = config_.enableDemoMode;
    semantic.setConfig(semConfig);

    // 设置绘图回调
    semantic.setDrawCallback(
        [this](double x, double y, const semantic::PixelAttribute &attr) {
          onDrawPixel(x, y, attr);
        });

    // 现在解析 - ParamExprNode将使用semantic的tStorage_
    auto program = parser.parse();

    if (!program) {
      errorCount_ = ErrLog::error_count();
      if (ui_) {
        ui_->showMessage(1, "Parsing failed.");
        ui_->setStatus("Parse Error");
      }
      isRunning_ = false;
      return errorCount_;
    }

    // 获取解析错误
    const auto &parseErrors = parser.getErrors();
    if (!parseErrors.empty()) {
      for (const auto &err : parseErrors) {
        if (ui_) {
          std::ostringstream oss;
          oss << "[" << err.location.line << ":" << err.location.column << "] "
              << err.message;
          ui_->showMessage(1, oss.str());
        }
      }
    }

    if (ui_) {
      ui_->setStatus("Executing...");
      ui_->showMessage(0, "Parsing completed. Executing...");
    }

    // 执行
    int result = semantic.run(program.get());

    errorCount_ = ErrLog::error_count();

    if (ui_) {
      if (result == 0 && errorCount_ == 0) {
        ui_->showMessage(0, "Execution completed successfully.");
        ui_->setStatus("Completed");
      } else {
        std::ostringstream oss;
        oss << "Execution completed with " << errorCount_ << " error(s).";
        ui_->showMessage(1, oss.str());
        ui_->setStatus("Completed with errors");
      }
      ui_->refresh();
    }

    isRunning_ = false;
    return errorCount_;

  } catch (const std::exception &e) {
    errorCount_++;
    std::string errorMsg = std::string("Exception: ") + e.what();
    ErrLog::error(errorMsg);
    if (ui_) {
      ui_->showMessage(1, errorMsg);
      ui_->setStatus("Error");
    }
    isRunning_ = false;
    return 1;
  }
}

void DrawLangApp::onDrawPixel(double x, double y,
                              const semantic::PixelAttribute &attr) {
  if (ui_) {
    ui::PixelAttribute uiAttr(attr.r, attr.g, attr.b,
                              static_cast<int>(attr.size));
    ui_->drawPixel(static_cast<int>(x), static_cast<int>(y), uiAttr);
  }
}

} // namespace interpreter_exp
