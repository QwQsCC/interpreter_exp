// Draw语言解释器测试示例
// 测试语法分析器和语义分析器，旧版本的测试，更全的测试用gtest在test目录下面

#include "spdlog/spdlog.h"
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// 包含Draw语言解释器相关头文件
#include "DrawLangAST.hpp"
#include "DrawLangParser.hpp"
#include "DrawLangSemantic.hpp"
#include "ErrorLog.hpp"
#include "SimpleLexer.hpp"

using namespace interpreter_exp;
using namespace interpreter_exp::ast;
using namespace interpreter_exp::parser;
using namespace interpreter_exp::semantic;
using namespace interpreter_exp::lexer;
using namespace interpreter_exp::errlog;

// 收集绘制的像素点
struct DrawnPixel {
  int x, y;
  unsigned char r, g, b;
};

std::vector<DrawnPixel> drawnPixels;

// 绘图回调函数
void drawCallback(double x, double y, const PixelAttribute &attr) {
  DrawnPixel pixel;
  pixel.x = static_cast<int>(x);
  pixel.y = static_cast<int>(y);
  pixel.r = attr.r;
  pixel.g = attr.g;
  pixel.b = attr.b;
  drawnPixels.push_back(pixel);
}

// 打印绘制结果
void printDrawnPixels(size_t maxCount = 20) {
  spdlog::info("Total pixels drawn: {}", drawnPixels.size());

  size_t count = std::min(maxCount, drawnPixels.size());
  for (size_t i = 0; i < count; ++i) {
    const auto &p = drawnPixels[i];
    spdlog::info("  Pixel[{}]: ({}, {}) color=({},{},{})", i, p.x, p.y,
                 (int)p.r, (int)p.g, (int)p.b);
  }

  if (drawnPixels.size() > maxCount) {
    spdlog::info("  ... and {} more pixels", (drawnPixels.size() - maxCount));
  }
}

// 测试用例1：基本的origin和scale语句
void testBasicStatements() {
  spdlog::info("\n=== Test Case 1: Basic Statements ===");

  const char *source = R"(
origin is (100, 200);
scale is (10, 10);
rot is 0;
)";

  spdlog::info("Input:\n{}", source);

  // 创建解释器
  DrawLangInterpreter interpreter;
  interpreter.setDrawCallback(drawCallback);

  // 执行
  drawnPixels.clear();
  bool success = interpreter.executeFromString(source, "test1");

  spdlog::info("Execution: {}", success ? "SUCCESS" : "FAILED");

  if (!success) {
    for (const auto &err : interpreter.getErrors()) {
      spdlog::error("  Error: {}", err);
    }
  }

  // 检查参数
  auto *sem = interpreter.getSemanticAnalyzer();
  spdlog::info("Origin: ({}, {})", sem->getOriginX(), sem->getOriginY());
  spdlog::info("Scale: ({}, {})", sem->getScaleX(), sem->getScaleY());
  spdlog::info("Rotation: {}", sem->getRotAngle());
}

// 测试用例2：for-draw语句
void testForDraw() {
  spdlog::info("\n=== Test Case 2: For-Draw Statement ===");

  const char *source = R"(
origin is (200, 200);
scale is (50, 50);
rot is 0;
for t from 0 to 6.28 step 0.1 draw (cos(t), sin(t));
)";

  spdlog::info("Input:\n{}", source);

  DrawLangInterpreter interpreter;
  interpreter.setDrawCallback(drawCallback);

  drawnPixels.clear();
  bool success = interpreter.executeFromString(source, "test2");

  spdlog::info("Execution: {}", success ? "SUCCESS" : "FAILED");

  if (!success) {
    for (const auto &err : interpreter.getErrors()) {
      spdlog::error("  Error: {}", err);
    }
  }

  printDrawnPixels(10);
}

// 测试用例3：颜色设置
void testColorStatement() {
  spdlog::info("\n=== Test Case 3: Color Statement ===");

  const char *source = R"(
origin is (100, 100);
scale is (1, 1);
color is (0, 255, 0);
for t from 0 to 100 step 1 draw (t, t);
)";

  spdlog::info("Input:\n{}", source);

  DrawLangInterpreter interpreter;
  interpreter.setDrawCallback(drawCallback);

  drawnPixels.clear();
  bool success = interpreter.executeFromString(source, "test3");

  spdlog::info("Execution: {}", success ? "SUCCESS" : "FAILED");

  if (success && !drawnPixels.empty()) {
    spdlog::info("First pixel color: ({}, {}, {})", (int)drawnPixels[0].r,
                 (int)drawnPixels[0].g, (int)drawnPixels[0].b);
  }

  printDrawnPixels(5);
}

// 测试用例4：表达式计算
void testExpressions() {
  spdlog::info("\n=== Test Case 4: Expression Evaluation ===");

  const char *source = R"(
origin is (2 * 50, 100 + 50);
scale is (5 + 5, 20 / 2);
rot is 3.14159 / 4;
for t from 0 to 2 step 0.5 draw (t * 10, t ** 2);
)";

  spdlog::info("Input:\n{}", source);

  DrawLangInterpreter interpreter;
  interpreter.setDrawCallback(drawCallback);

  drawnPixels.clear();
  bool success = interpreter.executeFromString(source, "test4");

  spdlog::info("Execution: {}", success ? "SUCCESS" : "FAILED");

  auto *sem = interpreter.getSemanticAnalyzer();
  spdlog::info("Origin (expected 100, 150): ({}, {})", sem->getOriginX(),
               sem->getOriginY());
  spdlog::info("Scale (expected 10, 10): ({}, {})", sem->getScaleX(),
               sem->getScaleY());

  printDrawnPixels();
}

// 测试用例5：直接使用Parser
void testParserDirectly() {
  spdlog::info("\n=== Test Case 5: Parser Direct Usage ===");

  const char *source = R"(
origin is (0, 0);
for t from 0 to 10 step 1 draw (t, t * 2);
)";

  spdlog::info("Input:\n{}", source);

  // 创建词法分析器
  auto input = std::make_unique<StringInputSource>(source, "test5");
  auto dfa = std::make_unique<TableDrivenDFA>();
  SimpleLexer lexer(std::move(input), std::move(dfa));

  // 创建语法分析器
  DrawLangParser parser(&lexer);
  parser.setFilename("test5");

  // 启用跟踪
  DrawParserConfig config;
  config.traceParsing = false; // 设为true可查看解析过程
  parser.setConfig(config);

  // 解析
  auto ast = parser.parse();

  if (ast) {
    spdlog::info("Parse SUCCESS!");
    spdlog::info("Number of statements: {}", ast->getChildCount());

    // 打印AST
    spdlog::info("AST:");
    ast->print(2);
  } else {
    spdlog::error("Parse FAILED!");
    for (const auto &err : parser.getErrors()) {
      spdlog::error("  Error: {}", err.toString());
    }
  }
}

// 测试用例6：错误处理
void testErrorHandling() {
  spdlog::info("\n=== Test Case 6: Error Handling ===");

  const char *source = R"(
origin is (100, 200);
scale is ;
rot is pi;
)";

  spdlog::info("Input (with syntax error):\n{}", source);

  DrawLangInterpreter interpreter;
  bool success = interpreter.executeFromString(source, "test6");

  spdlog::info("Execution: {}", success ? "SUCCESS" : "FAILED");
  spdlog::info("Errors:");
  for (const auto &err : interpreter.getErrors()) {
    spdlog::error("  {}", err);
  }
}

int main() {
  spdlog::info("========================================");
  spdlog::info(" Draw Language Interpreter Test Suite");
  spdlog::info("========================================");

  testBasicStatements();
  testForDraw();
  testColorStatement();
  testExpressions();
  testParserDirectly();
  testErrorHandling();

  spdlog::info("\n========================================");
  spdlog::info(" All tests completed!");
  spdlog::info("========================================");

  return 0;
}
