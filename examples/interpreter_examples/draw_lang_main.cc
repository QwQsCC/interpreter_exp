// Draw语言解释器主程序
// 使用ImGui界面展示Draw语言绘图结果

#include "DrawLangImGuiUI.hpp"
#include "DrawLangInterpreter.hpp"
#include "ErrorLog.hpp"
#include <cstring>
#include <iostream>
#include <string>

using namespace interpreter_exp;
using namespace interpreter_exp::ui;
using namespace interpreter_exp::errlog;

void printUsage(const char *programName) {
  std::cout << "Draw Language Interpreter" << std::endl;
  std::cout << "Usage: " << programName << " [options] [file]" << std::endl;
  std::cout << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -h, --help     Show this help message" << std::endl;
  std::cout << "  -d, --debug    Enable debug output" << std::endl;
  std::cout << "  -t, --trace    Enable trace output" << std::endl;
  std::cout << std::endl;
  std::cout << "If no file is specified, the GUI will open for file selection."
            << std::endl;
}

int main(int argc, char *argv[]) {
  std::string filePath;
  bool debugMode = false;
  bool traceMode = false;

  // 解析命令行参数
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printUsage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
      debugMode = true;
    } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) {
      traceMode = true;
    } else if (argv[i][0] != '-') {
      filePath = argv[i];
    }
  }

  std::cout << "========================================" << std::endl;
  std::cout << " Draw Language Interpreter" << std::endl;
  std::cout << "========================================" << std::endl;

  // 创建UI
  auto ui = createImGuiUI();
  auto *imguiUI = dynamic_cast<DrawLangImGuiUI *>(ui.get());

  if (!ui->initialize(1280, 800, "Draw Language Interpreter")) {
    std::cerr << "Failed to initialize UI" << std::endl;
    return 1;
  }

  // 配置解释器应用
  DrawLangApp &app = getApp();

  DrawLangApp::Config config;
  config.enableDebugOutput = debugMode;
  config.traceExecution = traceMode;
  app.setConfig(config);

  // 设置UI
  app.setUI(ui.get());

  // 注册到UI管理器
  getUIManager().setUI(ui.get());

  // 如果指定了文件，先加载
  if (!filePath.empty()) {
    ui->prepare(filePath);
    ui->showMessage(0, "File specified: " + filePath);
    ui->showMessage(0, "Press 'Execute' button or F5 to run.");
  } else {
    ui->showMessage(0, "Welcome to Draw Language Interpreter!");
    ui->showMessage(0, "Click 'Open File...' to select a Draw language file.");
    ui->showMessage(0, "");
  }

  std::cout << "UI initialized. Running main loop..." << std::endl;

  // 运行主循环
  ui->run();

  // 清理
  ui->shutdown();

  return 0;
}
