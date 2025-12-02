// Draw语言完整解释器应用
// 整合词法分析器、语法分析器、语义分析器和UI

#pragma once

#include "DrawLangAST.hpp"
#include "DrawLangLexer.hpp"
#include "DrawLangParser.hpp"
#include "DrawLangSemantic.hpp"
#include "DrawLangUI.hpp"
#include "ErrorLog.hpp"
#include <functional>
#include <memory>
#include <string>

namespace interpreter_exp {

// Draw语言解释器应用
// 注意：与semantic::DrawLangInterpreter不同，这个类是应用程序级别的封装

class DrawLangApp {
public:
  // 单例模式
  static DrawLangApp &getInstance();

  // 禁止拷贝
  DrawLangApp(const DrawLangApp &) = delete;
  DrawLangApp &operator=(const DrawLangApp &) = delete;

  // 配置
  struct Config {
    bool enableDebugOutput = false; // 调试输出
    bool enableDemoMode = false;    // 演示模式（延迟绘制）
    bool traceExecution = false;    // 跟踪执行
    lexer::DrawLangDFAType dfaType =
        lexer::DrawLangDFAType::TableDriven; // DFA类型
  };

  void setConfig(const Config &config);
  const Config &getConfig() const { return config_; }

  // UI管理

  // 设置UI对象
  void setUI(ui::DrawLangUI *ui);

  // 获取当前UI
  ui::DrawLangUI *getUI() const { return ui_; }

  // 解释执行

  // 从文件解释执行
  int interpretFile(const std::string &filePath);

  // 从字符串解释执行
  int interpretString(const std::string &source,
                      const std::string &sourceName = "string");

  // 重新执行上一次的文件
  int reinterpret();

  // 文件管理

  // 获取当前源文件路径
  const std::string &getSourceFilePath() const { return sourceFilePath_; }

  // 运行状态

  bool isRunning() const { return isRunning_; }
  int getErrorCount() const { return errorCount_; }

private:
  DrawLangApp();
  ~DrawLangApp();

  // 内部解释执行方法
  int doInterpret(lexer::DrawLangLexer *lexer);

  // 绘图回调
  void onDrawPixel(double x, double y, const semantic::PixelAttribute &attr);

private:
  Config config_;
  ui::DrawLangUI *ui_ = nullptr;

  std::string sourceFilePath_;
  bool isRunning_ = false;
  int errorCount_ = 0;

  // 组件实例（用于重新执行）
  std::unique_ptr<lexer::DrawLangLexer> lastLexer_;
  std::unique_ptr<parser::DrawLangParser> lastParser_;
  std::unique_ptr<semantic::DrawLangSemanticAnalyzer> lastSemantic_;
};
// 获取应用实例
inline DrawLangApp &getApp() { return DrawLangApp::getInstance(); }

} // namespace interpreter_exp
