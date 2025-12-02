// Draw语言语义分析器的实现
// 实现语义计算和绘图操作

#include "DrawLangSemantic.hpp"
#include "ErrorLog.hpp"
#include "lexer.hpp"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace interpreter_exp {
namespace semantic {

using namespace ast;
using namespace parser;
using namespace lexer;
using namespace errlog;

DrawLangSemanticAnalyzer::DrawLangSemanticAnalyzer(DrawLangParser *parser)
    : parser_(parser) {
  // 设置默认颜色（红色）
  double r, g, b;
  ColorStmtNode::getDefaultColor(r, g, b);
  attr_.setColor(r, g, b);

  // 如果有parser，设置T值存储
  if (parser_) {
    parser_->setTStorage(&tStorage_);
  }
}

DrawLangSemanticAnalyzer::~DrawLangSemanticAnalyzer() = default;

void DrawLangSemanticAnalyzer::setParser(DrawLangParser *parser) {
  parser_ = parser;
  if (parser_) {
    parser_->setTStorage(&tStorage_);
  }
}

int DrawLangSemanticAnalyzer::run(ProgramNode *program) {
  if (!program) {
    return -1;
  }

  // 演示模式：添加Zorro图案
  if (config_.enableDemoMode) {
    executeZorroDemo(program);
  }

  // 遍历所有语句
  size_t stmtCount = program->getChildCount();
  for (size_t i = 0; i < stmtCount; ++i) {
    auto *stmt = program->getStatement(i);
    if (stmt) {
      executeStatement(stmt);
    }
  }

  return 0;
}

void DrawLangSemanticAnalyzer::executeStatement(StatementNode *stmt) {
  if (!stmt)
    return;

  switch (stmt->getNodeType()) {
  case DrawASTNodeType::OriginStmt:
    executeOriginStmt(static_cast<OriginStmtNode *>(stmt));
    break;
  case DrawASTNodeType::ScaleStmt:
    executeScaleStmt(static_cast<ScaleStmtNode *>(stmt));
    break;
  case DrawASTNodeType::RotStmt:
    executeRotStmt(static_cast<RotStmtNode *>(stmt));
    break;
  case DrawASTNodeType::ForDrawStmt:
    executeForDrawStmt(static_cast<ForDrawStmtNode *>(stmt));
    break;
  case DrawASTNodeType::ColorStmt:
    executeColorStmt(static_cast<ColorStmtNode *>(stmt));
    break;
  case DrawASTNodeType::SizeStmt:
    executeSizeStmt(static_cast<SizeStmtNode *>(stmt));
    break;
  default:
    // 未知语句类型
    break;
  }
}

void DrawLangSemanticAnalyzer::executeOriginStmt(OriginStmtNode *stmt) {
  originX_ = stmt->getX();
  originY_ = stmt->getY();

  if (config_.enableDebugOutput) {
    ErrLog::logPrint("ORIGIN: ({}, {})\n", originX_, originY_);
  }
}

void DrawLangSemanticAnalyzer::executeScaleStmt(ScaleStmtNode *stmt) {
  scaleX_ = stmt->getScaleX();
  scaleY_ = stmt->getScaleY();

  if (config_.enableDebugOutput) {
    ErrLog::logPrint("SCALE: ({}, {})\n", scaleX_, scaleY_);
  }
}

void DrawLangSemanticAnalyzer::executeRotStmt(RotStmtNode *stmt) {
  rotAngle_ = stmt->getAngle();

  if (config_.enableDebugOutput) {
    ErrLog::logPrint("ROT: {}\n", rotAngle_);
  }
}

void DrawLangSemanticAnalyzer::executeForDrawStmt(ForDrawStmtNode *stmt) {
  drawLoop(stmt->getStartExpr(), stmt->getEndExpr(), stmt->getStepExpr(),
           stmt->getXExpr(), stmt->getYExpr());
}

void DrawLangSemanticAnalyzer::executeColorStmt(ColorStmtNode *stmt) {
  if (stmt->usesColorName()) {
    // 使用颜色名称
    auto *colorName = stmt->getColorName();
    if (colorName) {
      double r, g, b;
      colorName->getRGB(r, g, b);
      attr_.setColor(r, g, b);
    }
  } else {
    // 使用RGB值
    attr_.setColor(stmt->getRed(), stmt->getGreen(), stmt->getBlue());
  }

  if (config_.enableDebugOutput) {
    ErrLog::logPrint("COLOR: ({}, {}, {})\n", static_cast<int>(attr_.r),
                     static_cast<int>(attr_.g), static_cast<int>(attr_.b));
  }
}

void DrawLangSemanticAnalyzer::executeSizeStmt(SizeStmtNode *stmt) {
  double sz = stmt->getSize();
  if (sz >= 1) {
    attr_.setSize(sz);
  }

  if (config_.enableDebugOutput) {
    ErrLog::logPrint("SIZE: {}\n", attr_.size);
  }
}

void DrawLangSemanticAnalyzer::calcCoord(ExpressionNode *xTree,
                                         ExpressionNode *yTree, double *ptrX,
                                         double *ptrY) {
  // 计算表达式的值，得到点的原始坐标
  double xVal = xTree ? xTree->value() : 0.0;
  double yVal = yTree ? yTree->value() : 0.0;

  // 比例变换
  xVal *= scaleX_;
  yVal *= scaleY_;

  // 旋转变换 (与原始compile_exp保持一致的顺时针旋转)
  // x' = x * cos(θ) + y * sin(θ)
  // y' = y * cos(θ) - x * sin(θ)
  double cosAngle = std::cos(rotAngle_);
  double sinAngle = std::sin(rotAngle_);
  double xTemp = xVal * cosAngle + yVal * sinAngle;
  double yTemp = yVal * cosAngle - xVal * sinAngle;
  xVal = xTemp;
  yVal = yTemp;

  // 平移变换
  xVal += originX_;
  yVal += originY_;

  // 返回变换后的坐标
  if (ptrX)
    *ptrX = xVal;
  if (ptrY)
    *ptrY = yVal;
}

void DrawLangSemanticAnalyzer::drawLoop(ExpressionNode *startTree,
                                        ExpressionNode *endTree,
                                        ExpressionNode *stepTree,
                                        ExpressionNode *xTree,
                                        ExpressionNode *yTree) {
  // 计算起点、终点、步长
  double startVal = startTree ? startTree->value() : 0.0;
  double endVal = endTree ? endTree->value() : 0.0;
  double stepVal = stepTree ? stepTree->value() : 1.0;

  // 调试输出 - 始终输出以方便调试
  if (config_.enableDebugOutput) {
    spdlog::debug("FOR loop: start={}, end={}, step={}", startVal, endVal,
                  stepVal);
  }

  if (stepVal == 0.0) {
    ErrLog::error_msg("Step value cannot be zero!");
    return;
  }

  // 检查步长方向是否正确
  if ((stepVal > 0 && startVal > endVal) ||
      (stepVal < 0 && startVal < endVal)) {
    spdlog::warn("Step direction mismatch, loop will not execute!");
    return;
  }

  int pointCount = 0;

  // 循环绘制
  // 注意：ParamExprNode使用parser的tStorage_指针，
  // setParser已经将其指向了analyzer的tStorage_
  for (tStorage_ = startVal; tStorage_ <= endVal; tStorage_ += stepVal) {
    double x, y;
    calcCoord(xTree, yTree, &x, &y);

    // 每100个点输出一次调试信息
    if (config_.enableDebugOutput &&
        (pointCount < 5 || pointCount % 100 == 0)) {
      spdlog::debug("T={} -> raw({}, {}) -> transformed({}, {})", tStorage_,
                    (xTree ? xTree->value() : 0.0),
                    (yTree ? yTree->value() : 0.0), x, y);
    }

    drawPixel(x, y);
    pointCount++;
  }

  if (config_.enableDebugOutput) {
    spdlog::debug("FOR loop completed: {} points drawn", pointCount);
  }
}

void DrawLangSemanticAnalyzer::drawPixel(double x, double y) {
  if (drawCallback_) {
    drawCallback_(x, y, attr_);
  } else {
    // 默认输出到控制台
    if (config_.enableDebugOutput) {
      spdlog::debug("DrawPixel({}, {}) color=({}, {}, {})", static_cast<int>(x),
                    static_cast<int>(y), static_cast<int>(attr_.r),
                    static_cast<int>(attr_.g), static_cast<int>(attr_.b));
    }
  }
}

void DrawLangSemanticAnalyzer::executeZorroDemo(ProgramNode *program) {
  // 这个函数用于演示目的，创建一个"Z"图案
  // 与旧代码中的Zorro函数类似

  // 创建一些测试语句并添加到程序中
  // 这里简化处理，实际可以按需实现

  if (config_.enableDebugOutput) {
    spdlog::debug("// Zorro demo mode enabled");
  }
}

DrawLangInterpreter::DrawLangInterpreter() {
  semanticAnalyzer_ = std::make_unique<DrawLangSemanticAnalyzer>();
}

DrawLangInterpreter::~DrawLangInterpreter() = default;

bool DrawLangInterpreter::executeFromString(const std::string &source,
                                            const std::string &name) {
  errors_.clear();

  try {
    // 创建词法分析器
    auto input = std::make_unique<StringInputSource>(source, name);
    auto dfa = std::make_unique<TableDrivenDFA>();
    lexer_ = std::make_unique<SimpleLexer>(std::move(input), std::move(dfa));

    // 创建语法分析器
    parser_ = std::make_unique<DrawLangParser>(lexer_.get());
    parser_->setFilename(name);

    // 设置语义分析器
    semanticAnalyzer_->setParser(parser_.get());

    // 语法分析
    ast_ = parser_->parse();
    if (!ast_) {
      errors_.push_back("Parse failed: no valid statements");
      return false;
    }

    // 检查语法错误
    if (parser_->hasErrors()) {
      for (const auto &err : parser_->getErrors()) {
        errors_.push_back(err.toString());
      }
    }

    // 语义分析/执行
    int result = semanticAnalyzer_->run(ast_.get());

    return result == 0 && errors_.empty();

  } catch (const std::exception &ex) {
    errors_.push_back(std::string("Exception: ") + ex.what());
    return false;
  }
}

bool DrawLangInterpreter::executeFromFile(const std::string &filename) {
  errors_.clear();

  try {
    // 创建文件输入源
    auto input = std::make_unique<FileInputSource>(filename);
    auto dfa = std::make_unique<TableDrivenDFA>();
    lexer_ = std::make_unique<SimpleLexer>(std::move(input), std::move(dfa));

    // 创建语法分析器
    parser_ = std::make_unique<DrawLangParser>(lexer_.get());
    parser_->setFilename(filename);

    // 设置语义分析器
    semanticAnalyzer_->setParser(parser_.get());

    // 语法分析
    ast_ = parser_->parse();
    if (!ast_) {
      errors_.push_back("Parse failed: no valid statements");
      return false;
    }

    // 检查语法错误
    if (parser_->hasErrors()) {
      for (const auto &err : parser_->getErrors()) {
        errors_.push_back(err.toString());
      }
    }

    // 语义分析/执行
    int result = semanticAnalyzer_->run(ast_.get());

    return result == 0 && errors_.empty();

  } catch (const std::exception &ex) {
    errors_.push_back(std::string("Exception: ") + ex.what());
    return false;
  }
}

void DrawLangInterpreter::setDrawCallback(DrawPixelCallback callback) {
  if (semanticAnalyzer_) {
    semanticAnalyzer_->setDrawCallback(std::move(callback));
  }
}

bool DrawLangInterpreter::hasErrors() const { return !errors_.empty(); }

std::vector<std::string> DrawLangInterpreter::getErrors() const {
  return errors_;
}

} // namespace semantic
} // namespace interpreter_exp
